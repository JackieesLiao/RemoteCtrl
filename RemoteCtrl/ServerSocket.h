#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H
#include "pch.h"
#include "framework.h"
#include <string>
//���ݰ�����
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
	WORD sHead;         //��ͷ���̶�λFE FF
	DWORD nLength;      //�������ӿ������ʼ����У��ͽ���
	WORD sCmd;          //��������
	std::string strData;//����������
	WORD sSum;          //��У��
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
	static void releaseInstance();//����ʵ��
	static CServerSocket* m_instance;
	//ר�Ź���һ������������
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
