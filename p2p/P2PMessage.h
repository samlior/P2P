#pragma once
#include <string>
#include <boost/scoped_ptr.hpp>

enum CP2PMessageType
{
	//�򶴳ɹ���Ϣ,�ͻ����յ�����Ϣ����Ϊ�򶴳ɹ�
	P2P_MSG_TYPE_REPLY = 1,

	//����Ϣ
	P2P_MSG_TYPE_PUNCH,

	//����������Ϣ
	P2P_MSG_TYPE_DATA,

	//��¼����
	P2P_MSG_TYPE_LOGIN,

	//��¼�ɹ���Ϣ
	P2P_MSG_TYPE_LOGIN_SUCCESS,

	//�ǳ�����
	P2P_MSG_TYPE_LOGOUT,

	//������
	P2P_MSG_TYPE_PING,

	//�����ذ�
	P2P_MSG_TYPE_PONG,

	//δ֪��Ϣ
	P2P_MSG_TYPE_UNKONW
};


class CP2PMessage;
using CP2PMessagePtr = boost::scoped_ptr<CP2PMessage>;


struct CEndPoint;

class CP2PMessage
{
public:
	CP2PMessage(CP2PMessageType type, char* pData) :m_type(type), m_pData(pData) {}
	CP2PMessage(const CP2PMessage& message) = delete;
	~CP2PMessage()
	{
		if (m_pData)
		{
			delete m_pData;
			m_pData = nullptr;
		}
	}

	static CP2PMessage* createMessageWithSourceData(const char* pData, size_t iLen);
	static CP2PMessage* createMessageWithCustomData(CP2PMessageType type, const char* pData, size_t iLen);

	CP2PMessageType getType() { return m_type; }
	const char* getData() { return m_pData; }
	const char* getCustomData();
	size_t getLen() { return m_pData ? strlen(m_pData) : 0; }

protected:
	CP2PMessageType m_type;
	char* m_pData;
};