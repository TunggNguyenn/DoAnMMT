#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <tchar.h>
#include <WinSock2.h>
#include <string>
#include <WS2tcpip.h>


#pragma comment (lib, "ws2_32.lib")
using namespace std;


int main()
{
	/*Khởi tạo thư viện Winsock*/
	WSADATA  wsaData;
	WORD wVersion = MAKEWORD(2, 2);

	if (WSAStartup(wVersion, &wsaData) != 0)
	{
		cout << "Version not supported\n";
	}
	else
	{
		/*Khởi tạo cấu trúc địa chỉ cho Winsock (Proxy Server)*/
		SOCKADDR_IN IPProxyServer;
		INT nPortId = 8888;	//Cổng
		IPProxyServer.sin_family = AF_INET;	//Họ địa chỉ Internet
		IPProxyServer.sin_addr.s_addr = INADDR_ANY;	//IP của máy mình
		IPProxyServer.sin_port = htons(nPortId);

		/*Khởi tạo Socket cho ProxyServer*/
		SOCKET ProxyServer;
		ProxyServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	//Tại Socket cho ProxyServer
		bind(ProxyServer, (SOCKADDR*)&IPProxyServer, sizeof(IPProxyServer));	//Gán IP PORT vào Sokect của ProxyServer

		if (listen(ProxyServer, 5) == 0)
		{
			cout << "ProxyServer dang lang nghe ket noi tu Client...\n";
			SOCKET Browser;	//Socket của Browser
			SOCKADDR_IN IPBrowser;
			INT BrowserAddrLen = sizeof(IPBrowser);
			Browser = accept(ProxyServer, (SOCKADDR*)&IPBrowser, &BrowserAddrLen);
			if (Browser == -1)
			{
				cout << "Loi ket noi tu Client\n";

			}
			else
			{
				cout << "Chap nhan ket noi tu Client thanh cong.\n";
				char *headerRequest = NULL;
				char *bodyRequest = NULL;
				int size = 0;

				bool Method = 0;	//GET: 0, POST: 1
				bool flag = 0;	//Kiểm tra khoảng cách giữa headerRequest và bodyRequest

				/*LẤY PHẦN headerRequest TỪ BROWSER*/
				char c;
				int iResult = recv(Browser, &c, sizeof(c), 0);
				if (iResult == 1)
				{
					if (c == 'G')
					{
						Method = 0;
					}
					else if (c == 'P')
					{
						Method = 1;
					}

					size++;
					headerRequest = (char*)realloc(headerRequest, size);
					headerRequest[size - 1] = c;
					cout << c;
				}

				do
				{
					iResult = recv(Browser, &c, sizeof(c), 0);
					cout << c;

					if (iResult == 1)
					{
						size++;
						headerRequest = (char*)realloc(headerRequest, size);
						headerRequest[size - 1] = c;

						if (c != '\r')
						{
							flag = 0;
						}
						else
						{
							iResult = recv(Browser, &c, sizeof(c), 0);
							cout << c;

							if (c == '\n')
							{
								size++;
								headerRequest = (char*)realloc(headerRequest, size);
								headerRequest[size - 1] = c;
								if (flag == 0)
								{
									flag = 1;
								}
								else
								{
									break;
								}
							}
						}
					}
					else
					{
						break;
					}

				} while (1);

				size++;
				headerRequest = (char*)realloc(headerRequest, size);
				headerRequest[size - 1] = '\0';
				size--;


				char *HostName = NULL;
				int hostnameLen = 0;

				for (int i = 0; i < size; i++)
				{
					if (headerRequest[i] == 'H')
					{
						i++;
						if (headerRequest[i] == 'o')
						{
							i++;
							if (headerRequest[i] == 's')
							{
								i++;
								if (headerRequest[i] == 't')
								{
									i++;
									if (headerRequest[i] == ':')
									{
										i++;
										for (int j = i + 1; j < size; j++)
										{
											if (headerRequest[j] != '\r')
											{
												hostnameLen++;
												HostName = (char*)realloc(HostName, hostnameLen);
												HostName[hostnameLen - 1] = headerRequest[j];
							
											}
											else
											{
												break;
											}
										}
										hostnameLen++;
										HostName = (char*)realloc(HostName, hostnameLen);
										HostName[hostnameLen - 1] = '\0';
										break;
									}
									else
									{
										continue;
									}
								}
								else
								{
									continue;
								}
							}
							else
							{
								continue;
							}
						}
						else
						{
							continue;
						}
					}
					else
					{
						continue;
					}
				}

				/*Chuyển Domain sang địa chỉ IP*/
				hostent *ConnectIP = NULL;
				ConnectIP = gethostbyname(HostName);

				if (!ConnectIP)	//Xem xét lại
				{
					cout << "DNS khong the phan giai duoc ten mien nay...\n";
				}

				/*Khởi tạo cấu trúc địa chỉ cho Server*/
				SOCKADDR_IN IPServer;
				INT PortServer = 80;	//Cổng 
				IPServer.sin_family = AF_INET;	//Họ địa chỉ Internet
				IPServer.sin_addr.s_addr = (*(DWORD*)ConnectIP->h_addr_list[0]);	//IP của server
				IPServer.sin_port = htons(PortServer);



				/*Tạo Socket để kết nối tới Server qua mạng Internet tại port 80*/
				SOCKET Server;
				Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				iResult = connect(Server, (SOCKADDR*)&IPServer, sizeof(IPServer));

				if (iResult != 0)
				{
					cout << "Loi ket noi den Server tu ProxyServer\n";
				}
				else
				{
					send(Server, headerRequest, size, 0);


				}
			}
			
		
			closesocket(Browser);
		}


		closesocket(ProxyServer);

	}


	WSACleanup();
	system("pause");
}