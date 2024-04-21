#include "pch.h"
#include "ServerSocket.h"
CServerSocket* CServerSocket::m_instance = nullptr;
CServerSocket* pServerSocket = CServerSocket::GetInstance();//单例对象
CServerSocket::CHelper CServerSocket::helper;
/// <summary>
/// 初始化套接字
/// </summary>
/// <returns></returns>
bool CServerSocket::InitSocket()
{
	//SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_serv_sock == -1) return false;
	sockaddr_in serv_adr{};
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.S_un.S_addr = INADDR_ANY;
	serv_adr.sin_port = htons(9527);
	if (bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) return false;
	if (listen(m_serv_sock, 1) == -1) return false;
	return true;
}
/// <summary>
/// 等待客户端连接
/// </summary>
/// <returns></returns>
bool CServerSocket::GetAccept()
{
	sockaddr_in client_adr{};
	int client_size = sizeof(client_adr);
	m_client = accept(m_serv_sock, (sockaddr*)&client_adr, &client_size);
	if (m_client == -1) return false;
	return true;
}
int CServerSocket::DealCMD()
{
	if (m_client == -1) return false;
	char buffer[1024]{};
	while (true) {
		auto recv_buf = recv(m_client, buffer, sizeof(buffer),0);
		if (recv_buf <= 0) return -1;
		//todo:处理命令
	}
}
bool CServerSocket::Send(const char* pData, std::size_t nSize)
{
	return send(m_client, pData, nSize, 0) > 0 ? true : false;
}
CServerSocket::CServerSocket()
{
	m_client = INVALID_SOCKET;
	if (InitWSASocket() == FALSE) {
		MessageBox(NULL, _T("无法初始化套接字环境,请检查网络环境设置。"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_serv_sock = socket(PF_INET, SOCK_STREAM, 0);

}

CServerSocket::~CServerSocket()
{
	closesocket(m_serv_sock);
	WSACleanup();
}
/// <summary>
/// 初始化套接字环境
/// </summary>
/// <returns></returns>
BOOL CServerSocket::InitWSASocket()
{
	WSADATA data{};
	if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
		return FALSE;
	}
	return TRUE;
}

void CServerSocket::releaseInstance()
{
	if (m_instance != nullptr) {
		auto tmp = m_instance;
		m_instance = nullptr;
		delete tmp;
	}
}
