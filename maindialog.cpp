///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
#include "Conversion.H"
#include "SockServ.H"
#include "CMemory.H"
#include "StringBuilder.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
HWND hMainDialog = NULL;
HWND hLogText = NULL;
HWND hDebugLog = NULL;
#define IDT_STATS_TIMER	WM_USER + 101

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CALLBACK MainDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hTextStatic = NULL;
    static HMENU SystemMenu_hMenu = NULL;
    static HWND hPagesFree = NULL;
    static HWND hPagesUsed = NULL;
    static HWND hManagerPages = NULL;
    static HWND hPageCount = NULL;
    static HWND hPageSize = NULL;
    static HWND hPhysicalGrant = NULL;
    static HWND hPhysicalRequest = NULL;
                                        
    static SockServ *sockSrv = new SockServ();

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        hMainDialog = hWnd;
        SystemMenu_hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(SystemMenu_hMenu, MF_SEPARATOR, 0, 0);
        //AppendMenu(SystemMenu_hMenu, MF_STRING, MAINDIALOG_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"HTTPd16");
        //SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hPagesFree = GetDlgItem(hWnd, IDC_STATICMEMPAGESFREE);
        hPagesUsed = GetDlgItem(hWnd, IDC_STATICMEMPAGESUSED);
        hManagerPages = GetDlgItem(hWnd, IDC_STATICMEMMANAGERPAGES);
        hPageCount = GetDlgItem(hWnd, IDC_STATICMEMPAGECOUNT);
        hPageSize = GetDlgItem(hWnd, IDC_STATICMEMPAGESIZE);
        hPhysicalGrant = GetDlgItem(hWnd, IDC_STATICMEMGRANT);
        hPhysicalRequest = GetDlgItem(hWnd, IDC_STATICMEMREQUEST);

		hLogText = GetDlgItem(hWnd, IDC_TEXTLOG);

        hDebugLog = hLogText;

	    sockSrv->Init(hMainDialog, hLogText);
		sockSrv->Start();

	    SetTimer(hWnd, IDT_STATS_TIMER, 500, NULL);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }

    else if(uMsg == WM_TIMER)
    {
    	if(wParam == IDT_STATS_TIMER)
        {
        	char sText[255];
        	char sNum[32];
        	char sNum2[32];

            FileSizeFriendly(pMem->MPI.uPhysicalRequest, 2, sNum, sizeof(sNum));
			Set_Text(hPhysicalRequest, sNum);

            FileSizeFriendly(pMem->MPI.uPhysicalGrant, 2, sNum, sizeof(sNum));
			Set_Text(hPhysicalGrant, sNum);

            FileSizeFriendly(pMem->Pages.uSize, 0, sNum, sizeof(sNum));
			Set_Text(hPageSize, sNum);

			FormatInteger(sNum, sizeof(sNum), pMem->Pages.uCount);
			Set_Text(hPageCount, sNum);

			FormatInteger(sNum, sizeof(sNum), pMem->Pages.uManagerPages);
			Set_Text(hManagerPages, sNum);

			FormatInteger(sNum, sizeof(sNum), pMem->Pages.uUsed);
            FileSizeFriendly(pMem->Pages.uUsed * pMem->Pages.uSize, 2, sNum2, sizeof(sNum2));
            sprintf(sText, "%s (%s)", sNum, sNum2);
			Set_Text(hPagesUsed, sText);

            FormatInteger(sNum, sizeof(sNum), pMem->Pages.uCount - pMem->Pages.uUsed);
            FileSizeFriendly((pMem->Pages.uCount - pMem->Pages.uUsed) * pMem->Pages.uSize, 2, sNum2, sizeof(sNum2));
            sprintf(sText, "%s (%s)", sNum, sNum2);
			Set_Text(hPagesFree, sText);
        }
	}

    //--------------------------------------------------------------------------

    else if(uMsg == WM_COMMAND)
    {
        if(wParam == IDOK)
        {
        	/*
		    int httpResponseBufSz = 4096;
		    char *httpResponseBuf = (char*)calloc(httpResponseBufSz, 1);
		    memset(httpResponseBuf, 0, httpResponseBufSz);

		    char *swapBuf = (char*)calloc(httpResponseBufSz, 1);
		    memset(swapBuf, 0, httpResponseBufSz);

        	sockSrv->Test(httpResponseBuf);

			ReplaceStrings(httpResponseBuf, "\n", "\r\n", swapBuf);

			Set_Text(hLogText, swapBuf);
			*/
            return TRUE;
        }
        else if(wParam == IDCANCEL)
        {
			sockSrv->Stop();
		    sockSrv->Cleanup();

            EndDialog(hWnd, 0);
            DestroyWindow(hWnd);
            return TRUE;
        }

        return FALSE;
    }

    //--------------------------------------------------------------------------

	else if(uMsg == WM_SYSCOMMAND) //- Received a system menu message.
    {
    /*
        if(LOWORD(wParam) == MAINDIALOG_MENU_ABOUT) //- About.
        {
            _AboutDialogInfo ADI;

            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);
            return TRUE;
        }
	*/
        return FALSE;
    }

    //--------------------------------------------------------------------------

    else if(uMsg == WM_CLOSE) //- Received close message.
    {
        EndDialog(hWnd,0);
        DestroyWindow(hWnd);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

