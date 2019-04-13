#include "P2PMessage.h"
#include "UDPSocket.h"
#include <boost/format.hpp>

#define MSG_TYPE_HEADER_LEN			2
#define MSG_DATA_LEN_HEADER_LEN		4
#define MSG_HEADER_LEN				(MSG_TYPE_HEADER_LEN + MSG_DATA_LEN_HEADER_LEN)

using namespace std;
using namespace boost;

CP2PMessage::~CP2PMessage()
{
	if (m_pData)
	{
		delete m_pData;
		m_pData = nullptr;
	}
}

CP2PMessage* CP2PMessage::createMessageWithSourceData(const char* pData, size_t iLen)
{
	CP2PMessage* pMessage = new CP2PMessage(P2P_MSG_TYPE_UNKONW, nullptr);
	if (pData && iLen >= MSG_HEADER_LEN)
	{
		char pMsgType[MSG_TYPE_HEADER_LEN] = { 0 };
		memcpy_s(pMsgType, MSG_TYPE_HEADER_LEN, pData, MSG_TYPE_HEADER_LEN);
		int iMsgType = atoi(pMsgType);
		if (iMsgType)
		{
			char pMsgDataLen[MSG_DATA_LEN_HEADER_LEN] = { 0 };
			memcpy_s(pMsgDataLen, MSG_DATA_LEN_HEADER_LEN, pData + MSG_TYPE_HEADER_LEN, MSG_DATA_LEN_HEADER_LEN);
			int iDataLen = atoi(pMsgDataLen);
			if (iDataLen == iLen - MSG_HEADER_LEN)
			{
				pMessage->m_type = static_cast<CP2PMessageType>(iMsgType);
				pMessage->m_pData = new char[iLen + 1];
				memset(pMessage->m_pData, 0, iLen + 1);
				memcpy_s(pMessage->m_pData, iLen, pData, iLen);
			}
		}
	}
	return pMessage;
}

CP2PMessage* CP2PMessage::createMessageWithCustomData(CP2PMessageType type, const char* pData, size_t iLen)
{
	char* pCustomData = new char[MSG_HEADER_LEN + iLen + 1];
	memset(pCustomData, 0, MSG_HEADER_LEN + iLen + 1);
	format f("%02d%04d");
	f % static_cast<int>(type) % iLen;
	memcpy_s(pCustomData, MSG_HEADER_LEN, f.str().c_str(), MSG_HEADER_LEN);
	if (pData && iLen)
		memcpy_s(pCustomData + MSG_HEADER_LEN, iLen, pData, iLen);
	return new CP2PMessage(type, pCustomData);
}

size_t CP2PMessage::getCustomDataLen() const
{
	size_t len = 0;
	if (m_pData)
	{
		len = strlen(m_pData);
		if (len >= MSG_HEADER_LEN)
		{
			len -= MSG_HEADER_LEN;
		}
		else
		{
			len = 0;
		}
	}
	return len;
}

const char* CP2PMessage::getCustomData() const
{
	return m_pData && strlen(m_pData) > MSG_HEADER_LEN ? m_pData + MSG_HEADER_LEN : nullptr;
}