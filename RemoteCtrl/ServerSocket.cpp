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
constexpr int kBufferSize = 4096;
/// <summary>
/// 用于处理从客户端接收的数据并解析成命令
/// </summary>
/// <returns></returns>
int CServerSocket::DealCMD()
{
	if (m_client == -1) return false;
	//创建缓冲区，大小为kBufferSize
	char* buffer = new char[kBufferSize];
	//初始化缓冲区大小为0
	memset(buffer, 0, kBufferSize);
	std::size_t index = 0;
	while (true) {
		std::size_t recv_buf_len = recv(m_client, buffer + index, kBufferSize - index,0);
		//如果接收失败或连接断开，返回-1
		if (recv_buf_len <= 0) return -1;
		//更新索引，表示已接收数据的长度
		index += recv_buf_len;
		//将接收到的总长度保存到 recv_buf_len
		recv_buf_len = index;
		//创建 CPacket 对象并解析接收到的数据
		m_packet = CPacket((BYTE*)buffer, recv_buf_len);
		if (recv_buf_len > 0) {
			//移动未解析的剩余数据到缓冲区开头
			memmove(buffer,buffer + recv_buf_len, kBufferSize - recv_buf_len);
			//更新索引，减去已处理的数据长度
			index -= recv_buf_len;
			return m_packet.GetCmd();
		}
	}
	return -1;
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

CPacket::CPacket():
	sHead(0),nLength(0),sCmd(0),sSum(0)
{
}

CPacket::CPacket(const CPacket&packet)
{
	sHead = packet.sHead;
	nLength = packet.nLength;
	sCmd = packet.sCmd;
	strData = packet.strData;
	sSum = packet.sSum;
}

CPacket::CPacket(const BYTE*pData, std::size_t&nSize)
{
	std::size_t i = 0;
	for (; i < nSize; i++) {
		if (*(WORD*)(pData + i) == 0xFEFF) {//循环查找包头
			sHead = *(WORD*)(pData + i);
			i += 2;
			break;
		}
	}
	//如果在找到包头后的位置i加上8个字节的偏移量超过了数据流的大小 nSize
	//则说明数据不完整
	if (i + 8 >= nSize) {    //length(4)+cmd(2)+sum(2)
		nSize = 0;
		return;
	}
	//有数据
	nLength = *(DWORD*)(pData + i); i += 4;//表示数据长度，从 pData 的位置 i 开始的4个字节。
	if (nLength + i > nSize) {//包未完全接收到，返回，解析失败
		nSize = 0;
		return;
	}
	sCmd = *(DWORD*)(pData + i);    i += 2;//表示命令长度，从 pData 的位置 i + 4 开始的2个字节
	if (nLength > 4) {
		strData.resize(nLength - 2 - 2);
		memcpy((void*)strData.c_str(),pData + i,nLength - 4);
		i += nLength - 4;
	}
	sSum = *(WORD*)(pData + i);    i += 2;
	WORD str_sum = 0;
	for (std::size_t j = 0; j < strData.size(); j++) {
		str_sum += BYTE(strData[i]) & 0xFF;
	}
	if (str_sum == sSum) {
		//nSize = nLength + 2 + 4;
		nSize = i;
		return;
	}
	nSize = 0;
}

CPacket& CPacket::operator=(const CPacket&packet)
{
	if (this != &packet) {
		sHead = packet.sHead;
		nLength = packet.nLength;
		sCmd = packet.sCmd;
		strData = packet.strData;
		sSum = packet.sSum;
	}
	return *this;
}

CPacket::~CPacket()
{

}

WORD CPacket::GetHead() const
{
	return sHead;
}

DWORD CPacket::GetLength() const
{
	return nLength;
}

WORD CPacket::GetCmd() const
{
	return sCmd;
}

const std::string& CPacket::GetData() const
{
	return strData;
}

WORD CPacket::GetSum() const
{
	return sSum;
}
