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
	//��¼�ɹ��ص�
	virtual void onLoginSuccess(CP2PClient* pClient, long long llLoginId) {}

	//�򶴳ɹ��ص�
	virtual void onPunchSuccess(CP2PClient* pClient, const CEndPoint& endpoint, long long llRemoteLoginId) {}

	//���շ�������Ϣ�ص�
	virtual void onRecvDataFromServer(CP2PClient* pClient, const char* pData) {}

	//���ܿͻ�����Ϣ�ص�
	virtual void onRecvDataFromClient(CP2PClient* pClient, const CEndPoint& endpoint, const char* pData) {}
};



class CP2PClientDataSource
{
public:
	//�������ݻص�
	virtual CP2PMessage* parseSourceData(CP2PClient* pClient, const char* pData, size_t iLen);

	//����������Ϣ�Ļص�
	virtual CP2PMessage* createPongMessage(CP2PClient* pClient);
	virtual CP2PMessage* createPingMessage(CP2PClient* pClient);
	virtual CP2PMessage* createPunchToClientMessage(CP2PClient* pClient);
	virtual CP2PMessage* createPunchToServerMessage(CP2PClient* pClient, const std::string& strTarAddr);
	virtual CP2PMessage* createReplyMessage(CP2PClient* pClient, long long llLoginId);
	virtual CP2PMessage* createLoginMessage(CP2PClient* pClient);
	virtual CP2PMessage* createLogoutMessage(CP2PClient* pClient);
	virtual CP2PMessage* createDataMessage(CP2PClient* pClient, const CEndPoint& endpoint, const std::string& strData);

	//����reply��Ϣ�ص�
	virtual bool parseReplyLoginId(CP2PClient* pClient, const CP2PMessage* pMessage, long long& ll);
	virtual bool parseReplyLoginId(CP2PClient* pClient, const CP2PMessagePtr& ptr, long long& ll);

	//������¼�ɹ���Ϣ�ص�
	virtual bool parseLoginSuccessLoginId(CP2PClient* pClient, const CP2PMessage* pMessage, long long& ll);
	virtual bool parseLoginSuccessLoginId(CP2PClient* pClient, const CP2PMessagePtr& ptr, long long& ll);

	//�������Է�������punch��Ϣ�Ļص�
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

	//�������������ͼ��ʱ��,��λΪ����
	void setKeepAliveSleepTime(long long llTime) { m_keeper.setSleepTime(llTime); }

	//��ʼ���ر������߳�
	bool startKeepAlive() { m_keeper.start(); return true; }
	bool stopKeepAlive() { m_keeper.stop(); return true; }

	virtual bool init() override;
	virtual bool reset() override;
	virtual void close() override;

	bool sendto(const CEndPoint& endpoint, const CP2PMessage* pMessage);
	bool sendto(const CEndPoint& endpoint, const CP2PMessagePtr& ptr);

	//�����̻߳ص�����
	virtual void recvData(const char* pData, size_t iLen, sockaddr_in* pAddr);

	//���ô���
	void setDelegate(CP2PClientDelegate* pDelegate) { m_pDelegate = pDelegate; }
	CP2PClientDelegate* getDelegate() { return m_pDelegate; }


	//�������ݺ���
	bool send(const std::string& strTarAddr, const std::string& strData);

	//����ture�������¼�����ͳɹ�,����delegate��onLoginSuccess�����ǲŴ����¼�ɹ�
	bool login();

	//�ǳ�,�����ĵǳ��Ƿ�ɹ�
	bool logout();

	//����ture������������ͳɹ�,����delegate��onPunchSuccess�����ǲŴ���򶴳ɹ�
	bool punch(const std::string& strTarAddr);

protected:
	long long m_llLoginId;

	CP2PAliveKeeper m_keeper;

	CP2PClientDelegate* m_pDelegate;
	CP2PClientDataSource* m_pDataSource;
};

