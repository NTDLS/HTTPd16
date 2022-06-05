extern HWND hDebugLog;

#include "CMemory.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_SocketClients_Header {
	char *Method;
	char *Request;
    char *FullRequest;
	char *Version;
	char *Accept;
	char *AcceptLanguage;
	char *AcceptEncoding;
	char *UserAgent;
	char *CacheControl;
	char *Cookie;
	char *Host;
	char *Connection;
	char *Referer;
	char *ContentType;
} SOCKETCLIENTHEADER, *LPSOCKETCLIENTHEAER;

typedef struct _tag_SocketClients_Entity {
	bool InUse;
    int Index;
	SOCKET Socket;
    SOCKETCLIENTHEADER Header;
	long BytesSent;
    long BytesRecv;
} SOCKETCLIENTENTITY, *LPSOCKETCLIENTENTITY;

typedef struct _tag_SocketClients_Entities {
	SOCKETCLIENTENTITY *Collection;
	int Count;
} SOCKETCLIENTENTITIES, *LPSOCKETCLIENTENTITIES;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SocketClients {
private:
public:
	SOCKETCLIENTENTITIES Entities;

	SocketClients()
    {
	    Entities.Count = 0;
        Entities.Collection = NULL;
    }

	~SocketClients()
    {
        if(Entities.Count > 0)
        {
		    pMem->Free(Entities.Collection);
        }
	    Entities.Count = 0;
    }

    void InitClient(SOCKETCLIENTENTITY *pClient, SOCKET socket, int index)
    {
        pClient->Socket = socket;
        pClient->InUse = true;
        pClient->Index = index;
        memset(&pClient->Header, 0, sizeof(SOCKETCLIENTHEADER));
    }

	void Upsert(SOCKET socket)
    {
    	for(int i = 0; i < Entities.Count; i++)
        {
        	if(Entities.Collection[i].InUse == false)
            {
	            InitClient(&Entities.Collection[i], socket, i);
                return;
            }
		}

	    Entities.Count++;
	    Entities.Collection = (SOCKETCLIENTENTITY*) realloc(Entities.Collection, sizeof(SOCKETCLIENTENTITY) * Entities.Count);
        InitClient(&Entities.Collection[Entities.Count - 1], socket, Entities.Count - 1);
    }

    void Send(int index, char *sBuf, int iSz)
    {
    	send(Entities.Collection[index].Socket, sBuf, iSz, 0);
        Entities.Collection[index].BytesSent += iSz;
    }

    void Disconnect(int index)
    {
    	SOCKETCLIENTENTITY *pClient = &Entities.Collection[index];

		shutdown(pClient->Socket, 2 /*SD_BOTH*/);
		closesocket(pClient->Socket);

		pClient->Socket = NULL;

	    //char sLogText[128];
		//sprintf(sLogText, "Freeing %d", index);
		//ListBox_Insert(hDebugLog, sLogText);

		if(pClient->Header.Method) pMem->Free(pClient->Header.Method);
		if(pClient->Header.Request) free(pClient->Header.Request);
		if(pClient->Header.Version) free(pClient->Header.Version);
		if(pClient->Header.Accept) free(pClient->Header.Accept);
		if(pClient->Header.AcceptLanguage) free(pClient->Header.AcceptLanguage);
		if(pClient->Header.AcceptEncoding) free(pClient->Header.AcceptEncoding);
		if(pClient->Header.UserAgent) free(pClient->Header.UserAgent);
		if(pClient->Header.CacheControl) free(pClient->Header.CacheControl);
		if(pClient->Header.Cookie) free(pClient->Header.Cookie);
		if(pClient->Header.Host) free(pClient->Header.Host);
		if(pClient->Header.Connection) free(pClient->Header.Connection);
		if(pClient->Header.Referer) free(pClient->Header.Referer);
		if(pClient->Header.ContentType) free(pClient->Header.ContentType);
        if(pClient->Header.FullRequest) free(pClient->Header.FullRequest);

        pClient->InUse = false;
    }

    void Disconnect(SOCKET socket)
    {
    	for(int i = 0; i < Entities.Count; i++)
        {
	        if(Entities.Collection[i].Socket == socket)
            {
            	Disconnect(i);
            }
        }
    }

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
