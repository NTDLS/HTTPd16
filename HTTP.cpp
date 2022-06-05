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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, SYSTEMTIME *pOutST, int *iFurthestExtent)
{
	char *sBuffer = NULL;
	int iTokenSz = this->GetHttpHeaderTag(sHeader, iHeaderLen, "If-Modified-Since:", sBuffer, iFurthestExtent);
	if(iTokenSz > 0)
	{
		if(BreakGMTTimeString(sBuffer, pOutST))
		{
			pMem->Free(sBuffer);
			return iTokenSz;
		}
	}

	memset(pOutST, 0, sizeof(SYSTEMTIME));

	pMem->Free(sBuffer);
	return iTokenSz;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, char *&sBuf, int *iFurthestExtent)
{
	int iTagLength = (int)strlen(sTag);
	int iRPos = 0;

	iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, 0);
	//The tag must either be on a line by its-self or the first line.
	while(iRPos > 0 && sHeader[iRPos - 1] != '\n')
	{
		//the tag may be further down the line...
		iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, iRPos + 1);
	}

	if(iRPos >= 0)
	{
		iRPos += iTagLength; //Skip the tag itsself;
		SkipWhiteSpaces(sHeader, iHeaderLen, &iRPos);

		int iEndPos = InStr("\n", 1, sHeader, iHeaderLen, iRPos);
		if(iEndPos > 0 && iEndPos > iRPos)
		{
			if(iEndPos > iRPos)
			{
				if(iFurthestExtent)
				{
					if(iEndPos > ((int)*iFurthestExtent))
					{
						*iFurthestExtent = iEndPos - 1;
					}                                                                                                                                                                     
				}

				int iLength = iEndPos - iRPos;
				sBuf = (char *)calloc(sizeof(char), iLength + 1);
                strncpy(sBuf, sHeader + iRPos, iLength);

				//pMem->CloneStringNSafe(sBuf, sHeader + iRPos, iLength);
				return Trim(sBuf, iLength);
			}
		}
	}

	sBuf = (char *)calloc(sizeof(char), 1);
    strcpy(sBuf, "");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen,
								const char *sTag, char *sBuf, int iMaxBuf, bool bPartialOk, int *iFurthestExtent)
{
	int iTagLength = (int)strlen(sTag);
	int iRPos = 0;

	if((iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, 0)) >= 0)
	{
		iRPos += iTagLength; //Skip the tag itsself;
		SkipWhiteSpaces(sHeader, iHeaderLen, &iRPos);

		int iEndPos = InStr("\n", 1, sHeader, iHeaderLen, iRPos);
		if(iEndPos > 0 && iEndPos > iRPos)
		{
			if(iEndPos > iRPos)
			{
				int iLength = iEndPos - iRPos;
				if(iLength >= iMaxBuf)
				{
					if(bPartialOk)
					{
						iLength = iMaxBuf - 1;
					}
					else {
						strcpy(sBuf, "");
						return 0;
					}
				}

				if(iFurthestExtent)
				{
					if(iEndPos > ((int)*iFurthestExtent))
					{
						*iFurthestExtent = iEndPos - 1;
					}
				}

				strncpy(sBuf, sHeader + iRPos, iLength);
				return Trim(sBuf, iLength);
			}
		}
	}

	strcpy(sBuf, "");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen,
								const char *sTag, char *sBuf, int iMaxBuf, int *iFurthestExtent)
{
	return GetHttpHeaderTag(sHeader, iHeaderLen, sTag, sBuf, iMaxBuf, false, iFurthestExtent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DoesHeaderContainTag(const char *sBuf, const int iBufLen, const char *sTag)
{
	return DoesHeaderContainTag(sBuf, iBufLen, sTag, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DoesHeaderContainTag(const char *sBuf, const int iBufLen,
								 const char *sTag, int *iHeaderEndPos)
{
	if(iHeaderEndPos)
	{
		*iHeaderEndPos = 0;
	}

	int iTagPos = InStrI(sTag, sBuf, iBufLen, 0);

	//The tag must either be on a line by its-self or the first line.
	while(iTagPos > 0 && sBuf[iTagPos - 1] != '\n')
	{
		//the tag may be further down the line...
		iTagPos = InStrI(sTag, sBuf, iBufLen, iTagPos + 1);
	}

	if(iTagPos >= 0)
	{
		int iLineFeedLen = 0;
		int iLineFeedPos = 0;
		int iSingleLineFeedPos = InStr("\n\n", 2, sBuf, iBufLen, 0);
		int iDoubleLineFeedPos = InStr("\r\n\r\n", 4, sBuf, iBufLen, 0);

		if(iDoubleLineFeedPos > 0 && iSingleLineFeedPos > 0)
		{
			if(iDoubleLineFeedPos < iSingleLineFeedPos) //We want to use the first one.
			{
				iLineFeedLen = 4;
				iLineFeedPos = iDoubleLineFeedPos;
			}
			else {
				iLineFeedLen = 2;
				iLineFeedPos = iSingleLineFeedPos;
			}
		}
		else if(iDoubleLineFeedPos > 0) {
			iLineFeedLen = 4;
			iLineFeedPos = iDoubleLineFeedPos;
		}
		else if(iSingleLineFeedPos > 0) {
			iLineFeedLen = 2;
			iLineFeedPos = iSingleLineFeedPos;
		}

		if(iLineFeedPos > 0 && iTagPos < iLineFeedPos)
		{
			if(iHeaderEndPos)
			{
				*iHeaderEndPos = (iLineFeedPos + iLineFeedLen);
			}
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsValueEqual(const char *sValue1, const char *sValue2)
{
	if(sValue1 && sValue2)
	{
		return(strcmpi(sValue1, sValue2) == 0);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DoesValueContain(const char *sIn, const char *sValue)
{
	if(sIn && sValue)
	{
		return(InStrI(sValue, sIn) >= 0);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


