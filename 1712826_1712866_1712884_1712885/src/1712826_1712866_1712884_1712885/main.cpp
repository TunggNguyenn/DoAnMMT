#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <tchar.h>
#include <WinSock2.h>
#include <string>
#include <Windows.h>
#include <fstream>
#include <wchar.h>


#pragma comment (lib, "ws2_32.lib")
using namespace std;

/*Hàm xóa If-Modified-Since*/
void Delete_Modified(char* headerRequest)
{
	char *  Modified = strstr(headerRequest, (char*)"If-Modified-Since");
	if (Modified == NULL)
	{
		return;
	}
	int lengthModified = strlen(Modified);

	for (int i = 0; i < lengthModified; i++)
	{
		if (Modified[i] != '\n')
		{
			for (int j = i; j < lengthModified - i; j++)
			{
				Modified[j] = Modified[j + 1];
			}
		}
		else
		{
			for (int j = i; j < lengthModified - i; j++)
			{
				Modified[j] = Modified[j + 1];
			}
			break;
		}
		i--;
	}
}

//Hàm kiểm tra chuỗi sub có xuất hiện trong chuỗi str hay ko
bool FindSubStr(const char *str, char *sub)
{
	const char *p = str;
	int len = strlen(sub);

	if (str == NULL) return false;
	while (*p != NULL)
	{
		if (strlen(p) >= len)
		{
			if (strncmp(p, sub, len) == 0)
			{
				return true;
			}
		}
		else break;
		p++;
	}
	return false;
}

//Hàm lấy chuỗi trong Src, lấy tất cả các kí tự ngay phía sau chuỗi Begin cho đến khi gặp kí tự kết thúc EndCh
char* GetStr(char* Src, char* Begin, char EndCh)
{
	char* p = Src;
	int lenBegin = strlen(Begin);
	char* Res = NULL;

	while (*p != NULL)
	{
		if (strlen(p) >= lenBegin)
		{
			if (strncmp(p, Begin, lenBegin) == 0)
			{
				p += lenBegin;		//di chuyển con trỏ đến vị trí cần sao chép
				char* temp = p;
				while (*temp != EndCh && *temp != ':')		//tìm vị trí xuất hiện kí tự kết thúc
				{
					++temp;
				}
				Res = (char*)malloc(temp - p + 1);
				strncpy(Res, p, temp - p);
				Res[temp - p] = '\0';

				return Res;
			}
		}
		else break;

		++p;
	}
	return NULL;			//không tìm thấy Begin nên ko thể lấy chuỗi
}

//Hàm Kiểm tra trang Web nhập từ Browser có bị cấm hay không
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

//Hàm chuyển đổi chuỗi char* thành string
string ConvertStr(char* URL, int lenURL)
{
	int i = 0;
	string Res;

	while (i < lenURL)
	{
		Res.push_back(URL[i]);
		++i;
	}
	return Res;
}

//Hàm kiểm tra URL đã được caching chưa
bool CheckURLinList(char* URL, int lenURL)
{
	ifstream URLList;
	string str;
	string URL_string = ConvertStr(URL, lenURL);

	URLList.open("WebCache\\\\URLList.txt", ios::in);

	if (URLList.fail())
		cout << "Khong the mo tap tin URLList.txt" << endl;

	while (!URLList.eof())
	{
		//lấy từng chuỗi trên dòng
		getline(URLList, str);
		if (str == URL_string)			//kiểm tra có URL có trong tập tin
		{
			URLList.close();
			return true;
		}
	}

	URLList.close();
	return false;
}

