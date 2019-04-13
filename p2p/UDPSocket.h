#pragma once
#include <string>
#include <functional>
#include "Utils/ThreadBase.h"

struct sockaddr_in;

class CEndPointConvertError
{
public:
	CEndPointConvertError(const std::string& str) noexcept :strError(str) {}
	std::string strError;
};

struct CEndPoint
{
	std::string strIP;
	int iPort;

	void convertFromStr(const std::string& strAddrInfo);
	void convertToAddr(sockaddr_in* pAddr) const;
	std::string convertToStr() const;

	bool operator == (const CEndPoint& endpoint) const { return iPort == endpoint.iPort && strIP == endpoint.strIP; }

	CEndPoint(const std::string& strOuterIP, int iOuterPort) :
		strIP(strOuterIP),
		iPort(iOuterPort) {}

	CEndPoint(const CEndPoint& endpoint) :
		strIP(endpoint.strIP),
		iPort(endpoint.iPort) {}

	CEndPoint() :iPort(0) {}

	CEndPoint(sockaddr_in* pAddr);

	CEndPoint(const std::string& strAddrInfo);
};



class CUDPSocket : public CThreadBase
{
public:
	using CUDPSocketRecvfromCallBack = std::function<void(const char*, size_t, sockaddr_in*)>;

	CUDPSocket() :m_funcCallBack(nullptr), m_iSock(0) {}
	virtual ~CUDPSocket();

	void setRecvfromCallBack(CUDPSocketRecvfromCallBack func) { m_funcCallBack = func; }

	virtual bool init();
	virtual bool reset();
	virtual void close();

	//start receive thread
	bool startRecvfrom();

	//stop receive thread
	bool stopRecvfrom();

	virtual bool run() override;

	bool sendto(const char* pIP, int iPort, const char* pData);
	bool sendto(const CEndPoint& endpoint, const char* pData);
	bool sendto(sockaddr_in* pAddr, const char* pData);

	//global start up for windows
	static bool startUp();
	//global clean up for windows
	static void clearUp();

protected:
	int m_iSock;

	CUDPSocketRecvfromCallBack m_funcCallBack;
};



class CUDPStaticSocket : public CUDPSocket
{
public:
	CUDPStaticSocket(int iListenPort, const std::string& strListenIP = "0.0.0.0") :
		m_epListen(strListenIP, iListenPort) {}
	CUDPStaticSocket(const CEndPoint& endpoint) :
		m_epListen(endpoint) {}
	~CUDPStaticSocket() {}

	virtual bool init() override;

protected:
	CEndPoint m_epListen;
};



class CUDPAliveKeeper : public CThreadBase
{
public:
	using CUDPAliveKeeperAppendDataCallBack = std::function<void(const CEndPoint&, std::string&)>;

	CUDPAliveKeeper(CUDPSocket* pSocket, const std::string& strIP, int iPort) :
		m_pSocket(pSocket), 
		m_llSleepTime(5000), 
		m_funcCallBack(nullptr),
		m_epServer(strIP, iPort) {}
	CUDPAliveKeeper(CUDPSocket* pSocket, const CEndPoint& endpoint) :
		m_pSocket(pSocket),
		m_llSleepTime(5000),
		m_funcCallBack(nullptr),
		m_epServer(endpoint) {}
	~CUDPAliveKeeper() {}

	virtual bool run() override;

	void setSleepTime(long long llTime) { m_llSleepTime = llTime; }
	void setAppendDataCallBack(CUDPAliveKeeperAppendDataCallBack func) { m_funcCallBack = func; }

	const CEndPoint& getServerAddr() { return m_epServer; }

protected:
	long long m_llSleepTime;
	CEndPoint m_epServer;
	CUDPSocket* m_pSocket;
	CUDPAliveKeeperAppendDataCallBack m_funcCallBack;
};