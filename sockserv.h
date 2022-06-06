#include "SocketClients.H"

class SockServ {

public:
	bool Init(HWND hOwner, HWND hLogText);
	void Cleanup();
	bool Start();
	void Stop();

	void Log(SOCKETCLIENTENTITY *pClient, char *sText);
	void Log(char *sText);

	void TestSendHTTP(char *returnString);
    bool SendFile(char *sFileName, SOCKETCLIENTENTITY *pClient);

	SocketClients Clients;
	SOCKET ListenSocket;
    HWND hOwner;
	HWND hLogText;

    //---Config(Begin)--
    char *sRootPath;
    char *sDefaultFile;
    int iListenPort;
    bool bBindAllIPs;
    char *sBindToIP;
    //---Config(End)--

	void TCPPump(void);
};

VOID CALLBACK InterruptTimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);

