﻿#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <tchar.h>
#include <WinSock2.h>
#include <string>
#include <Windows.h>
#include <fstream>



#pragma comment (lib, "ws2_32.lib")
using namespace std;

/*Lấy Host từ headerRequest*/
void getHost(char *headerRequest, int sizeRequest1, char *&HostName, int newPort)
{
	int hostnameLen = 0;

	for (int i = 0; i < sizeRequest1; i++)
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
							for (int j = i + 1; j < sizeRequest1; j++)
							{
								if (headerRequest[j] != '\r' && headerRequest[j] != ':')
								{
									hostnameLen++;
									HostName = (char*)realloc(HostName, hostnameLen);
									HostName[hostnameLen - 1] = headerRequest[j];
								}
								else if (headerRequest[j] == ':')	//Port khác ngoài Port = 80
								{
									string portFlag = "";
									for (int k = j + 1; j < sizeRequest1; k++)
									{
										if (headerRequest[k] != '\r')
										{
											portFlag = portFlag + headerRequest[k];
										}
										else
										{
											break;
										}
									}
									newPort = stoi(portFlag);
									break;
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
}

/*Kiểm tra trang Web nhập từ Browser có bị cấm hay không*/
int BlackList(char *HostName)
{
	if (HostName == NULL)
	{
		return 0;
	}
	if (strlen(HostName) < 3)
	{
		return 0;
	}

	ifstream iForbidden;
	iForbidden.open("blacklist.conf", ios::in);

	string Host;

	iForbidden.seekg(ios::beg);
	while (!iForbidden.eof())
	{
		getline(iForbidden, Host);

		if (HostName[0] == 'w' && HostName[1] == 'w' && HostName[2] == 'w')
		{
			if (Host[0] == 'w' && Host[1] == 'w' && Host[2] == 'w')
			{
				if (Host.length() != strlen(HostName))
				{
					continue;
				}
				else
				{
					int i;
					for (i = 0; i < Host.length(); i++)
					{
						if (Host[i] != HostName[i])
						{
							break;
						}
					}

					if (i != Host.length())
					{
						continue;
					}

					iForbidden.close();
					return 1;
				}
			}
			else
			{
				if (Host.length() != strlen(HostName) - 3)
				{
					continue;
				}
				else
				{
					int i;
					for (i = 0; i < Host.length(); i++)
					{
						if (Host[i] != HostName[i + 3])
						{
							break;
						}
					}
					if (i != Host.length())
					{
						continue;
					}

					iForbidden.close();
					return 1;
				}
			}
		}
		else
		{
			if (Host[0] == 'w' && Host[1] == 'w' && Host[2] == 'w')
			{
				if (Host.length() - 3 != strlen(HostName))
				{
					continue;
				}
				else
				{
					int i;
					for (i = 0; i < Host.length() - 3; i++)
					{
						if (Host[i + 3] != HostName[i])
						{
							break;
						}
					}
					if (i + 3 != Host.length())
					{
						continue;
					}

					iForbidden.close();
					return 1;
				}
			}
			else
			{
				if (Host.length() != strlen(HostName))
				{
					continue;
				}
				else
				{
					int i;
					for (i = 0; i < Host.length(); i++)
					{
						if (Host[i] != HostName[i])
						{
							break;
						}
					}
					if (i != Host.length())
					{
						continue;
					}

					iForbidden.close();
					return 1;
				}
			}
		}
	}
	iForbidden.close();
	return 0;
}

/*Thực thi chương trình*/
DWORD WINAPI Exe(LPVOID lpParam)
{
	int newPort = 80;	//Cổng để kết nối đến Server

	SOCKET Browser = (SOCKET)lpParam;
	cout << "Chap nhan ket noi tu Client thanh cong.\n";
	char *headerRequest = NULL;
	char *bodyRequest = NULL;
	int sizeRequest1 = 0;	//Size của header Request
	int sizeRequest2 = 0;	//Size của body Request

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
		else
		{
			closesocket(Browser);
			ExitThread(0);	//Thoát nếu ngoài phương thức GET, POST
		}

		sizeRequest1++;
		headerRequest = (char*)realloc(headerRequest, sizeRequest1);
		headerRequest[sizeRequest1 - 1] = c;
		cout << c;
	}

	do
	{
		iResult = recv(Browser, &c, sizeof(c), 0);
		cout << c;

		if (iResult == 1)
		{
			sizeRequest1++;
			headerRequest = (char*)realloc(headerRequest, sizeRequest1);
			headerRequest[sizeRequest1 - 1] = c;

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
					sizeRequest1++;
					headerRequest = (char*)realloc(headerRequest, sizeRequest1);
					headerRequest[sizeRequest1 - 1] = c;
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

	sizeRequest1++;
	headerRequest = (char*)realloc(headerRequest, sizeRequest1);
	headerRequest[sizeRequest1 - 1] = '\0';
	sizeRequest1--;

	/*Lây PHẦN Body Request*/
	if (Method == 1)
	{
		char HTMLResquest[513] = { 0 };

		do
		{
			iResult = recv(Browser, HTMLResquest, sizeof(HTMLResquest) - 1, 0);	//Đọc 512 Bytes

			//Loại bỏ 1 ký tự bị thừa từ HEADER
			if (HTMLResquest[0] == '\0')
			{
				HTMLResquest[0] = ' ';
			}

			if (iResult < 512)	//Kết thúc HTML
			{
				HTMLResquest[iResult] = '\0';
				cout << HTMLResquest;
				for (int i = 0; i < iResult; i++)
				{
					sizeRequest2++;
					bodyRequest = (char*)realloc(bodyRequest, sizeRequest2);
					bodyRequest[sizeRequest2 - 1] = HTMLResquest[i];
				}
				break;
			}

			HTMLResquest[iResult] = '\0';	//Ngắt chuỗi nếu RESPONSE nhận < 512 BYTES hoặc cuối cùng để in chuỗi
			cout << HTMLResquest;
			for (int i = 0; i < iResult; i++)
			{
				sizeRequest2++;
				bodyRequest = (char*)realloc(bodyRequest, sizeRequest2);
				bodyRequest[sizeRequest2 - 1] = HTMLResquest[i];
			}

		} while (iResult > 0);

		sizeRequest2++;
		bodyRequest = (char*)realloc(bodyRequest, sizeRequest2);
		bodyRequest[sizeRequest2 - 1] = '\0';
		sizeRequest2--;
	}

	char *HostName = NULL;

	/*Lấy Host và Port từ headerRequest*/
	getHost(headerRequest, sizeRequest1, HostName, newPort);	

	/*Kiểm tra những trang Web cấm*/
	int blackList = BlackList(HostName);
	

	if (blackList == 1)	//Nếu blackList = 1 thì có trong BlackList.conf. Ngược lại thì không...
	{
		char Forbidden[] = "HTTP/1.1 403 Forbidden\r\n\r\n"
			"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
			"<html><head>\r\n"
			"<title>403 Forbidden</title>\r\n"
			"</head><body>\r\n"
			"<h1>403 Forbidden</h1>\r\n"
			"<p>You don't have permission to access /forbidden/\r\n"
			"on this server.</p>\r\n"
			"</body></html>\r\n";
		cout << Forbidden;
		send(Browser, Forbidden, 251, 0);

		/*Xóa headerRequest, bodyRequest*/
		if (headerRequest != NULL)
		{
			delete headerRequest;
		}
		if (bodyRequest != NULL)
		{
			delete bodyRequest;
		}
		closesocket(Browser);
		ExitThread(0);
	}


	/*Chuyển Domain sang địa chỉ IP*/
	hostent *ConnectIP = NULL;
	ConnectIP = gethostbyname(HostName);

	if (!ConnectIP)	//Xem xét lại
	{
		cout << "DNS khong the phan giai duoc ten mien nay...\n";
		/*Xóa headerRequest, bodyRequest*/
		if (headerRequest != NULL)
		{
			delete headerRequest;
		}
		if (bodyRequest != NULL)
		{
			delete bodyRequest;
		}
		closesocket(Browser);
		ExitThread(0);
	}

	/*Khởi tạo cấu trúc địa chỉ cho Server*/
	SOCKADDR_IN IPServer;
	INT PortServer = 80;	//Cổng 
	if (newPort > 0)
	{
		PortServer = newPort;	//Cổng 
	}
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
		/*Xóa headerRequest, bodyRequest*/
		if (headerRequest != NULL)
		{
			delete headerRequest;
		}
		if (bodyRequest != NULL)
		{
			delete bodyRequest;
		}

		closesocket(Server);	//Đóng socket của Server
		closesocket(Browser);	//Đóng sockett của Browser
		ExitThread(0);
	}
	else
	{
		send(Server, headerRequest, sizeRequest1, 0);	//Gửi header Request lên Web Server
		send(Server, bodyRequest, sizeRequest2, 0);	//Gửi body Request lên Web Server

		/*Nhận header từ Server*/
		char *headerResponse = NULL;
		char *bodyResponse = NULL;
		int sizeResponse = 0;

		//
		do
		{
			iResult = recv(Server, &c, sizeof(c), 0);
			cout << c;

			if (iResult == 1)
			{
				sizeResponse++;
				headerResponse = (char*)realloc(headerResponse, sizeResponse);
				headerResponse[sizeResponse - 1] = c;

				if (c != '\r')
				{
					flag = 0;
				}
				else
				{
					iResult = recv(Server, &c, sizeof(c), 0);
					cout << c;

					if (c == '\n')
					{
						sizeResponse++;
						headerResponse = (char*)realloc(headerResponse, sizeResponse);
						headerResponse[sizeResponse - 1] = c;
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

		sizeResponse++;
		headerResponse = (char*)realloc(headerResponse, sizeResponse);
		headerResponse[sizeResponse - 1] = '\0';
		sizeResponse--;

		send(Browser, headerResponse, sizeResponse, 0);  //

		char HTMLResponse[513] = { 0 };

		do
		{
			iResult = recv(Server, HTMLResponse, sizeof(HTMLResponse) - 1, 0);	//Đọc 512 Bytes

			//Loại bỏ 1 ký tự bị thừa từ HEADER
			if (HTMLResponse[0] == '\0')
			{
				HTMLResponse[0] = ' ';
			}

			if (iResult < 512)	//Kết thúc HTML
			{
				HTMLResponse[iResult] = '\0';
				cout << HTMLResponse;
				send(Browser, HTMLResponse, iResult, 0);  //
				break;
			}

			HTMLResponse[iResult] = '\0';	//Ngắt chuỗi nếu RESPONSE nhận < 512 BYTES hoặc cuối cùng để in chuỗi
			cout << HTMLResponse;
			send(Browser, HTMLResponse, iResult, 0);  //

		} while (iResult > 0);


		/*Xóa headerRequest, bodyRequest, headerResponse, bodyResponse*/
		if (headerRequest != NULL)
		{
			delete headerRequest;
		}
		if (bodyRequest != NULL)
		{
			delete bodyRequest;
		}
		if (headerResponse != NULL)
		{
			delete headerResponse;
		}
		if (bodyResponse != NULL)
		{
			delete bodyResponse;
		}
	}

	closesocket(Server);
	closesocket(Browser);
	ExitThread(0);
}


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


		if (listen(ProxyServer, SOMAXCONN) == 0)
		{
			do
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
					CloseHandle(CreateThread(NULL, 0, Exe, (LPVOID)Browser, 0, NULL));	//Thực thi xong chương trình là xóa Thread ngay
				}
			} while (1);
		}
		else
		{
			cout << "Loi listen cua ProxyServer" << endl;
		}

		closesocket(ProxyServer);
	}


	WSACleanup();
	system("pause");
}