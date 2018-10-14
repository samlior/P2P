#pragma once
#include "UDPSocket.h"
#include "P2PMessage.h"
#include "Utils/ThSfMap.h"


class CP2PServer;


class CP2PServerDelegate
{
public:
	//�ͻ��˵�½��Ϣ�ص�,�û���ҪΪ��¼�Ŀͻ��˷���Ψһ��id
	//����true-��¼�ɹ�,false��¼ʧ��
	virtual bool onClientLogin(CP2PServer* pServer, const CEndPoint& endpoint, const char* pData, long long& llLoginId) 
	{
		static long long llId = 1;
		llLoginId = llId++;
		return true; 
	}

	//�ͻ��˵ǳ���Ϣ�ص�
	virtual void onClientLogout(CP2PServer* pServer, const CEndPoint& endpoint, long long llLoginId) {}

	//�ͻ��˴�����ص�
	//����true-����Ϸ�,false����Ƿ�
	virtual bool onClientPunch(CP2PServer* pServer, const CEndPoint& epSrc, long long llLoginId, const CEndPoint& epTar) { return true; }
	
	//���ܿͻ�����Ϣ�ص�
	virtual void onRecvData(CP2PServer* pServer, const CEndPoint& endpoint, const char* pData, long long llLoginId) {}
};


class CP2PServerDataSource
{
public:
	//�������ݻص�
	virtual CP2PMessage* parseSourceData(CP2PServer* pServer, const char* pData, size_t iLen);

	//����������Ϣ�Ļص�
	virtual CP2PMessage* createPongMessage(CP2PServer* pServer);
	virtual CP2PMessage* createPingMessage(CP2PServer* pServer);
	virtual CP2PMessage* createPunchMessage(CP2PServer* pServer, const std::string& strTarAddr);
	virtual CP2PMessage* createLoginSuccessMessage(CP2PServer* pServer, long long llLoginId);
	virtual CP2PMessage* createDataMessage(CP2PServer* pServer, const CEndPoint& endpoint, const std::string& strData);

	//�������Կͻ��˵�punch��Ϣ�Ļص�
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

