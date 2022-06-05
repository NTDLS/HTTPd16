////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <String.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Winsock.H>

#include "Resource.h"
#include "MainDialog.H"
#include "Routines.H"
#include "Win16.H"
#include "String.H"
#include "SockServ.H"
#include "CMemory.H"
#include "ServerWindowMap.H"
#include "HTTP.H"
#include "XMLReader.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define IDT_INTERRUPT_TIMER	WM_USER + 100
#define DEFAULT_BUFLEN		256

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ServerWindowMap ServerMappings;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SockServ::Init(HWND hOwner, HWND hLogText)
{
	this->hOwner = hOwner;
	this->hLogText = hLogText;

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1);
	return WSAStartup(wVersionRequested, &wsaData) == 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SockServ::Start()
{
    XMLReader config;
    config.FromFile(".\\Config.xml");

    char sBuffer[255];

    config.ToString("BindToIP", sBuffer, sizeof(sBuffer));
    this->sBindToIP = pMem->StrDup(sBuffer);

    config.ToString("RootPath", sBuffer, sizeof(sBuffer));
    Trim(sBuffer, '\\');
    this->sRootPath = pMem->StrDup(sBuffer);

    config.ToString("DefaultFile", sBuffer, sizeof(sBuffer));
    this->sDefaultFile = pMem->StrDup(sBuffer);

    this->bBindAllIPs = config.ToBoolean("BindAllIPs");
    this->iListenPort = config.ToInteger("Port");

    config.Destroy();

	// Create a SOCKET for listening for
	// incoming connection requests.
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
	    ListBox_Insert(hLogText, "socket() failed.");
        return false;
	}

	char sHostName[255];
	gethostname(sHostName, sizeof(sHostName));
	hostent *host_entry = gethostbyname(sHostName);
	char *sLocalIP = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in service;
    if(this->bBindAllIPs)
    {
		service.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
	    service.sin_addr.s_addr = inet_addr(this->sBindToIP);
    }
	service.sin_family = AF_INET;
	service.sin_port = htons(this->iListenPort);

	if (bind(ListenSocket, (SOCKADDR *) &service, sizeof(service)) == SOCKET_ERROR)
	{
	    ListBox_Insert(hLogText, "bind() failed.");
		closesocket(ListenSocket);
        return false;
	}

	//----------------------
	// Listen for incoming connection requests. on the created socket
	if (listen(ListenSocket, 1) == SOCKET_ERROR)
	{
	    ListBox_Insert(hLogText, "listen() failed.");
		closesocket(ListenSocket);
        return false;
	}

   //----------------------
	// Create a SOCKET for accepting incoming requests.

	u_long NonBlock = 1;
	if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		//Set_Text(hUsername, "Setting non blocking failed");
	    ListBox_Insert(hLogText, "ioctlsocket() failed.");
		return false;
	}

    ServerMappings.Upsert(hOwner, this);

    SetTimer(hOwner, IDT_INTERRUPT_TIMER, 10, (TIMERPROC)InterruptTimerProc);

    ListBox_Insert(hLogText, "Waiting for client to connect....");

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VOID CALLBACK InterruptTimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{
	ServerMappings.At(hwnd)->TCPPump();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SockServ::TCPPump(void)
{
    char sLogText[255];

	fd_set readSet;
	fd_set writeSet;
	fd_set exceptSet;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&exceptSet);

	FD_SET(ListenSocket, &readSet);
	FD_SET(ListenSocket, &exceptSet);

	timeval timeout;
    memset(&timeout, 0, sizeof(timeval));
	timeout.tv_sec = 0;

	int selectResult = 0;

    selectResult = select(ListenSocket, &readSet, NULL, &exceptSet, &timeout);
	if(selectResult == SOCKET_ERROR)
    {
    	sprintf(sLogText, "select() failed %ld", WSAGetLastError());
	    ListBox_Insert(hLogText, sLogText);

        shutdown(ListenSocket, 2); //SD_BOTH
	    closesocket(ListenSocket);
    }
	else if(selectResult > 0)
	{
	    if(FD_ISSET(ListenSocket, &exceptSet))
        {
	    	sprintf(sLogText, "socket exception %ld", WSAGetLastError());
		    ListBox_Insert(hLogText, sLogText);
			shutdown(ListenSocket, 2); //SD_BOTH*
		    closesocket(ListenSocket);
        }
	    //if(FD_ISSET(ListenSocket, &readSet))
        else
        {
		    SOCKET socket = accept(ListenSocket, NULL, NULL);
		    if (socket == INVALID_SOCKET)
		    {
		    	sprintf(sLogText, "accept() failed %ld", WSAGetLastError());
			    ListBox_Insert(hLogText, sLogText);
		        closesocket(socket);
		    }
            else
            {
			    ListBox_Insert(hLogText, "Connection accepted.");
            	Clients.Upsert(socket);
            }
        }
	}

    FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&exceptSet);

	for(int i = 0; i < Clients.Entities.Count; i++)
    {
		if(Clients.Entities.Collection[i].InUse)
		{
			FD_SET(Clients.Entities.Collection[i].Socket, &readSet);
			FD_SET(Clients.Entities.Collection[i].Socket, &writeSet);
			FD_SET(Clients.Entities.Collection[i].Socket, &exceptSet);
        }
    }

	if(select(-1, &readSet, &writeSet, &exceptSet, &timeout) != 0)
	{
		for(int iClient = 0; iClient < Clients.Entities.Count; iClient++)
		{
			if(Clients.Entities.Collection[iClient].InUse)
			{
	            SOCKETCLIENTENTITY *pClient = &Clients.Entities.Collection[iClient];

				if(FD_ISSET(pClient->Socket, &readSet))
				{
					sprintf(sLogText, "Free Pages: %lu", pMem->Pages.uCount - pMem->Pages.uUsed);
					ListBox_Insert(hLogText, sLogText);

			    	sprintf(sLogText, "Receiving %d", pClient->Index);
				    ListBox_Insert(hLogText, sLogText);

					unsigned long ulReceivedHeaderAlloc = 4096ul;
					char *sReceivedHeader = (char*) calloc(ulReceivedHeaderAlloc, 1);
					int iReceivedHeaderSz = recv(pClient->Socket, sReceivedHeader, ulReceivedHeaderAlloc, 0);

			    	sprintf(sLogText, "Client %d received %d bytes.", pClient->Index, iReceivedHeaderSz);
				    ListBox_Insert(hLogText, sLogText);

                    if(iReceivedHeaderSz == SOCKET_ERROR)
                    {
                    	free(sReceivedHeader);
				    	sprintf(sLogText, "recv() failed %ld", WSAGetLastError());
					    ListBox_Insert(hLogText, sLogText);
						Clients.Disconnect(pClient->Index);
                        continue;
                    }
                    else if(iReceivedHeaderSz <= 0)
                    {
                    	free(sReceivedHeader);
				    	sprintf(sLogText, "Client %d disconnected", pClient->Index);
					    ListBox_Insert(hLogText, sLogText);
						Clients.Disconnect(pClient->Index);
                        continue;
                    }
                    else if(iReceivedHeaderSz == ulReceivedHeaderAlloc)
                    {
                    	free(sReceivedHeader);
	                    //The buffer is not large enough and I don't care to make it large enough.
                        Clients.Disconnect(pClient->Index);
                        continue;
                    }


					int iFurthestExtent = 0;
                    int iBufferAlloc = 2048;

				    char *sBuffer = (char*) calloc(sizeof(char), iBufferAlloc);
					int iTokenSz = GetNextToken(sReceivedHeader, iReceivedHeaderSz, sBuffer, iBufferAlloc, &iFurthestExtent);
					pClient->Header.Method = pMem->StrDup(sBuffer);

					int iRequestLength = GetNextToken(sReceivedHeader, iReceivedHeaderSz, sBuffer, iBufferAlloc, &iFurthestExtent);
					pClient->Header.Request = (char *)calloc(sizeof(char), iTokenSz + 1);
        	        strcpy(pClient->Header.Request, sBuffer);

					iTokenSz = GetNextToken(sReceivedHeader, iReceivedHeaderSz, sBuffer, iBufferAlloc, &iFurthestExtent);
					pClient->Header.Version = (char *)calloc(sizeof(char), iTokenSz + 1);
        	        strcpy(pClient->Header.Version, sBuffer);

					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Accept:", pClient->Header.Accept, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Accept-Language:", pClient->Header.AcceptLanguage, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Accept-Encoding:", pClient->Header.AcceptEncoding, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "User-Agent:", pClient->Header.UserAgent, &iFurthestExtent);
				 	iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Cache-Control:", pClient->Header.CacheControl, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Cookie:", pClient->Header.Cookie, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Host:", pClient->Header.Host, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Connection:", pClient->Header.Connection, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Referer:", pClient->Header.Referer, &iFurthestExtent);
					iTokenSz = GetHttpHeaderTag(sReceivedHeader, iReceivedHeaderSz, "Content-Type:", pClient->Header.ContentType, &iFurthestExtent);

                    ReplaceCharacter(pClient->Header.Request, '/', '\\');
                    Trim(pClient->Header.Request, '\\');

                    sprintf(sBuffer, "%s\\%s", this->sRootPath, pClient->Header.Request);
                    pClient->Header.FullRequest = pMem->StrDup(sBuffer);

					//If the request is a directory, then append the default filename.
                    if(IsDirectory(pClient->Header.FullRequest))
                    {                                                                                                       
					    free(pClient->Header.FullRequest);
	                    sprintf(sBuffer, "%s\\%s", this->sRootPath, pClient->Header.Request);
	                    Trim(sBuffer, '\\');
                        strcat(sBuffer, "\\");
                        strcat(sBuffer, this->sDefaultFile);
	                    pClient->Header.FullRequest = pMem->StrDup(sBuffer);
                    }

				    char *sDebug = (char*)calloc(1024, 1);
					sprintf(sDebug, "FullRequest:\"%s\",Method:\"%s\",Request:\"%s\",Version:\"%s\",Accept:\"%s\",AcceptLanguage:\"%s\",AcceptEncoding:\"%s\",UserAgent:\"%s\",CacheControl:\"%s\",Cookie:\"%s\",Host:\"%s\",Connection:\"%s\",Referer:\"%s\",ContentType:\"%s\"",
	                    pClient->Header.FullRequest,
						pClient->Header.Method,
						pClient->Header.Request,
						pClient->Header.Version,
						pClient->Header.Accept,
						pClient->Header.AcceptLanguage,
						pClient->Header.AcceptEncoding,
						pClient->Header.UserAgent,
						pClient->Header.CacheControl,
						pClient->Header.Cookie,
						pClient->Header.Host,
						pClient->Header.Connection,
						pClient->Header.Referer,
						pClient->Header.ContentType);
                	ListBox_Insert(hLogText, sDebug);
                    free(sDebug);

                    free(sBuffer);
                   	free(sReceivedHeader);
				}
				if(FD_ISSET(pClient->Socket, &writeSet))
				{
			    	sprintf(sLogText, "Sending %d", pClient->Index);
				    ListBox_Insert(hLogText, sLogText);

                    SendFile(pClient->Header.FullRequest, pClient);

			    	sprintf(sLogText, "Disconnecting %d", pClient->Index);
				    ListBox_Insert(hLogText, sLogText);

					Clients.Disconnect(pClient->Index);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SockServ::SendFile(char *sFileName, SOCKETCLIENTENTITY *pClient)
{
    char sHeader[255];

	FILE *hFile = fopen(sFileName, "rb");
    if(hFile == NULL)
    {
		sprintf(sHeader, "HTTP/1.0 404 Not found\n\n");
		Clients.Send(pClient->Index, sHeader, strlen(sHeader));
	    return false;
    }

    fseek(hFile, 0, SEEK_END);
    long lFileSz = ftell(hFile);
    fseek(hFile, 0, SEEK_SET);

	sprintf(sHeader, "HTTP/1.0 200 OK\ncontent-length:%ld\n\n", lFileSz);
	Clients.Send(pClient->Index, sHeader, strlen(sHeader));

    char *sBuf = (char*)calloc(DEFAULT_BUFLEN + 1, 1);

    long lBytesRemain = lFileSz;
    while(lBytesRemain > 0)
    {
		long lBytesToRead = DEFAULT_BUFLEN;
        if(lBytesRemain < lBytesToRead)
        {
			lBytesToRead = lBytesRemain;
        }

	    long lBytesRead = fread(sBuf, 1, lBytesToRead, hFile);
		Clients.Send(pClient->Index, sBuf, lBytesRead);

        lBytesRemain -= lBytesRead;
    }

    free(sBuf);
    fclose(hFile);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SockServ::Stop()
{
	KillTimer(hOwner, IDT_INTERRUPT_TIMER);

	if(ListenSocket != NULL)
    {
		shutdown(ListenSocket, 2 /*SD_BOTH*/);
		closesocket(ListenSocket);
    }

	for(int i = 0; i < Clients.Entities.Count; i++)
	{
		if(Clients.Entities.Collection[i].InUse)
		{
            Clients.Disconnect(i);
        }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SockServ::Cleanup()
{
	WSACleanup();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SockServ::TestSendHTTP(char *returnString)
{
	struct sockaddr_in serveraddr;

	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("10.20.1.138");
	serveraddr.sin_port = htons((unsigned short) 80);
	int error = connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    char httpRequestBuf[DEFAULT_BUFLEN];
    strcpy(httpRequestBuf, "GET / HTTP/1.1\r\n");
    strcat(httpRequestBuf, "Host: 10.20.1.138\r\n");
    strcat(httpRequestBuf, "Accept: * / *\r\n");
    strcat(httpRequestBuf, "\r\n");
	int bytesSent = send(sock, httpRequestBuf, strlen(httpRequestBuf), 0);

    int httpResponseBufSz = 4096;
    char *httpResponseBuf = (char*)calloc(httpResponseBufSz, 1);
    memset(httpResponseBuf, 0, httpResponseBufSz);
	int bytesRecv = recv(sock, httpResponseBuf, httpResponseBufSz, 0);

    sprintf(returnString, "%d", bytesRecv);

    if(bytesRecv > 0)
    {
		strcpy(returnString, httpResponseBuf);
    }

	closesocket(sock);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

