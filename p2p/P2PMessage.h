#pragma once
#include <string>
#include <boost/scoped_ptr.hpp>

enum CP2PMessageType
{
	//打洞成功消息,客户端收到此消息则认为打洞成功
	P2P_MSG_TYPE_REPLY = 1,

	//打洞消息
	P2P_MSG_TYPE_PUNCH,

	//数据类型消息
	P2P_MSG_TYPE_DATA,

	//登录请求
	P2P_MSG_TYPE_LOGIN,

	//登录成功消息
	P2P_MSG_TYPE_LOGIN_SUCCESS,

	//登出请求
	P2P_MSG_TYPE_LOGOUT,

	//心跳包
	P2P_MSG_TYPE_PING,

	//心跳回包
	P2P_MSG_TYPE_PONG,

	//未知消息
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