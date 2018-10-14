#pragma once
#include "UDPSocket.h"
#include "P2PMessage.h"
#include "Utils/ThSfMap.h"


class CP2PServer;


class CP2PServerDelegate
{
public:
	//客户端登陆消息回调,用户需要为登录的客户端分配唯一的id
	//返回true-登录成功,false登录失败
	virtual bool onClientLogin(CP2PServer* pServer, const CEndPoint& endpoint, const char* pData, long long& llLoginId) 
	{
		static long long llId = 1;
		llLoginId = llId++;
		return true; 
	}

	//客户端登出消息回调
	virtual void onClientLogout(CP2PServer* pServer, const CEndPoint& endpoint, long long llLoginId) {}

	//客户端打洞请求回调
	//返回true-请求合法,false请求非法
	virtual bool onClientPunch(CP2PServer* pServer, const CEndPoint& epSrc, long long llLoginId, const CEndPoint& epTar) { return true; }
	
	//接受客户端消息回调
	virtual void onRecvData(CP2PServer* pServer, const CEndPoint& endpoint, const char* pData, long long llLoginId) {}
};


class CP2PServerDataSource
{
public:
	//解析数据回调
	virtual CP2PMessage* parseSourceData(CP2PServer* pServer, const char* pData, size_t iLen);

	//创建各种消息的回调
	virtual CP2PMessage* createPongMessage(CP2PServer* pServer);
	virtual CP2PMessage* createPingMessage(CP2PServer* pServer);
	virtual CP2PMessage* createPunchMessage(CP2PServer* pServer, const std::string& strTarAddr);
	virtual CP2PMessage* createLoginSuccessMessage(CP2PServer* pServer, long long llLoginId);
	virtual CP2PMessage* createDataMessage(CP2PServer* pServer, const CEndPoint& endpoint, const std::string& strData);

	//解析来自客户端的punch消息的回调
	//warnning : it will new a ptr
	virtual bool parsePunchTargetAddr(CP2PServer* pServer, CP2PMessage* pMessage, CEndPoint*& pEndpoint);
	virtual bool parsePunchTargetAddr(CP2PServer* pServer, const CP2PMessagePtr& ptr, CEndPoint*& pEndpoint);
};


class CP2PServer : public CUDPStaticSocket, public CThSfMap<long long, CEndPoint>
{
public:
	using CContainer = CThSfMap<long long, CEndPoint>;

	CP2PServer(CP2PServerDataSource* pDataSource, int iListenPort, const std::string& strListenIP = "0.0.0.0") :
		m_pDataSource(pDataSource),
		CUDPStaticSocket(iListenPort, strListenIP) {}
	CP2PServer(CP2PServerDataSource* pDataSource, const CEndPoint& endpoint) :
		m_pDataSource(pDataSource),
		CUDPStaticSocket(endpoint) {}

	virtual bool init() override;

	bool sendto(const CEndPoint& endpoint, CP2PMessage* pMessage);
	bool sendto(const CEndPoint& endpoint, const CP2PMessagePtr& ptr);

	bool send(const std::string& strTarAddr, const std::string& strData);

	virtual void recvData(const char* pData, size_t iLen, sockaddr_in* pAddr);

	void setDelegate(CP2PServerDelegate* pDelegate) { m_pDelegate = pDelegate; }
	CP2PServerDelegate* getDelegate() { return m_pDelegate; }

	std::string outputUserList();

protected:
	CP2PServerDelegate* m_pDelegate;
	CP2PServerDataSource* m_pDataSource;
};

