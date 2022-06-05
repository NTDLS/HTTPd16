///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | (((WORD)((BYTE)(b))) << 8)))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | (((DWORD)((WORD)(b))) << 16)))
#define LOWORD(l)           ((WORD)(l))
//#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
//#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsDirectory(char *sPath);
void Set_Text(HWND hWnd, const char *sInBuf);
int Get_TextLength(HWND hWnd);
int Get_Text(HWND hWnd, char *sOutBuf, int iMaxSize);
void ListBox_Insert(HWND hWnd, const char *sInBuf);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

