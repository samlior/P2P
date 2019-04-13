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

class CP2PMessage
{
public:
	//ͨ����׼���ݰ���ʼ��,pData������ϱ�׼���ݰ���ʽ
	CP2PMessage(CP2PMessageType type, char* pData) :m_type(type), m_pData(pData) {}
	CP2PMessage(const CP2PMessage& message) = delete;
	virtual ~CP2PMessage();

	//ͨ����׼���ݰ�������Ϣ,pData������ϱ�׼���ݰ���ʽ
	static CP2PMessage* createMessageWithSourceData(const char* pData, size_t iLen);

	//ͨ���Զ�������������Ϣ,�ڲ��ᰴ�ձ�׼���ݰ���ʽ������Ϣ
	static CP2PMessage* createMessageWithCustomData(CP2PMessageType type, const char* pData, size_t iLen);

	//��ȡ����
	CP2PMessageType getType() const { return m_type; }
	//��ȡ��������
	const char* getData() const { return m_pData; }
	//��ȡ��������
	size_t getLen() const { return m_pData ? strlen(m_pData) : 0; }
	//��ȡ�Զ�������
	const char* getCustomData() const;
	//��ȡ�Զ������ݳ���
	size_t getCustomDataLen() const;

protected:
	CP2PMessageType m_type;
	char* m_pData;
};

using CP2PMessagePtr = boost::scoped_ptr<CP2PMessage>;