///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2010, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////                
#ifndef _NSWFL_XMLReader_CPP_
#define _NSWFL_XMLReader_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <STDLib.H>
#include <STDIO.H>

#include <String.H>
#include "String.H"
#include "Memory.H"
#include "XMLReader.H"
#include "XMLWriter.H"
#include "Conversion.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReader::~XMLReader(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ScanPosition(void)
{
	return this->State.iScanPosition;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ScanPosition(int iPosition)
{
	int iOldValue = this->State.iScanPosition;

	this->State.iScanPosition = iPosition;

	return iOldValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ProgressiveScan(bool bEnable)
{
	bool bOldValue = this->State.bProgressiveScan;
	this->State.bProgressiveScan = bEnable;

	this->State.iScanPosition = 0;

	return bOldValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::Text(char *sTarget, int iMaxSz)
{
	strcpy(sTarget, this->State.sOriginalXML);
	return this->State.iOriginalXMLSz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *XMLReader::Text(void)
{
	return this->State.sOriginalXML;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::Length(void)
{
	return this->State.iOriginalXMLSz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _XML_USE_COMPRESSION
XMLReader::XMLReader(const char *sXML, const int iXMLSz, bool bCompressed)
{
	memset(&this->State, 0, sizeof(this->State));

	if (bCompressed)
	{
		DecompressXML(sXML, iXMLSz, this);
	}
	else this->Initialize(sXML, iXMLSz);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReader::XMLReader(XMLReader *pReader)
{
	Initialize(pReader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::Initialize(XMLReader *pReader)
{
	return this->Initialize(pReader->Text(), pReader->Length());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReader::XMLReader(const char *sXML, const int iXMLSz)
{
	memset(&this->State, 0, sizeof(this->State));

	this->Initialize(sXML, iXMLSz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReader::XMLReader(const char *sXML)
{
	memset(&this->State, 0, sizeof(this->State));

	this->Initialize(sXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReader::XMLReader(void)
{
	memset(&this->State, 0, sizeof(this->State));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReader *XMLReader::SetPointer(const char *sXML, const int iXMLSz)
{
	memset(&this->State, 0, sizeof(this->State));

	this->State.sOriginalXML = (char *)sXML;
	this->State.iOriginalXMLSz = iXMLSz;

	this->ProgressiveScan(false);

	return this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReader *XMLReader::SetPointer(const char *sXML)
{
	return this->SetPointer(sXML, (int)strlen(sXML));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::Initialize(const char *sXML)
{
	return this->Initialize(sXML, (int)strlen(sXML));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	//Read XML from a file.

	1) DO NOT Initialize before this, that would cause a memory leak.
	2) Always Destroy() if this function returns TRUE.
*/
bool XMLReader::FromFile(const char *sFileName)
{
	FILE *hFile = fopen(sFileName, "rb");
	if (hFile == NULL)
	{
		return false; //File not found or access denied.
	}

    fseek(hFile, 0, SEEK_END);
    long lFileSz = ftell(hFile);
    fseek(hFile, 0, SEEK_SET);
	
	if (lFileSz == 0)
	{
		//The file is empty.
		fclose(hFile);
		return false;
	}
	else if (lFileSz < 0)
	{
		//File access error.
		fclose(hFile);
		return false;
	}

	char *sBuf = (char *)calloc(lFileSz + 1, sizeof(char));
	if (sBuf == NULL)
	{
		//Couldnt allocate RAM. Are we out?
		fclose(hFile);
		return false;
	}

	long lBytesRead = fread(sBuf, sizeof(char), lFileSz, hFile);
	if (lBytesRead != lFileSz)
	{
		//File read error. What the...!?
		fclose(hFile);
		return false;
	}

	fclose(hFile);

	bool bResult = this->Initialize(sBuf, lBytesRead);

	free(sBuf);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void XMLReader::Destroy(void)
{
	this->ProgressiveScan(false);
	if (this->State.sOriginalXML)
	{
		free(this->State.sOriginalXML);
	}

	memset(&this->State, 0, sizeof(this->State));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	//Initialize the XML class

	1) Always Destroy() if this function returns TRUE.
*/
bool XMLReader::Initialize(const char *sXML, const int iXMLSz)
{
	memset(&this->State, 0, sizeof(this->State));

	if (iXMLSz <= 0)
	{
		//No XML to parse.
		return false;
	}

	this->State.iOriginalXMLSz = iXMLSz;

	this->State.sOriginalXML = (char *)calloc(this->State.iOriginalXMLSz + 1, sizeof(char));

	if (!this->State.sOriginalXML)
	{
		//Memory allocation failure.
		return false;
	}

	memcpy(this->State.sOriginalXML, sXML, this->State.iOriginalXMLSz);
	this->State.sOriginalXML[this->State.iOriginalXMLSz] = '\0';

	this->ProgressiveScan(false);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToDouble(const char *sTag, double *dOutValue)
{
	int iResult = 0;
	char sInteger[64];
	if ((iResult = ToString(sTag, sInteger, sizeof(sInteger))) >= 0)
	{
		*dOutValue = fFormattedValue(sInteger);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToBoolean(const char *sTag, bool *bOutValue)
{
	int iResult = 0;
	char sValue[32];
	if ((iResult = this->ToString(sTag, sValue, sizeof(sValue))) >= 0)
	{
		int iValue = 0;

		if (strcmpi(sValue, "yes") == 0
			|| strcmpi(sValue, "true") == 0
			|| strcmpi(sValue, "on") == 0)
		{
			iValue = 1;
		}
		else {
			iValue = iFormattedValue(sValue);
		}

		if (iValue)
		{
			*bOutValue = true;
		}
		else {
			*bOutValue = false;
		}

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToUInteger(const char *sTag, DWORD *uiOutValue)
{
	int iResult = 0;
	char sInteger[64];
	if ((iResult = ToString(sTag, sInteger, sizeof(sInteger))) >= 0)
	{
		*uiOutValue = uiFormattedValue(sInteger);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToUInteger(const char *sTag, unsigned int *uiOutValue)
{
	int iResult = 0;
	char sInteger[64];
	if ((iResult = ToString(sTag, sInteger, sizeof(sInteger))) >= 0)
	{
		*uiOutValue = uiFormattedValue(sInteger);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToInteger(const char *sTag, int *iOutValue)
{
	int iResult = 0;
	char sInteger[64];
	if ((iResult = ToString(sTag, sInteger, sizeof(sInteger))) >= 0)
	{
		*iOutValue = iFormattedValue(sInteger);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double XMLReader::ToDouble(const char *sTag, double dDefaultValue)
{
	double dValue = 0;
	if (ToDouble(sTag, &dValue))
	{
		return dValue;
	}
	else return dDefaultValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double XMLReader::ToDouble(const char *sTag)
{
	double dValue = 0;
	if (ToDouble(sTag, &dValue))
	{
		return dValue;
	}
	else return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToBoolean(const char *sTag, bool bDefaultValue)
{
	bool bValue = 0;
	if (this->ToBoolean(sTag, &bValue))
	{
		return bValue;
	}
	else return bDefaultValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToBoolean(const char *sTag)
{
	bool bValue = 0;
	if (ToBoolean(sTag, &bValue))
	{
		return bValue;
	}
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int XMLReader::ToUInteger(const char *sTag, unsigned int uiDefaultValue)
{
	unsigned int uiValue = 0;
	if (ToUInteger(sTag, &uiValue))
	{
		return uiValue;
	}
	else return uiDefaultValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int XMLReader::ToUInteger(const char *sTag)
{
	unsigned int uiValue = 0;
	if (ToUInteger(sTag, &uiValue))
	{
		return uiValue;
	}
	else return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ToInteger(const char *sTag, int iDefaultValue)
{
	int iValue = 0;
	if (ToInteger(sTag, &iValue))
	{
		return iValue;
	}
	else return iDefaultValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ToInteger(const char *sTag)
{
	int iValue = 0;
	if (ToInteger(sTag, &iValue))
	{
		return iValue;
	}
	else return 0; //ToInteger failed.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::Contains(const char *sTag)
{
	if (this->State.iOriginalXMLSz <= 0)
	{
		//No XML.
		return false;
	}

	char sTagIn[255];
	char sTagOut[255];

	sprintf(sTagIn, "<%s>", sTag);
	sprintf(sTagOut, "</%s>", sTag);

	int iTagIn = 0;
	int iTagOut = 0;

	if ((iTagIn = InStrI(sTagIn, this->State.sOriginalXML, this->State.iOriginalXMLSz, 0)) >= 0)
	{
		if ((iTagOut = InStrI(sTagOut, this->State.sOriginalXML, this->State.iOriginalXMLSz, 0)) >= 0)
		{
			return true;
		}
		else {
			//Closing tag not found.
			return false;
		}
	}
	else {
		//Tag not found.
		return false;
	}

	//Unknown error.
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::AttributeLength(const char *sTag)
{
	if (this->State.iOriginalXMLSz <= 0)
	{
		//No XML.
		return -1;
	}

	char sTagIn[255];
	char sTagOut[255];

	sprintf(sTagIn, "<%s>", sTag);
	sprintf(sTagOut, "</%s>", sTag);

	int iTagIn = 0;
	int iTagOut = 0;

	if ((iTagIn = InStrI(sTagIn, this->State.sOriginalXML, this->State.iOriginalXMLSz, 0)) >= 0)
	{
		if ((iTagOut = InStrI(sTagOut, this->State.sOriginalXML, this->State.iOriginalXMLSz, 0)) >= 0)
		{
			return (iTagOut - iTagIn) - (int)strlen(sTagIn);
		}
		else {
			//Closing tag not found.
			return -2;
		}
	}
	else {
		//Tag not found.
		return -3;
	}

	//Unknown error.
	return -4;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToReader(const char *sTag, const int iTag, XMLReader *lpXMLReader)
{
	char sFullTag[255];
	char sITag[64];
	if (itoa(iTag, sITag, 10) == 0)
	{
		strcpy(sFullTag, sTag);
		strcat(sFullTag, sITag);
		return this->ToReader(sFullTag, lpXMLReader);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToReader(const char *sTag, XMLReader *lpXMLReader)
{
	int iAlloc = this->State.iOriginalXMLSz + 1;
	int iLength = 0;

	char *sBuf = (char *)calloc(iAlloc, sizeof(char));

	if ((iLength = this->ToString(sTag, sBuf, iAlloc)) >= 0)
	{
		memset(lpXMLReader, 0, sizeof(XMLReader));
		lpXMLReader->SetPointer(sBuf, iLength);
		return true;
	}
	else {

		free(sBuf);

		lpXMLReader = NULL;

		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ToString(const char *sTag, char *sOutAttrib, int iMaxSz)
{
	return this->ToString(sTag, sOutAttrib, iMaxSz, "");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToString(const char *sTag, char *sOutAttrib, int iMaxSz, int *iReturnLength)
{
	int iLength = ToString(sTag, sOutAttrib, iMaxSz, "");
	if (iLength >= 0)
	{
		*iReturnLength = iLength;
		return true;
	}
	else {
		*iReturnLength = 0;
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool XMLReader::ToString(const char *sTag, char *sOutAttrib, int iMaxSz, const char *sDefaultValue, int *iReturnLength)
{
	int iLength = ToString(sTag, sOutAttrib, iMaxSz, sDefaultValue);
	if (iLength >= 0)
	{
		*iReturnLength = iLength;
		return true;
	}
	else {
		*iReturnLength = (int)strlen(sDefaultValue);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ToString(const char *sTag, StringBuilder *OutAttrib)
{
	return this->ToString(sTag, OutAttrib, "");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ToString(const char *sTag, StringBuilder *OutAttrib, const char *sDefaultValue)
{
	OutAttrib->Clear();

	if (this->State.iOriginalXMLSz <= 0)
	{
		OutAttrib->Append(sDefaultValue);
		return -1; //No XML.
	}

	char sTagIn[255];
	char sTagOut[255];

	sprintf(sTagIn, "<%s>", sTag);
	sprintf(sTagOut, "</%s>", sTag);

	int iTagInSz = (int)strlen(sTagIn);
	int iTagIn = 0;
	int iTagOut = 0;
	int iAttributeSz = 0;
	int iStartPosition = 0;

	if (this->State.bProgressiveScan)
	{
		iStartPosition = this->State.iScanPosition;
	}

	if ((iTagIn = InStrI(sTagIn, this->State.sOriginalXML, this->State.iOriginalXMLSz, iStartPosition)) >= 0)
	{
		if ((iTagOut = InStrI(sTagOut, this->State.sOriginalXML, this->State.iOriginalXMLSz, iTagIn + iTagInSz)) >= 0)
		{
			iAttributeSz = (iTagOut - iTagIn) - iTagInSz;

			OutAttrib->Append(this->State.sOriginalXML + iTagIn + iTagInSz, iAttributeSz);

			if (this->State.bProgressiveScan)
			{
				this->State.iScanPosition = iTagOut + iTagInSz + 1;
			}

			return iAttributeSz;
		}
		else {
			OutAttrib->Append(sDefaultValue);
			return -2; //Closing tag not found.
		}
	}
	else {
		OutAttrib->Append(sDefaultValue);
		return -3; //Tag not found.
	}

	OutAttrib->Append(sDefaultValue);
	return -4; //Unknown error.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int XMLReader::ToString(const char *sTag, char *sOutAttrib, int iMaxSz, const char *sDefaultValue)
{
	if (this->State.iOriginalXMLSz <= 0)
	{
		strcpy(sOutAttrib, sDefaultValue);
		return -1; //No XML.
	}

	char sTagIn[255];
	char sTagOut[255];

	sprintf(sTagIn, "<%s>", sTag);
	sprintf(sTagOut, "</%s>", sTag);

	int iTagInSz = (int)strlen(sTagIn);
	int iTagIn = 0;
	int iTagOut = 0;
	int iAttributeSz = 0;
	int iStartPosition = 0;

	if (this->State.bProgressiveScan)
	{
		iStartPosition = this->State.iScanPosition;
	}

	if ((iTagIn = InStrI(sTagIn, this->State.sOriginalXML, this->State.iOriginalXMLSz, iStartPosition)) >= 0)
	{
		if ((iTagOut = InStrI(sTagOut, this->State.sOriginalXML, this->State.iOriginalXMLSz, iTagIn + iTagInSz)) >= 0)
		{
			iAttributeSz = (iTagOut - iTagIn) - iTagInSz;
			strncpy(sOutAttrib, this->State.sOriginalXML + iTagIn + iTagInSz, iAttributeSz);

            sOutAttrib[iAttributeSz] = '\0';

			if (this->State.bProgressiveScan)
			{
				this->State.iScanPosition = iTagOut + iTagInSz + 1;
			}

			return iAttributeSz;
		}
		else {
			strcpy(sOutAttrib, sDefaultValue);
			return -2; //Closing tag not found.
		}
	}
	else {
		strcpy(sOutAttrib, sDefaultValue);
		return -3; //Tag not found.
	}

	strcpy(sOutAttrib, sDefaultValue);
	return -4; //Unknown error.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	//Writes XML to a file.
*/
bool XMLReader::ToFile(const char *sFileName)
{
	FILE *hFile = fopen(sFileName, "wb");
	if (hFile == NULL)
	{
		return false; //File not found or access denied.
	}

	fwrite(this->State.sOriginalXML, sizeof(char), this->State.iOriginalXMLSz, hFile);

	fclose(hFile);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

