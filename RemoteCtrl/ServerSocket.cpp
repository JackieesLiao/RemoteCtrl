#include "pch.h"
#include "ServerSocket.h"
CServerSocket* CServerSocket::m_instance = nullptr;
CServerSocket* pServerSocket = CServerSocket::GetInstance();//��������
CServerSocket::CHelper CServerSocket::helper;
/// <summary>
/// ��ʼ���׽���
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
/// �ȴ��ͻ�������
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
/// ���ڴ���ӿͻ��˽��յ����ݲ�����������
/// </summary>
/// <returns></returns>
int CServerSocket::DealCMD()
{
	if (m_client == -1) return false;
	//��������������СΪkBufferSize
	char* buffer = new char[kBufferSize];
	//��ʼ����������СΪ0
	memset(buffer, 0, kBufferSize);
	std::size_t index = 0;
	while (true) {
		std::size_t recv_buf_len = recv(m_client, buffer + index, kBufferSize - index,0);
		//�������ʧ�ܻ����ӶϿ�������-1
		if (recv_buf_len <= 0) return -1;
		//������������ʾ�ѽ������ݵĳ���
		index += recv_buf_len;
		//�����յ����ܳ��ȱ��浽 recv_buf_len
		recv_buf_len = index;
		//���� CPacket ���󲢽������յ�������
		m_packet = CPacket((BYTE*)buffer, recv_buf_len);
		if (recv_buf_len > 0) {
			//�ƶ�δ������ʣ�����ݵ���������ͷ
			memmove(buffer,buffer + recv_buf_len, kBufferSize - recv_buf_len);
			//������������ȥ�Ѵ�������ݳ���
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
		MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���,�������绷�����á�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
/// ��ʼ���׽��ֻ���
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
		if (*(WORD*)(pData + i) == 0xFEFF) {//ѭ�����Ұ�ͷ
			sHead = *(WORD*)(pData + i);
			i += 2;
			break;
		}
	}
	//������ҵ���ͷ���λ��i����8���ֽڵ�ƫ�����������������Ĵ�С nSize
	//��˵�����ݲ�����
	if (i + 8 >= nSize) {    //length(4)+cmd(2)+sum(2)
		nSize = 0;
		return;
	}
	//������
	nLength = *(DWORD*)(pData + i); i += 4;//��ʾ���ݳ��ȣ��� pData ��λ�� i ��ʼ��4���ֽڡ�
	if (nLength + i > nSize) {//��δ��ȫ���յ������أ�����ʧ��
		nSize = 0;
		return;
	}
	sCmd = *(DWORD*)(pData + i);    i += 2;//��ʾ����ȣ��� pData ��λ�� i + 4 ��ʼ��2���ֽ�
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
