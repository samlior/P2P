#include "P2PServer.h"
#include "P2PMessage.h"
#include <boost/format.hpp>

using namespace std;
using namespace boost;



bool CP2PServerDataSource::parsePunchTargetAddr(CP2PServer* pServer, CP2PMessage* pMessage, CEndPoint*& pEndpoint)
{
	const char* pCustomData = pMessage->getCustomData();
	bool bRet = false;
	if (pCustomData)
	{
		try
		{
			pEndpoint = new CEndPoint(pCustomData);
			bRet = true;
		}
		catch (const CEndPointConvertError&)
		{
			//etc...
		}
	}
	return bRet;
}

bool CP2PServerDataSource::parsePunchTargetAddr(CP2PServer* pServer, const CP2PMessagePtr& ptr, CEndPoint*& pEndpoint)
{
	const char* pCustomData = ptr->getCustomData();
	bool bRet = false;
	if (pCustomData)
	{
		try
		{
			pEndpoint = new CEndPoint(pCustomData);
			bRet = true;
		}
		catch (const CEndPointConvertError&)
		{
			//etc...
		}
	}
	return bRet;
}


CP2PMessage* CP2PServerDataSource::parseSourceData(CP2PServer* pServer, const char* pData, size_t iLen)
{
	return CP2PMessage::createMessageWithSourceData(pData, iLen);
}

CP2PMessage* CP2PServerDataSource::createPingMessage(CP2PServer* pServer)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_PING, nullptr, 0);
}

CP2PMessage* CP2PServerDataSource::createPongMessage(CP2PServer* pServer)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_PONG, nullptr, 0);
}

CP2PMessage* CP2PServerDataSource::createPunchMessage(CP2PServer* pServer, const string& strTarAddr)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_PUNCH, strTarAddr.c_str(), strTarAddr.size());
}

CP2PMessage* CP2PServerDataSource::createLoginSuccessMessage(CP2PServer* pServer, long long llLoginId)
{
	format f("%lld");
	f % llLoginId;
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_LOGIN_SUCCESS, f.str().c_str(), f.str().size());
}

CP2PMessage* CP2PServerDataSource::createDataMessage(CP2PServer* pServer, const CEndPoint& endpoint, const string& strData)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_DATA, strData.c_str(), strData.size());
}






bool CP2PServer::init()
{
	if (!CUDPStaticSocket::init())
		return false;

	CUDPStaticSocket::setRecvfromCallBack(bind(&CP2PServer::recvData, this, placeholders::_1, placeholders::_2, placeholders::_3));
	if (!CUDPStaticSocket::startRecvfrom())
		return false;

	return true;
}

bool CP2PServer::send(const string& strTarAddr, const string& strData)
{
	try
	{
		CEndPoint endpoint(strTarAddr);
		CP2PMessagePtr ptr(m_pDataSource->createDataMessage(this, endpoint, strData));
		return sendto(endpoint, ptr);
	}
	catch (const CEndPointConvertError&)
	{

	}
	return false;
}

bool CP2PServer::sendto(const CEndPoint& endpoint, CP2PMessage* pMessage)
{
	return CUDPStaticSocket::sendto(endpoint, pMessage->getData());
}

bool CP2PServer::sendto(const CEndPoint& endpoint, const CP2PMessagePtr& ptr)
{
	return CUDPStaticSocket::sendto(endpoint, ptr->getData());
}

void CP2PServer::recvData(const char* pData, size_t iLen, sockaddr_in* pAddr)
{
	CP2PMessagePtr ptr(m_pDataSource->parseSourceData(this, pData, strlen(pData)));

	if (ptr->getType() != P2P_MSG_TYPE_LOGIN &&
		ptr->getType() != P2P_MSG_TYPE_LOGOUT &&
		ptr->getType() != P2P_MSG_TYPE_PUNCH &&
		ptr->getType() != P2P_MSG_TYPE_DATA)
	{
		return;
	}

	CEndPoint epFrom(pAddr);
	long long llClientLoginId = 0;
	bool bLogin = CContainer::find(epFrom, llClientLoginId);

	switch (ptr->getType())
	{
	case P2P_MSG_TYPE_LOGIN:
		if (!bLogin)
		{
			if (!m_pDelegate ||
				m_pDelegate->onClientLogin(this, epFrom, ptr->getCustomData(), llClientLoginId))
			{
				CContainer::add(llClientLoginId, epFrom);
				CP2PMessagePtr ptrLoginSuccess(m_pDataSource->createLoginSuccessMessage(this, llClientLoginId));
				sendto(pAddr, ptrLoginSuccess);
			}
		}
		break;
	case P2P_MSG_TYPE_LOGOUT:
		if (bLogin)
		{
			CContainer::remove(llClientLoginId);
			if (m_pDelegate)
			{
				m_pDelegate->onClientLogout(this, epFrom, llClientLoginId);
			}
		}
		break;
	case P2P_MSG_TYPE_PUNCH:
		if (bLogin)
		{
			CEndPoint* pTarClient = nullptr;
			if (m_pDataSource->parsePunchTargetAddr(this, ptr, pTarClient))
			{
				if (!m_pDelegate ||
					m_pDelegate->onClientPunch(this, *pTarClient, llClientLoginId, *pTarClient))
				{
					CP2PMessagePtr ptrPunch(m_pDataSource->createPunchMessage(this, epFrom.convertToStr()));
					sendto(*pTarClient, ptrPunch);
				}
			}

			if (pTarClient)
			{
				delete pTarClient;
				pTarClient = nullptr;
			}
		}
		break;
	case P2P_MSG_TYPE_DATA:
		if (bLogin && m_pDelegate)
		{
			m_pDelegate->onRecvData(this, epFrom, ptr->getCustomData(), llClientLoginId);
		}
	break;

	default:
		return;
	}
}

string CP2PServer::outputUserList()
{
	string strResps;
	lock_guard<mutex> lock(m_mtxThSfMap);
	for (auto itr = m_pThSfMapContainer->begin(); itr != m_pThSfMapContainer->end(); ++itr)
	{
		format f("%lld : %s");
		f % itr->first % itr->second.convertToStr().c_str();
		itr == m_pThSfMapContainer->begin() ?
			strResps += f.str() :
			strResps += "\n" + f.str();
	}
	return strResps;
}