//Tạo tên cho tập tin từ URL
char* CreateFileName(char* &URL, int &lenURL)
{
	if (lenURL > 181)
	{
		URL = (char*)realloc(URL, 181);
		URL[180] = '\0';
		lenURL = 180;
	}

	int len = lenURL + 10 + 4 + 1;
	char* URL_fileName = (char*)malloc(len);
	char str[] = "WebCache\\\\";
	int id = 0;
	int id_url_filename = 0;

	while (id < 10)
	{
		URL_fileName[id_url_filename] = str[id];
		++id;
		++id_url_filename;
	}

	id = 0;
	while (id < lenURL)
	{
		if (URL[id] == '/' || URL[id] == ':' || URL[id] == '*' || URL[id] == '?' || URL[id] == '"' || URL[id] == '<' || URL[id] == '>' || URL[id] == '|')	//thay thế các ký tự ko hợp lệ
			URL_fileName[id_url_filename] = '_';
		else
			URL_fileName[id_url_filename] = URL[id];
		++id;
		++id_url_filename;
	}
	URL_fileName[id_url_filename] = '.';
	++id_url_filename;
	URL_fileName[id_url_filename] = 't';
	++id_url_filename;
	URL_fileName[id_url_filename] = 'x';
	++id_url_filename;
	URL_fileName[id_url_filename] = 't';
	++id_url_filename;
	URL_fileName[id_url_filename] = '\0';

	return URL_fileName;
}

//Tạo file để lưu dữ liệu caching
void WriteCachingFile(char* URL_fileName, char* Str, int szStr)
{
	FILE *CachingFile;
	CachingFile = fopen(URL_fileName, "ab");
	if (!CachingFile) cout << "Khong the mo tap tin de caching" << endl;
	else
	{
		cout << "Da mo tap tin de luu du lieu caching" << endl;

		fwrite(&szStr, sizeof(int), 1, CachingFile);
		fwrite(Str, szStr, 1, CachingFile);

		fclose(CachingFile);
	}
}



