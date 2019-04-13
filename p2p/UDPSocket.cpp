#include "UDPSocket.h"
#include <boost/format.hpp>

#ifdef _WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define UDP_RECV_MAX_BUF_SIZE 1024

using namespace std;
using namespace boost;

void CEndPoint::convertToAddr(sockaddr_in* pAddr) const
{
	memset(pAddr, 0, sizeof (*pAddr));
	pAddr->sin_family = AF_INET;
	pAddr->sin_addr.s_addr = inet_addr(strIP.c_str());
	pAddr->sin_port = htons(iPort);
}

string CEndPoint::convertToStr() const
{
	format f("%s:%d");
	f % strIP.c_str() % iPort;
	return f.str();
}

CEndPoint::CEndPoint(sockaddr_in* pAddr) :
	strIP(inet_ntoa(pAddr->sin_addr)),
	iPort(ntohs(pAddr->sin_port))
{
	
}

CEndPoint::CEndPoint(const std::string& strAddrInfo) :
	iPort(0)
{
	convertFromStr(strAddrInfo);
}

void CEndPoint::convertFromStr(const std::string& strAddrInfo)
{
	size_t pos = strAddrInfo.find(':');
	if (pos == string::npos)
	{
		throw CEndPointConvertError("end point convert error, missing ':' : " + strAddrInfo);
	}

	strIP = strAddrInfo.substr(0, pos);
	if (strAddrInfo.size() < pos + 1)
	{
		throw CEndPointConvertError("end point convert error, error length : " + strAddrInfo);
	}

	iPort = atoi(strAddrInfo.substr(pos + 1).c_str());
	if (iPort < 1 || iPort > 65535)
	{
		throw CEndPointConvertError("end point convert error, convert type : " + strAddrInfo);
	}
}



CUDPSocket::~CUDPSocket()
{
	close();
}

bool CUDPSocket::init()
{
	if (m_iSock)
		return false;

	m_iSock = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
	if (m_iSock == -1)
		return false;

	//forbidden 10054 error
#ifdef _WIN32
	BOOL b = FALSE;
	DWORD d = 0;
	WSAIoctl(m_iSock, _WSAIOW(IOC_VENDOR, 12), &b, sizeof b, NULL, 0, &d, NULL, NULL);
#endif

	return true;
}

bool CUDPSocket::reset()
{
	close();
	return init();
}

void CUDPSocket::close()
{
	if (m_iSock)
	{
		stopRecvfrom();

#ifdef _WIN32
		closesocket(m_iSock);
#else
		close(m_iSock);
#endif
		m_iSock = 0;
	}
}

bool CUDPSocket::startUp()
{
#ifdef _WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return false;
#endif
	return true;
}

void CUDPSocket::clearUp()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

bool CUDPSocket::startRecvfrom()
{
	if (!m_iSock)
		return false;

	CThreadBase::start();
	return true;
}

bool CUDPSocket::stopRecvfrom()
{
	CThreadBase::stop();
	return true;
}

bool CUDPSocket::run()
{
	int iMaxFd = m_iSock + 1;
	fd_set readfds;
	struct timeval timeout;

	FD_ZERO(&readfds);
	FD_SET(m_iSock, &readfds);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	int ret = select(iMaxFd, &readfds, NULL, NULL, &timeout);
	if (ret > 0 && FD_ISSET(m_iSock, &readfds))
	{
		sockaddr_in peer;
		socklen_t addrlen = sizeof(peer);
		memset(&peer, 0, addrlen);
		char buf[UDP_RECV_MAX_BUF_SIZE] = { 0 };

		int rd_size = recvfrom(m_iSock, buf, UDP_RECV_MAX_BUF_SIZE, 0,
			(struct sockaddr*)&peer, &addrlen);
		if (rd_size > 0 && m_funcCallBack)
		{
			m_funcCallBack(buf, rd_size, &peer);
		}
	}
	return true;
}

bool CUDPSocket::sendto(const char* pIP, int iPort, const char* pData)
{
	return sendto(CEndPoint(pIP, iPort), pData);
}

bool CUDPSocket::sendto(const CEndPoint& endpoint, const char* pData)
{
	sockaddr_in addr;
	endpoint.convertToAddr(&addr);
	return sendto(&addr, pData);
}

bool CUDPSocket::sendto(sockaddr_in* pAddr, const char* pData)
{
	if (!m_iSock)
		return false;

	int iLen = static_cast<int>(strlen(pData));
	return iLen == ::sendto(m_iSock, pData, iLen,
		0, (struct sockaddr *)pAddr, sizeof(*pAddr));
}



bool CUDPAliveKeeper::run()
{
	if (m_funcCallBack)
	{
		string strData;
		m_funcCallBack(m_epServer, strData);
		m_pSocket->sendto(m_epServer, strData.c_str());
	}
	return CThreadBase::threadSleep(m_llSleepTime);
}



bool CUDPStaticSocket::init()
{
	if (!CUDPSocket::init())
		return false;

	sockaddr_in addr;
	m_epListen.convertToAddr(&addr);
	return ::bind(m_iSock, (const struct sockaddr*)&addr, sizeof(addr)) != -1;
}