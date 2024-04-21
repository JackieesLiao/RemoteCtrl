#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H
#include "pch.h"
#include "framework.h"
#include <string>
//数据包定义
class CPacket {
public:
	CPacket();
	CPacket(const CPacket&);
	CPacket(const BYTE*, std::size_t&);
	CPacket& operator=(const CPacket&);
	~CPacket();
	WORD GetHead() const;
	DWORD GetLength() const;
	WORD GetCmd() const;
	const std::string& GetData() const;
	WORD GetSum() const;
private:
	WORD sHead;         //包头：固定位FE FF
	DWORD nLength;      //包长：从控制命令开始，到校验和结束
	WORD sCmd;          //控制命令
	std::string strData;//包数据内容
	WORD sSum;          //和校验
};
class CServerSocket
{
	friend class CPacket;
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
	CPacket m_packet;
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
