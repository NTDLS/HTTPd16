///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <String.H>
#include <Winsock.H>
#include <ToolHelp.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Resource.h"
#include "MainDialog.h"
#include "CMemory.H"
#include "XMLWriter.H"

#pragma comment(lib,"winsock.lib")

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR CmdLine, int CmdShow)
{
	/*
	XMLWriter xmlConfig("Configuration");
	xmlConfig.Add("Port", 80);
	xmlConfig.AddBool("BindAllIPs", true);
	xmlConfig.Add("BindToIP", "127.0.0.1");
	xmlConfig.Add("RootPath", "C:\\Source\\FHTTPd\\WWWRoot");
	xmlConfig.Add("DefaultFile", "Index.htm");

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("Type");
		Item.Add("Extension", this->Collection.Items[iItem].Extension);
		Item.Add("Mime", this->Collection.Items[iItem].Type);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.AddBool("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

    xmlConfig.ToFile("C:\\Config.xml");

	xmlConfig.Destroy();
    */

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)MainDialogProc);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


