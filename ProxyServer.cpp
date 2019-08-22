// ProxyServer.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "ProxyServer.h"
#include "MyFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object
CWinApp theApp;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			// --------------------------------------------------------------------- <START>
			// 1. Khởi động Socket trong Window
			if (AfxSocketInit(NULL) == false)
			{
				std::cerr << "Error: Can't init Socket Library :(" << std::endl;
				return 0;
			}
			// 2. Khai báo các biến
			CSocket serverSocket; // Socket lắng nghe kết nối từ Browser
			CSocket connectorSocket; // Socket giữ kết nối với Browser
			DWORD dwThreadId; // Id của tiểu trình
			HANDLE hThread; // Xử lí của tiểu trình
			// 3. Tạo Socket Server, đăng ký port là 8888, giao thức TCP
			if (serverSocket.Create(PROXY_SERVER_PORT, SOCK_STREAM, NULL) == 0)
			{
				std::cerr << "Error: Failed Proxy Server initialization :(" << std::endl;
				serverSocket.Close();
				return 0;
			}
			std::cout << "Successful Proxy Server initialization :)" << std::endl;
			// 4. Kiểm tra thời gian sống của từng file lưu đối tượng
			deleteFileOverTimeToLive();
			// 5. Vừa lắng nghe kết nối, vừa xử lí yêu cầu
			while (true)
			{
				// 5.1 Lắng nghe kết nối
				if (serverSocket.Listen(64) == false)
				{
					std::cerr << "Error: Can't listen on this port :(" << std::endl;
					serverSocket.Close();
					return 0;
				}
				// 5.2 Tạo Socket Connector giữ kết nối và trao đổi dữ liệu với Browser
				if (serverSocket.Accept(connectorSocket) == false)
				{
					std::cout << "Error: Can't connect to the Browser :(" << std::endl;
					connectorSocket.Close();
					continue;
				}
				std::cout << "Successfully connected to the Browser :)" << std::endl;
				// 5.3 Đa luồng
				SOCKET* sock = new SOCKET();
				*sock = connectorSocket.Detach();
				hThread = CreateThread(NULL, 0, Thread, (LPVOID)sock, 0, &dwThreadId);
			}
			// --------------------------------------------------------------------- <END>
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
