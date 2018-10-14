

#include <iostream>
#include "P2PClient.h"

using namespace std;


//ͨ���̳еķ�ʽ�Զ���delegate
class CMyP2PClientDelegate : public CP2PClientDelegate
{
public:
	virtual void onLoginSuccess(CP2PClient* pClient, long long llLoginId) override
	{
		cout << "login success" << endl;
	}
	virtual void onPunchSuccess(CP2PClient* pClient, const CEndPoint& endpoint, long long llRemoteLoginId) override
	{
		cout << "punch success" << endl;
	}
	virtual void onRecvDataFromServer(CP2PClient* pClient, const char* pData) override
	{
		cout << "recv from server : " << pData << endl;
	}
	virtual void onRecvDataFromClient(CP2PClient* pClient, const CEndPoint& endpoint, const char* pData) override
	{
		cout << "recv from other client : " << pData << endl;
	}
};



#define RECV_BUFSIZE 1024

int main()
{
	//ȫ�ֳ�ʼ��
	CUDPSocket::startUp();


	//�����ͻ��˶���,�����������ַ���˿�
	CP2PClientDataSource* pDataSource = new CP2PClientDataSource;
	CMyP2PClientDelegate* pDelegate = new CMyP2PClientDelegate;
	CP2PClient* pClient = new CP2PClient(pDataSource, "35.121.11.90", 62000);
	//���ô���
	pClient->setDelegate(pDelegate);
	//��ʼ���ͻ���
	if (!pClient->init())
	{
		cout << "client init failed" << endl;
		return 0;
	}


	//����ָ��
	char line[RECV_BUFSIZE] = { 0 };
	while (fprintf(stdout, ">>> ")) {

		string strLine;
		getline(cin, strLine);
		strcat_s(line, strLine.c_str());


		char *cmd = strtok(line, " ");
		if (strncmp(cmd, "login", 5) == 0) {
			pClient->login();
		}
		else if (strncmp(cmd, "logout", 5) == 0) {
			pClient->logout();
		}
		else if (strncmp(cmd, "punch", 5) == 0) {
			char *host_port = strtok(NULL, "\n");
			pClient->punch(host_port);
		}
		else if (strncmp(cmd, "send", 4) == 0) {
			char *host_port = strtok(NULL, " ");
			char *data = strtok(NULL, "\n");
			pClient->send(host_port, data);
		}
		else if (strncmp(cmd, "quit", 4) == 0) {
			pClient->logout();
			break;
		}
		else {
			printf("Unknown command %s\n", cmd);
		}

		memset(line, 0, RECV_BUFSIZE);
	}
	free(line);

	delete pClient;
	pClient = nullptr;
	delete pDataSource;
	pDataSource = nullptr;
	delete pDelegate;
	pDelegate = nullptr;

	//�ͷ�socket��Դ
	CUDPSocket::clearUp();
	return 0;
}