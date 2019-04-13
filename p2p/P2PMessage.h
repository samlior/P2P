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

class CP2PMessage
{
public:
	//通过标准数据包初始化,pData必须符合标准数据包格式
	CP2PMessage(CP2PMessageType type, char* pData) :m_type(type), m_pData(pData) {}
	CP2PMessage(const CP2PMessage& message) = delete;
	virtual ~CP2PMessage();

	//通过标准数据包生成消息,pData必须符合标准数据包格式
	static CP2PMessage* createMessageWithSourceData(const char* pData, size_t iLen);

	//通过自定义数据生成消息,内部会按照标准数据包格式生产消息
	static CP2PMessage* createMessageWithCustomData(CP2PMessageType type, const char* pData, size_t iLen);

	//获取类型
	CP2PMessageType getType() const { return m_type; }
	//获取完整数据
	const char* getData() const { return m_pData; }
	//获取完整长度
	size_t getLen() const { return m_pData ? strlen(m_pData) : 0; }
	//获取自定义数据
	const char* getCustomData() const;
	//获取自定义数据长度
	size_t getCustomDataLen() const;

protected:
	CP2PMessageType m_type;
	char* m_pData;
};

using CP2PMessagePtr = boost::scoped_ptr<CP2PMessage>;