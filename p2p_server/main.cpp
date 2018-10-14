
#include <iostream>
#include "P2PServer.h"

using namespace std;


//通过继承的方式自定义delegate
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
		return;
	}
	virtual bool onClientPunch(CP2PServer* pServer, const CEndPoint& epSrc, long long llLoginId, const CEndPoint& epTar) override
	{ 
		cout << epSrc.convertToStr() << " want to punch with " << epTar.convertToStr() << endl;
		return true; 
	}
	virtual void onRecvData(CP2PServer* pServer, const CEndPoint& endpoint, const char* pData, long long llLoginId) override
	{
		cout << "recv from client : " << pData << endl;
		return;
	}
};




#define RECV_BUFSIZE 1024

int main()
{
	//全局初始化
	CUDPSocket::startUp();

	//创建服务器对象,传入监听的ip及端口
	CP2PServerDataSource* pDataSource = new CP2PServerDataSource;
	CMyP2PServerDelegate* pDelegate = new CMyP2PServerDelegate;
	CP2PServer* pServer = new CP2PServer(pDataSource, 62000, "0.0.0.0");
	//设置代理
	pServer->setDelegate(pDelegate);
	//初始化服务器
	if (!pServer->init())
	{
		cout << "server init failed" << endl;
		return 0;
	}


	//接收指令
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
	free(line);

	delete pServer;
	pServer = nullptr;
	delete pDataSource;
	pDataSource = nullptr;
	delete pDelegate;
	pDelegate = nullptr;

	//释放socket资源
	CUDPSocket::clearUp();
	return 0;
}