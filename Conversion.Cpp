///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2010, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _NSWFL_CONVERSION_CPP_
#define _NSWFL_CONVERSION_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <STDLib.H>
#include <STDIO.H>

#include <String.H>
#include "String.H"
#include "Memory.H"
#include "Conversion.H"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FileSizeFriendly(unsigned long ulFileSize, int iDecimalPlaces, char *sOut, int iMaxOutSz)
{
	bool bAllowChange = false;

	if (iDecimalPlaces < 0)
	{
		bAllowChange = true;
		iDecimalPlaces = (iDecimalPlaces * -1);
	}

	long i64DivideBy = 1;

	char *sSzStr = "";
	char sFormatted[128];
	char sFormat[10];


	if (ulFileSize >= GigaByte) {
		i64DivideBy = GigaByte;
		sSzStr = "GB";
	}
	else if (ulFileSize >= MegaByte) {
		i64DivideBy = MegaByte;
		sSzStr = "MB";
	}
	else if (ulFileSize >= KiloByte) {
		i64DivideBy = KiloByte;
		sSzStr = "KB";
	}
	else {
		i64DivideBy = 1;
		if (bAllowChange)
		{
			iDecimalPlaces = 0;
		}
		sSzStr = "B";
	}

	sprintf(sFormat, "%%.%df", iDecimalPlaces);
	sprintf(sOut, sFormat, (double)ulFileSize / i64DivideBy);
	FormatFloatString(sFormatted, sizeof(sFormatted), sOut);

	sprintf(sOut, "%s %s", sFormatted, sSzStr);

	return sOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Bin2HexFriendly(void *lpTarget, int iMaxSz, const void *lpSrc, int iSz, int iBytes, int iWidth)
{
	int iWPos = 0;
	int iRPos = 0;
	int iByte = 0;
	int iCurrentWidth = 0;

	if (iBytes < 1)
	{
		return 0;
	}

	//Assert(((iSz * iBytes) + 1) > iMaxSz, "Buffer too small.");

	while (iRPos < iSz)
	{
		iByte = (iBytes - 1);

		while (iByte >= 0)
		{
			((char *)lpTarget)[iWPos++] = "0123456789ABCDEF"[((((char *)lpSrc)[iRPos] >> (iByte--) * 4) & 0xF)];
			iCurrentWidth++;
		}

		if (iCurrentWidth >= iWidth)
		{
			((char *)lpTarget)[iWPos++] = '\r';
			((char *)lpTarget)[iWPos++] = '\n';
			iCurrentWidth = 0;
		}
		else {
			((char *)lpTarget)[iWPos++] = ' ';
		}

		iRPos++;
	}

	((char *)lpTarget)[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Bin2Hex(void *lpTarget, int iMaxSz, const void *lpSrc, int iSz, int iBytes, bool bPrefix)
{
	int iWPos = 0;
	int iRPos = 0;
	int iByte = 0;

	if (iBytes < 1)
	{
		return 0;
	}

	//Assert(((iSz * iBytes) + 1) > iMaxSz, "Buffer too small.");

	if (bPrefix)
	{
		((char *)lpTarget)[iWPos++] = '0';
		((char *)lpTarget)[iWPos++] = 'x';
	}

	while (iRPos < iSz)
	{
		iByte = (iBytes - 1);

		while (iByte >= 0)
		{
			((char *)lpTarget)[iWPos++] = "0123456789ABCDEF"[((((char *)lpSrc)[iRPos] >> (iByte--) * 4) & 0xF)];
		}
		iRPos++;
	}

	((char *)lpTarget)[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Bin2Hex(void *lpTarget, int iMaxSz, const void *lpSrc, int iSz, int iBytes)
{
	return Bin2Hex(lpTarget, iMaxSz, lpSrc, iSz, iBytes, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int HexToDecI(const char *sHexString)
{
	struct CHexMap
	{
		char cChar;
		int iValue;
	};

	const int iHexMapSz = 16;

	CHexMap HexMap[iHexMapSz] =
	{
		{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3},
		{'4', 4}, {'5', 5}, {'6', 6}, {'7', 7},
		{'8', 8}, {'9', 9}, {'A',10}, {'B',11},
		{'C',12}, {'D',13}, {'E',14}, {'F',15}
	};

	char sAlloc[64];
	strcpy(sAlloc, UCase((char *)sHexString));

	char *sHex = sAlloc;
	int iResult = 0;
	bool bFirstTime = true;

	if (*sHex == '0' && *(sHex + 1) == 'X')
	{
		sHex += 2;
	}

	while (*sHex != '\0')
	{
		bool bFound = false;
		for (int i = 0; i < iHexMapSz; i++)
		{
			if (*sHex == HexMap[i].cChar)
			{
				if (!bFirstTime)
				{
					iResult <<= 4;
				}
				iResult |= HexMap[i].iValue;
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			break;
		}

		sHex++;
		bFirstTime = false;
	}

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int HexToDecU(const char *sHexString)
{
	struct CHexMap
	{
		char cChar;
		int iValue;
	};

	const int iHexMapSz = 16;

	CHexMap HexMap[iHexMapSz] =
	{
		{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3},
		{'4', 4}, {'5', 5}, {'6', 6}, {'7', 7},
		{'8', 8}, {'9', 9}, {'A',10}, {'B',11},
		{'C',12}, {'D',13}, {'E',14}, {'F',15}
	};

	char sAlloc[64];
	strcpy(sAlloc, UCase((char *)sHexString));

	char *sHex = sAlloc;
	unsigned int iResult = 0;
	bool bFirstTime = true;

	if (*sHex == '0' && *(sHex + 1) == 'X')
	{
		sHex += 2;
	}

	while (*sHex != '\0')
	{
		bool bFound = false;
		for (int i = 0; i < iHexMapSz; i++)
		{
			if (*sHex == HexMap[i].cChar)
			{
				if (!bFirstTime)
				{
					iResult <<= 4;
				}
				iResult |= HexMap[i].iValue;
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			break;
		}

		sHex++;
		bFirstTime = false;
	}

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *LongChar(char *sOutVal, int iMaxSz, DWORD dwInput)
{
	ultoa(dwInput, sOutVal, 10);
	return sOutVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FloatChar(char *sOutVal, int iMaxSz, float fInput, int iDecPlaces)
{
	gcvt(fInput, iDecPlaces, sOutVal);
	return sOutVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FloatChar(char *sOutVal, int iMaxSz, float fInput)
{
	FloatChar(sOutVal, iMaxSz, fInput, 12);
	return sOutVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *DoubleChar(char *sOutVal, int iMaxSz, double dInput)
{
	sprintf(sOutVal, "%.1f", dInput);
	return sOutVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *IntChar(char *sOutVal, int iMaxSz, int iInput)
{
	itoa(iInput, sOutVal, 10);
	return sOutVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CharLong(const char *sInput)
{
	return (long)atol(sInput);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CharFloat(const char *sInput)
{
	return (float)atof(sInput);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double CharDouble(const char *sInput)
{
	return (double)atof(sInput);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CharInt(const char *sInput)
{
	return (int)atoi(sInput);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double Round(double dValue, int iDecimalPlaces)
{
	char sFormat[64];
	char sTemp[64];
	sprintf(sFormat, "%%.%df", iDecimalPlaces);
	sprintf(sTemp, sFormat, dValue);
	return atof(sTemp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Round(double dValue, int iDecimalPlaces, char *sOutput, int iSizeOfOutput)
{
	char sFormat[64];
	sprintf(sFormat, "%%.%df", iDecimalPlaces);
	return sprintf(sOutput, sFormat, dValue);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatDouble(char *sOut, int iMaxSz, double dValue, int iDecimalPlaces)
{
	char sFormat[64];
	char sTemp[64];
	sprintf(sFormat, "%%.%df", iDecimalPlaces);
	sprintf(sTemp, sFormat, dValue);
	if (iDecimalPlaces > 0)
	{
		return FormatFloatString(sOut, iMaxSz, sTemp);
	}
	else {
		return FormatiString(sOut, iMaxSz, sTemp);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatFloat(char *sOut, int iMaxSz, float fValue, int iDecimalPlaces)
{
	return FormatDouble(sOut, iMaxSz, fValue, iDecimalPlaces);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatFloatString(char *sOut, int iMaxSz, const char *sIn)
{
	return FormatDoubleString(sOut, iMaxSz, sIn);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatDoubleString(char *sOut, int iMaxSz, const char *sIn)
{
	int iRPos = (int)strlen(sIn) - 1;
	int iWPos = 0;
	int iDPos = 0;

	bool bFoundDecimal = false;

	if (iRPos < 3)
	{
		strcpy(sOut, sIn);
		return sOut;
	}

	while (iRPos != -1)
	{
		if (sIn[iRPos] == '.')
		{
			bFoundDecimal = true;
			iDPos = -1;
		}

		if (iDPos++ == 3 && bFoundDecimal && sIn[iRPos] != '-')
		{
			//Assert(iWPos >= iMaxSz, "Buffer too small.");
			sOut[iWPos++] = ',';
			iDPos = 1;
		}

		if (iWPos >= iMaxSz)
		{
			//Assert(iWPos >= iMaxSz, "Buffer too small.");
		}
		sOut[iWPos++] = sIn[iRPos--];
	}

	ReverseString(sOut, iWPos);

	sOut[iWPos] = '\0';

	return sOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatiString(char *sOut, int iMaxSz, const char *sIn)
{
	int iRPos = (int)strlen(sIn) - 1;
	int iWPos = 0;
	int iDPos = 0;

	if (iRPos < 3)
	{
		strcpy(sOut, sIn);
		return sOut;
	}

	while (iRPos != -1)
	{
		if (iDPos++ == 3 && sIn[iRPos] != '-')
		{
			//Assert(iWPos >= iMaxSz, "Buffer too small.");
			sOut[iWPos++] = ',';
			iDPos = 1;
		}

		//Assert(iWPos >= iMaxSz, "Buffer too small.");
		sOut[iWPos++] = sIn[iRPos--];
	}

	ReverseString(sOut, iWPos);

	sOut[iWPos] = '\0';

	return sOut;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *FormatInteger(char *sOut, int iMaxSz, const long lIn)
{
	char sLong[64];
	ultoa(lIn, sLong, 10);
	return FormatiString(sOut, iMaxSz, sLong);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


float fFormattedValue(const char *sFormatted)
{
	return (float)dFormattedValue(sFormatted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double dFormattedValue(const char *sFormatted)
{
	int iLength = (int)strlen(sFormatted);
	int iRPos = 0;
	int iWPos = 0;

	char sVal[32];

	while (iRPos < iLength)
	{
		if (sFormatted[iRPos] == ',')
			iRPos++;

		if ((sFormatted[iRPos] < 48 || sFormatted[iRPos] > 57) && sFormatted[iRPos] != '.')
			break;

		sVal[iWPos] = sFormatted[iRPos];
		iRPos++;
		iWPos++;
	}
	sVal[iWPos] = '\0';

	return atof(sVal);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int iFormattedValue(const char *sFormatted)
{
	int iLength = (int)strlen(sFormatted);
	int iRPos = 0;
	int iWPos = 0;
	bool bIsNegative = false;

	char sVal[32];

	if (iLength <= 3)
	{
		return atol(sFormatted);
	}

	if (sFormatted[0] == '-')
	{
		bIsNegative = true;
		iRPos++;
	}

	while (iRPos < iLength)
	{
		if (sFormatted[iRPos] == ',')
			iRPos++;

		if (sFormatted[iRPos] < 48 || sFormatted[iRPos] > 57)
			break;

		sVal[iWPos] = sFormatted[iRPos];
		iRPos++;
		iWPos++;
	}
	sVal[iWPos] = '\0';

	if (bIsNegative)
	{
		return atol(sVal) * -1;
	}
	else {
		return atol(sVal);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int uiFormattedValue(const char *sFormatted)
{
	return (unsigned int)iFormattedValue(sFormatted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ParseBoolean(const int iValue)
{
	return(iValue != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ParseBoolean(const char *sText)
{
	if (StrEqlI(sText, "yes") || StrEqlI(sText, "true") || StrEqlI(sText, "on"))
	{
		return true;
	}

	return(ParseBoolean(atol(sText)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
