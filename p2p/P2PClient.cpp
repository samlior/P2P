#include "P2PClient.h"
#include <boost/format.hpp>

using namespace std;
using namespace boost;

bool CP2PAliveKeeper::run()
{
	if (m_funcCallBack)
	{
		lock_guard<mutex> lock(m_mtxThSfMap);
		string strData;
		for (auto itr = m_pThSfMapContainer->begin(); itr != m_pThSfMapContainer->end(); ++itr)
		{
			m_funcCallBack(itr->second, strData);
			m_pSocket->sendto(itr->second, strData.c_str());
		}
	}
	return CUDPAliveKeeper::run();
}



bool CP2PClientDataSource::parseLoginSuccessLoginId(CP2PClient* pClient, const CP2PMessage* pMessage, long long& ll)
{
	const char* pCustomData = pMessage->getCustomData();
	if (pCustomData)
		ll = atoi(pCustomData);
	return ll > 0;
}

bool CP2PClientDataSource::parseLoginSuccessLoginId(CP2PClient* pClient, const CP2PMessagePtr& ptr, long long& ll)
{
	return parseLoginSuccessLoginId(pClient, ptr.get(), ll);
}

bool CP2PClientDataSource::parseReplyLoginId(CP2PClient* pClient, const CP2PMessage* pMessage, long long& ll)
{
	const char* pCustomData = pMessage->getCustomData();
	if (pCustomData)
		ll = atoi(pCustomData);
	return ll > 0;
}

bool CP2PClientDataSource::parseReplyLoginId(CP2PClient* pClient, const CP2PMessagePtr& ptr, long long& ll)
{
	return parseReplyLoginId(pClient, ptr.get(), ll);
}

bool CP2PClientDataSource::parsePunchTargetAddr(CP2PClient* pClient, const CP2PMessage* pMessage, CEndPoint& endpoint)
{
	const char* pCustomData = pMessage->getCustomData();
	bool bRet = false;
	if (pCustomData)
	{
		try
		{
			endpoint.convertFromStr(pCustomData);
			bRet = true;
		}
		catch (const CEndPointConvertError&)
		{
			//etc...
		}
	}
	return bRet;
}

bool CP2PClientDataSource::parsePunchTargetAddr(CP2PClient* pClient, const CP2PMessagePtr& ptr, CEndPoint& endpoint)
{
	return parsePunchTargetAddr(pClient, ptr.get(), endpoint);
}



CP2PMessage* CP2PClientDataSource::createLoginMessage(CP2PClient* pClient)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_LOGIN, nullptr, 0);
}

CP2PMessage* CP2PClientDataSource::createLogoutMessage(CP2PClient* pClient)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_LOGOUT, nullptr, 0);
}

CP2PMessage* CP2PClientDataSource::createPingMessage(CP2PClient* pClient)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_PING, nullptr, 0);
}

CP2PMessage* CP2PClientDataSource::createPongMessage(CP2PClient* pClient)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_PONG, nullptr, 0);
}

CP2PMessage* CP2PClientDataSource::createPunchToClientMessage(CP2PClient* pClient)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_PUNCH, nullptr, 0);
}

CP2PMessage* CP2PClientDataSource::createPunchToServerMessage(CP2PClient* pClient, const string& strTarAddr)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_PUNCH, strTarAddr.c_str(), strTarAddr.size());
}

CP2PMessage* CP2PClientDataSource::createReplyMessage(CP2PClient* pClient, long long llLoginId)
{
	string strLoginId = to_string(llLoginId);
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_REPLY, strLoginId.c_str(), strLoginId.size());
}

CP2PMessage* CP2PClientDataSource::createDataMessage(CP2PClient* pClient, const CEndPoint& endpoint, const string& strData)
{
	return CP2PMessage::createMessageWithCustomData(P2P_MSG_TYPE_DATA, strData.c_str(), strData.size());
}

CP2PMessage* CP2PClientDataSource::parseSourceData(CP2PClient* pClient, const char* pData, size_t iLen)
{
	return CP2PMessage::createMessageWithSourceData(pData, iLen);
}



