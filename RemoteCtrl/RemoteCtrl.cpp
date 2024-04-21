﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#pragma comment(linker,"/subsystem:windows /entry:WinMainCRTStartup")
//#pragma comment(linker,"/subsystem:windows /entry:mainCRTStartup")
//#pragma comment(linker,"/subsystem:console /entry:mainCRTStartup")
//#pragma comment(linker,"/subsystem:console /entry:WinMainCRTStartup")

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            auto pServerSock = CServerSocket::GetInstance();
            auto count_accept_error{ 0 }; //统计accept异常次数
            if (!pServerSock->InitSocket()) {
                MessageBox(nullptr, _T("网络初始化异常，未能成功初始化，请检查网路状态。"), _T("网络初始化失败。"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (CServerSocket::GetInstance() != nullptr) {
                if (!pServerSock->GetAccept()) {
                    if (count_accept_error >= 3) {
                        MessageBox(nullptr, _T("多次无法正常接入用户，程序将自动退出。"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(nullptr,_T("无法正常接入用户，自动重试<<<<"),_T("接入用户失败！"), MB_OK | MB_ICONERROR);
                    count_accept_error++;
                }
                auto cmd_ret = pServerSock->DealCMD();
            }
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
