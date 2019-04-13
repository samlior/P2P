#include <iostream>
#include "P2PServer.h"
#include <boost/scoped_ptr.hpp>

#define RECV_BUFSIZE	1024
#define BIND_IP			"0.0.0.0"
#define LISTEN_PORT		62000

using namespace std;
using namespace boost;

//ͨ���̳еķ�ʽ�Զ���delegate
class CMyP2PServerDelegate : public CP2PServerDelegate
{
public:
	virtual bool onClientLogin(CP2PServer* pServer, const CEndPoint& endpoint, const char* pData, long long& llLoginId) override
	{
		cout << "login success : " << endpoint.convertToStr() << endl;
		return CP2PServerDelegate::onClientLogin(pServer, endpoint, pData, llLoginId);
	}
	virtual void onClientLogout(CP2PServer* pServer, const CEndPoint& endpoint, long long llLoginId) override
	{
		cout << "logout success : " << endpoint.convertToStr() << endl;
	}
	virtual bool onClientPunch(CP2PServer* pServer, const CEndPoint& epSrc, long long llLoginId, const CEndPoint& epTar) override
	{ 
		cout << epSrc.convertToStr() << " want to punch with " << epTar.convertToStr() << endl;
		return true; 
	}
	virtual void onRecvData(CP2PServer* pServer, const CEndPoint& endpoint, const char* pData, long long llLoginId) override
	{
		cout << "recv from client : " << pData << endl;
	}
};

int main()
{
	//ȫ�ֳ�ʼ��
	CUDPSocket::startUp();

	//��������������,���������ip���˿�
	scoped_ptr<CP2PServerDataSource> pDataSource(CP2PServerDataSource::createDefaultDataSource());
	scoped_ptr<CMyP2PServerDelegate> pDelegate(new CMyP2PServerDelegate);
	scoped_ptr<CP2PServer> pServer(new CP2PServer(pDataSource.get(), LISTEN_PORT, BIND_IP));
	//���ô���
	pServer->setDelegate(pDelegate.get());
	//��ʼ��������
	if (!pServer->init())
	{
		cout << "server init failed" << endl;
		return 0;
	}

	//����ָ��
	char line[RECV_BUFSIZE] = { 0 };
	while (fprintf(stdout, ">>> ")) {

		string strLine;
		getline(cin, strLine);
		strcat_s(line, strLine.c_str());

		char *cmd = strtok(line, " ");
		if (strncmp(cmd, "send", 4) == 0) {
			char *host_port = strtok(NULL, " ");
			char *data = strtok(NULL, "\n");
			pServer->send(host_port, data);
		}
		else if (strncmp(cmd, "list", 4) == 0) {
			cout << pServer->outputUserList() << endl;
		}
		else if (strncmp(cmd, "quit", 4) == 0) {
			break;
		}
		else {
			printf("Unknown command %s\n", cmd);
		}

		memset(line, 0, RECV_BUFSIZE);
	}

	//�ͷ�socket��Դ
	CUDPSocket::clearUp();
	return 0;
}