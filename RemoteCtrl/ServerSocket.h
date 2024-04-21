#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H
#include "pch.h"
#include "framework.h"
class CServerSocket
{
public:
	static CServerSocket* GetInstance() {
		if (m_instance == nullptr) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket();
	bool GetAccept();
	int DealCMD();
	bool Send(const char* pData, std::size_t nSize);
	CServerSocket(const CServerSocket&) = delete;
	CServerSocket& operator=(const CServerSocket&) = delete;
private:
	SOCKET m_serv_sock;
	SOCKET m_client;
private:
	CServerSocket();
	~CServerSocket();
	BOOL InitWSASocket();
	static void releaseInstance();//析构实例
	static CServerSocket* m_instance;
	//专门构造一个类用来析构
	class CHelper {
	public:
		CHelper() {
			CServerSocket::GetInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	static CHelper helper;
};

#endif // !SERVERSOCKET_H
