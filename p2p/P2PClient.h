#pragma once
#include "UDPSocket.h"
#include "P2PMessage.h"
#include "Utils/ThSfMap.h"

class CP2PClient;

class CP2PAliveKeeper : public CUDPAliveKeeper, public CThSfMap<long long, CEndPoint>
{
public:
	CP2PAliveKeeper(CUDPSocket* pSocket, const std::string& strIP, int iPort) :CUDPAliveKeeper(pSocket, strIP, iPort) {}
	CP2PAliveKeeper(CUDPSocket* pSocket, const CEndPoint& endpoint) :CUDPAliveKeeper(pSocket, endpoint) {}

	virtual bool run() override;
};



class CP2PClientDelegate
{
public:
	//登录成功回调
	virtual void onLoginSuccess(CP2PClient* pClient, long long llLoginId) {}

	//打洞成功回调
	virtual void onPunchSuccess(CP2PClient* pClient, const CEndPoint& endpoint, long long llRemoteLoginId) {}

	//接收服务器信息回调
	virtual void onRecvDataFromServer(CP2PClient* pClient, const char* pData) {}

	//接受客户端信息回调
	virtual void onRecvDataFromClient(CP2PClient* pClient, const CEndPoint& endpoint, const char* pData) {}
};



class CP2PClientDataSource
{
public:
	//解析数据回调
	virtual CP2PMessage* parseSourceData(CP2PClient* pClient, const char* pData, size_t iLen);

	//创建各种消息的回调
	virtual CP2PMessage* createPongMessage(CP2PClient* pClient);
	virtual CP2PMessage* createPingMessage(CP2PClient* pClient);
	virtual CP2PMessage* createPunchToClientMessage(CP2PClient* pClient);
	virtual CP2PMessage* createPunchToServerMessage(CP2PClient* pClient, const std::string& strTarAddr);
	virtual CP2PMessage* createReplyMessage(CP2PClient* pClient, long long llLoginId);
	virtual CP2PMessage* createLoginMessage(CP2PClient* pClient);
	virtual CP2PMessage* createLogoutMessage(CP2PClient* pClient);
	virtual CP2PMessage* createDataMessage(CP2PClient* pClient, const CEndPoint& endpoint, const std::string& strData);

	//解析reply消息回调
	virtual bool parseReplyLoginId(CP2PClient* pClient, const CP2PMessage* pMessage, long long& ll);
	virtual bool parseReplyLoginId(CP2PClient* pClient, const CP2PMessagePtr& ptr, long long& ll);

	//解析登录成功消息回调
	virtual bool parseLoginSuccessLoginId(CP2PClient* pClient, const CP2PMessage* pMessage, long long& ll);
	virtual bool parseLoginSuccessLoginId(CP2PClient* pClient, const CP2PMessagePtr& ptr, long long& ll);

	//解析来自服务器的punch消息的回调
	virtual bool parsePunchTargetAddr(CP2PClient* pClient, const CP2PMessage* pMessage, CEndPoint& endpoint);
	virtual bool parsePunchTargetAddr(CP2PClient* pClient, const CP2PMessagePtr& ptr, CEndPoint& endpoint);

	static CP2PClientDataSource* createDefaultDataSource() { return new CP2PClientDataSource; }
};



class CP2PClient : public CUDPSocket
{
public:
	CP2PClient(CP2PClientDataSource* pDataSource, const std::string& strIP, int iPort) :
		m_keeper(this, strIP, iPort), 
		m_llLoginId(0), 
		m_pDelegate(nullptr),
		m_pDataSource(pDataSource) {}
	CP2PClient(CP2PClientDataSource* pDataSource, const CEndPoint& endpoint) :
		m_keeper(this, endpoint),
		m_llLoginId(0), 
		m_pDelegate(nullptr),
		m_pDataSource(pDataSource) {}
	~CP2PClient() {}

	//设置心跳包发送间隔时间,单位为毫秒
	void setKeepAliveSleepTime(long long llTime) { m_keeper.setSleepTime(llTime); }

	//开始、关闭心跳线程
	bool startKeepAlive() { m_keeper.start(); return true; }
	bool stopKeepAlive() { m_keeper.stop(); return true; }

	virtual bool init() override;
	virtual bool reset() override;
	virtual void close() override;

	bool sendto(const CEndPoint& endpoint, const CP2PMessage* pMessage);
	bool sendto(const CEndPoint& endpoint, const CP2PMessagePtr& ptr);

	//接受线程回调函数
	virtual void recvData(const char* pData, size_t iLen, sockaddr_in* pAddr);

	//设置代理
	void setDelegate(CP2PClientDelegate* pDelegate) { m_pDelegate = pDelegate; }
	CP2PClientDelegate* getDelegate() { return m_pDelegate; }


	//发送数据函数
	bool send(const std::string& strTarAddr, const std::string& strData);

	//返回ture仅代表登录请求发送成功,进入delegate的onLoginSuccess方法是才代表登录成功
	bool login();

	//登出,不关心登出是否成功
	bool logout();

	//返回ture仅代表打洞请求发送成功,进入delegate的onPunchSuccess方法是才代表打洞成功
	bool punch(const std::string& strTarAddr);

protected:
	long long m_llLoginId;

	CP2PAliveKeeper m_keeper;

	CP2PClientDelegate* m_pDelegate;
	CP2PClientDataSource* m_pDataSource;
};

