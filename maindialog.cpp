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
#include "SockServ.H"
#include "CMemory.H"
#include "StringBuilder.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
HWND hMainDialog = NULL;
HWND hLogText = NULL;


HWND hDebugLog = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CALLBACK MainDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hTextStatic = NULL;
    static HMENU SystemMenu_hMenu = NULL;
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

		hLogText = GetDlgItem(hWnd, IDC_TEXTLOG);
        hDebugLog = hLogText;

		StringBuilder sText;

		sText.SetF("Physical Request: %lu", pMem->MPI.uPhysicalRequest);
		ListBox_Insert(hLogText, sText.Buffer);
		sText.SetF("Physical Grant: %lu", pMem->MPI.uPhysicalGrant);
		ListBox_Insert(hLogText, sText.Buffer);
		sText.SetF("Page Size: %lu", pMem->Pages.uSize);
		ListBox_Insert(hLogText, sText.Buffer);
		sText.SetF("Pages: %lu", pMem->Pages.uCount);
		ListBox_Insert(hLogText, sText.Buffer);
		sText.SetF("Manager Pages: %lu", pMem->Pages.uManagerPages);
		ListBox_Insert(hLogText, sText.Buffer);
		sText.SetF("Reserved Pages: %lu", pMem->Pages.uUsed);
		ListBox_Insert(hLogText, sText.Buffer);
		sText.SetF("Free Pages: %lu", pMem->Pages.uCount - pMem->Pages.uUsed);
		ListBox_Insert(hLogText, sText.Buffer);

	    sockSrv->Init(hMainDialog, hLogText);

		sockSrv->Start();

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }

    else if(uMsg == WM_TIMER)
    {
                    	    //Set_Text(hLogText, "ggg connected");
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