/*Thực thi chương trình*/
DWORD WINAPI Exe(LPVOID lpParam)
{
	SOCKET Browser = (SOCKET)lpParam;
	cout << "Chap nhan ket noi tu Client thanh cong.\n";
	char *headerRequest = NULL;
	char *bodyRequest = NULL;
	char *HostName = NULL;
	char *URL = NULL;
	int szHeaderRequest = 0;	//Size của header Request
	int szBodyRequest = 0;	//Size của body Request

	bool Method = 0;	//GET: 0, POST: 1
	bool flag = 0;	//Kiểm tra khoảng cách giữa headerRequest và bodyRequest
	bool checkURL = 0;	//kiểm tra URL có ở trong URLList hay ko để xử lí caching

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
			cout << endl << c << endl;
			cout << "Proxy Server chi ho tro GET va POST" << endl;
			ExitThread(0);	//Thoát nếu ngoài phương thức GET, POST
		}

		szHeaderRequest++;
		headerRequest = (char*)realloc(headerRequest, szHeaderRequest);
		headerRequest[szHeaderRequest - 1] = c;
		cout << c;
	}

	do
	{
		iResult = recv(Browser, &c, sizeof(c), 0);
		cout << c;

		if (iResult == 1)
		{
			szHeaderRequest++;
			headerRequest = (char*)realloc(headerRequest, szHeaderRequest);
			headerRequest[szHeaderRequest - 1] = c;

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
					szHeaderRequest++;
					headerRequest = (char*)realloc(headerRequest, szHeaderRequest);
					headerRequest[szHeaderRequest - 1] = c;
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

	szHeaderRequest++;
	headerRequest = (char*)realloc(headerRequest, szHeaderRequest);
	headerRequest[szHeaderRequest - 1] = '\0';
	szHeaderRequest--;

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
					szBodyRequest++;
					bodyRequest = (char*)realloc(bodyRequest, szBodyRequest);
					bodyRequest[szBodyRequest - 1] = HTMLResquest[i];
				}
				break;
			}

			HTMLResquest[iResult] = '\0';	//Ngắt chuỗi nếu RESPONSE nhận < 512 BYTES hoặc cuối cùng để in chuỗi
			cout << HTMLResquest;
			for (int i = 0; i < iResult; i++)
			{
				szBodyRequest++;
				bodyRequest = (char*)realloc(bodyRequest, szBodyRequest);
				bodyRequest[szBodyRequest - 1] = HTMLResquest[i];
			}

			if (iResult < 512)	//Kết thúc HTML
				break;

		} while (iResult > 0);

		szBodyRequest++;
		bodyRequest = (char*)realloc(bodyRequest, szBodyRequest);
		bodyRequest[szBodyRequest - 1] = '\0';
		szBodyRequest--;
	}

	if (!headerRequest)		//kiểm tra headerRequest != NULL
	{
		cout << "Khong nhan duoc request" << endl;
		closesocket(Browser);
		ExitThread(0);
	}

	//Lấy Host từ headerRequest
	HostName = GetStr(headerRequest, (char*)"Host: ", '\r');

	if (!HostName)		//không thể lấy host
	{
		cout << "Khong the lay host" << endl;
		/*Xóa headerRequest, bodyRequest*/
		if (headerRequest != NULL)
		{
			free(headerRequest);
		}
		if (bodyRequest != NULL)
		{
			free(bodyRequest);
		}
		closesocket(Browser);
		ExitThread(0);
	}

	/*Kiểm tra những trang Web cấm*/
	int CheckBlackList = BlackList(HostName);

	if (CheckBlackList == 1)		//Nếu blackList = 1 thì có trong BlackList.conf. Ngược lại thì không...
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
		send(Browser, Forbidden, sizeof(Forbidden), 0);

		/*Xóa headerRequest, bodyRequest*/
		if (headerRequest != NULL)
		{
			free(headerRequest);
		}
		if (bodyRequest != NULL)
		{
			free(bodyRequest);
		}
		closesocket(Browser);
		ExitThread(0);
	}

	/*Chuyển Domain sang địa chỉ IP*/
	hostent *ConnectIP = NULL;
	ConnectIP = gethostbyname(HostName);

	if (!ConnectIP)			//kiểm tra phân giải tên miền được ko
	{
		cout << "DNS khong the phan giai duoc ten mien: " << HostName << endl;
		/*Xóa headerRequest, bodyRequest*/
		if (headerRequest != NULL)
		{
			free(headerRequest);
		}
		if (bodyRequest != NULL)
		{
			free(bodyRequest);
		}
		closesocket(Browser);
		ExitThread(0);
	}

	//Lấy URL từ headerRequest
	URL = GetStr(headerRequest, (char*)"GET http://", ' ');
	int lenURL = strlen(URL);

	/*Kiểm tra If-modifier-Since nếu có*/
	bool checkModified = FindSubStr(headerRequest, (char*)"If-Modified-Since");

	/*Kiểm tra URL này đã được caching chưa*/
	checkURL = CheckURLinList(URL, lenURL);

	if (checkURL == false || (checkURL == true && checkModified == true))		//Chưa được caching
	{
		if (checkURL == false)
		{
			Delete_Modified(headerRequest);
			szHeaderRequest = strlen(headerRequest);
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
			/*Xóa headerRequest, bodyRequest*/
			if (headerRequest != NULL)
			{
				free(headerRequest);
			}
			if (bodyRequest != NULL)
			{
				free(bodyRequest);
			}

			closesocket(Server);	//Đóng socket của Server
			closesocket(Browser);	//Đóng sockett của Browser
			ExitThread(0);
		}
		else
		{
			send(Server, headerRequest, szHeaderRequest, 0);	//Gửi header Request lên Web Server
			send(Server, bodyRequest, szBodyRequest, 0);	//Gửi body Request lên Web Server

			/*Nhận header từ Server*/
			char *headerResponse = NULL;
			char *bodyResponse = NULL;
			int sizeResponse = 0;

			//Nhận Header Respone từ Web Server 
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

			send(Browser, headerResponse, sizeResponse, 0);

			///////////////////////////////////////////////////////////////////////////////////
			/*Lấy Cache đưa lên Browser*/
			bool NotFound = FindSubStr(headerResponse, (char*)"304 Not Modified");

			if (NotFound == true)
			{
				char* URL_fileName = CreateFileName(URL, lenURL);		//Tạo tên cho tập tin dựa vào URL
				FILE *CachingFile;
				CachingFile = fopen(URL_fileName, "rb");
				int i = 0;
				int szStr;
				char* Str;

				if (!CachingFile) cout << "Khong the mo tap tin de lay du lieu caching" << endl;
				else
				{
					cout << "Da mo tap tin de lay du lieu caching" << endl;

					while (1)
					{
						i = fread(&szStr, sizeof(int), 1, CachingFile);
						if (i == 0) break;
						Str = (char*)malloc(szStr);
						fread(Str, szStr, 1, CachingFile);
						send(Browser, Str, szStr, 0);
						free(Str);
						Str = NULL;
					}
					fclose(CachingFile);
				}


				if (headerRequest != NULL)
				{
					free(headerRequest);
				}
				if (bodyRequest != NULL)
				{
					free(bodyRequest);
				}
				if (URL != NULL)
				{
					free(URL);
				}
				if (HostName != NULL)
				{
					free(HostName);
				}
				closesocket(Browser);
				ExitThread(0);
			}
			//////////////////////////////////////////////////////////////////////////////////

			//Kiểm tra gói tin nếu là 200 OK thì lưu vào WebCache

			char* StatusLine = GetStr(headerResponse, (char*)"HTTP/1.", '\r');
			bool Check200OK = FindSubStr(StatusLine, (char*)"200 OK");
			char* URL_fileName = URL_fileName = CreateFileName(URL, lenURL);		//Tạo tên cho tập tin dựa vào URL

			////////////////////////////////////////////////////////////
			if (checkURL == true)
			{
				FILE *CachingFile;
				CachingFile = fopen(URL_fileName, "wb");
			}
			////////////////////////////////////////////////////////////

			if (Check200OK == true)
			{
				WriteCachingFile(URL_fileName, headerResponse, sizeResponse);			//Mở file để caching headerResponse

				fstream URLList;			//Mở file URLList.txt để cập nhật URL đã được caching
				URLList.open("WebCache\\\\URLList.txt", ios::app);
				if (URLList.fail())
					cout << "Khong the mo tap tin URLList.txt" << endl;
				URLList << URL << endl;
				URLList.close();
			}

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
					HTMLResponse[iResult] = '\0';	//Ngắt chuỗi nếu RESPONSE nhận < 512 BYTES hoặc cuối cùng để in chuỗi
					cout << HTMLResponse;
					send(Browser, HTMLResponse, iResult, 0);

					if (Check200OK == true && checkURL == false)
					{
						WriteCachingFile(URL_fileName, HTMLResponse, iResult);			//Mở file để caching headerResponse
					}
					break;
				}

				HTMLResponse[iResult] = '\0';	//Ngắt chuỗi nếu RESPONSE nhận < 512 BYTES hoặc cuối cùng để in chuỗi
				cout << HTMLResponse;
				send(Browser, HTMLResponse, iResult, 0);
				if (Check200OK == true)
				{
					WriteCachingFile(URL_fileName, HTMLResponse, iResult);			//Mở file để caching headerResponse
				}
				memset(HTMLResponse, 0, sizeof(HTMLResponse));	//xóa nội dung của HTMLResponse trước khi nhận			
			} while (iResult > 0);

			if (headerResponse != NULL)
			{
				free(headerResponse);
			}
			if (bodyResponse != NULL)
			{
				free(bodyResponse);
			}
			if (URL_fileName != NULL)
			{
				free(URL_fileName);
			}
		}
		closesocket(Server);
	}
	else if (checkModified == false && checkURL == true)
	{
		char* URL_fileName = CreateFileName(URL, lenURL);		//Tạo tên cho tập tin dựa vào URL
		FILE *CachingFile;
		CachingFile = fopen(URL_fileName, "rb");
		int i = 0;
		int szStr;
		char* Str;

		if (!CachingFile) cout << "Khong the mo tap tin de lay du lieu caching" << endl;
		else
		{
			cout << "Da mo tap tin de lay du lieu caching" << endl;

			while (1)
			{
				i = fread(&szStr, sizeof(int), 1, CachingFile);
				if (i == 0) break;
				Str = (char*)malloc(szStr);
				fread(Str, szStr, 1, CachingFile);
				send(Browser, Str, szStr, 0);
				free(Str);
				Str = NULL;
			}
			fclose(CachingFile);
		}
	}

	if (headerRequest != NULL)
	{
		free(headerRequest);
	}
	if (bodyRequest != NULL)
	{
		free(bodyRequest);
	}
	if (URL != NULL)
	{
		free(URL);
	}
	if (HostName != NULL)
	{
		free(HostName);
	}
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
		cout << "Version not supported" << endl;
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
			cout << "Loi listen cua ProxyServer\n" << endl;
		}

		closesocket(ProxyServer);
	}


	WSACleanup();
	system("pause");
}