bool CP2PClient::init()
{
	if (!CUDPSocket::init())
		return false;

	CUDPSocket::setRecvfromCallBack(bind(&CP2PClient::recvData, this, placeholders::_1, placeholders::_2, placeholders::_3));

	if (!CUDPSocket::startRecvfrom())
		return false;

	m_keeper.setAppendDataCallBack([this](const CEndPoint&, string& str) 
	{
		CP2PMessagePtr ptrPing(m_pDataSource->createPingMessage(this));
		str = ptrPing->getData();
	});

	if (!startKeepAlive())
		return false;

	return true;
}

bool CP2PClient::reset()
{
	close();
	return CUDPSocket::reset();
}

void CP2PClient::close()
{
	CUDPSocket::stopRecvfrom();
	stopKeepAlive();
	CUDPSocket::close();
}

bool CP2PClient::send(const string& strTarAddr, const string& strData)
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

bool CP2PClient::sendto(const CEndPoint& endpoint, const CP2PMessage* pMessage)
{
	return CUDPSocket::sendto(endpoint, pMessage->getData());
}

bool CP2PClient::sendto(const CEndPoint& endpoint, const CP2PMessagePtr& ptr)
{
	return sendto(endpoint, ptr.get());
}

void CP2PClient::recvData(const char* pData, size_t iLen, sockaddr_in* pAddr)
{
	CP2PMessagePtr ptr(m_pDataSource->parseSourceData(this, pData, strlen(pData)));

	CEndPoint epFrom(pAddr);
	if (epFrom == m_keeper.getServerAddr())
	{
		switch (ptr->getType())
		{
		case P2P_MSG_TYPE_PUNCH:
		{
			CEndPoint srcClient;
			if (m_pDataSource->parsePunchTargetAddr(this, ptr, srcClient))
			{
				CP2PMessagePtr ptrPunch(m_pDataSource->createReplyMessage(this, m_llLoginId));
				sendto(srcClient, ptrPunch);
			}
		}
			break;
		case P2P_MSG_TYPE_DATA:
		{
			if (m_pDelegate)
			{
				m_pDelegate->onRecvDataFromServer(this, ptr->getCustomData());
			}
		}
			break;
		case P2P_MSG_TYPE_LOGIN_SUCCESS:
		{
			if (m_pDataSource->parseLoginSuccessLoginId(this, ptr, m_llLoginId) && m_pDelegate)
			{
				m_pDelegate->onLoginSuccess(this, m_llLoginId);
			}
		}
			break;
		default:
			return;
		}
	}
	else
	{
		switch (ptr->getType())
		{
		case P2P_MSG_TYPE_REPLY:
		{
			long long llClientLoginId = 0;
			if (m_pDataSource->parseReplyLoginId(this, ptr, llClientLoginId))
			{
				m_keeper.add(llClientLoginId, epFrom);
				if (m_pDelegate)
				{
					m_pDelegate->onPunchSuccess(this, epFrom, llClientLoginId);
				}
			}
		}
			break;
		case P2P_MSG_TYPE_DATA:
		{
			if (m_pDelegate)
			{
				m_pDelegate->onRecvDataFromClient(this, epFrom, ptr->getCustomData());
			}
		}
			break;
		case P2P_MSG_TYPE_PING:
		{
			CP2PMessagePtr ptrPong(m_pDataSource->createPongMessage(this));
			sendto(pAddr, ptrPong);
		}
			break;
		default:
			return;
		}
	}
}

bool CP2PClient::login()
{
	CP2PMessagePtr ptrLogin(m_pDataSource->createLoginMessage(this));
	return sendto(m_keeper.getServerAddr(), ptrLogin);
}

bool CP2PClient::logout()
{
	CP2PMessagePtr ptrLogout(m_pDataSource->createLogoutMessage(this));
	return sendto(m_keeper.getServerAddr(), ptrLogout);
}

bool CP2PClient::punch(const string& strTarAddr)
{
	try
	{
		CEndPoint endpoint(strTarAddr);
		CP2PMessagePtr ptrPunchToServer(m_pDataSource->createPunchToServerMessage(this, strTarAddr));
		if (!sendto(m_keeper.getServerAddr(), ptrPunchToServer))
			return false;

		CP2PMessagePtr ptrPunchToClient(m_pDataSource->createPunchToClientMessage(this));
		if (!sendto(endpoint, ptrPunchToClient))
			return false;
		return true;
	}
	catch (const CEndPointConvertError&)
	{
		//etc...
	}
	return false;
}

