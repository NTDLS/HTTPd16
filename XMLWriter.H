///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2010, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _NSWFL_XMLWriter_H_
#define _NSWFL_XMLWriter_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Memory.H"
#include "String.H"
#include "StringBuilder.H"
#include "XMLReader.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class XMLWriter {

public:
	void *pUserData; //Use it for whatever you want!

	typedef struct _tag_XMLWriter_State {
		StringBuilder *pBuilder;
		char sName[255];
	} XMLWriterState, *LPXMLWriterState;

	XMLWriter(const char *sChildName);
	XMLWriter(void);
	~XMLWriter(void);

	bool Initialize(void);
	bool Initialize(const char *sChildName);
	bool ChangeName(const char *sChildName);
	int GetName(char *sOutName, int iMaxOut);
	void Destroy(void);

	StringBuilder *GetBuilder(void);

	XMLReader *ToReader(XMLReader *lpXMLReader);

	bool Add(const char *sAttributeName, const char *sValue);
	bool Add(const char *sAttributeName, const char *sValue, const int iValueSz);
	bool Add(const char *sAttributeName, const unsigned int iValue);
	bool Add(const char *sAttributeName, const DWORD dwValue);
	bool Add(const char *sAttributeName, const signed int iValue);
	bool Add(const char *sAttributeName, const double dValue);
	bool Add(const char *sAttributeName, const double dValue, const int iDecimalPlaces);
	bool AddBool(const char *sAttributeName, const bool bValue);

	bool Add(const char *sAttributeName, XMLWriter *XML);
	bool Add(const char *sAttributeName, XMLReader *XML);

	bool Add(XMLReader *XML);
	bool Add(XMLWriter *XML);

	int Length(void);

	int Text(char *sOutXML, int iMaxOutSz);
	void Text(StringBuilder *textOut);
	char *Text(void);
	char *Text(int *iOutSz);
	void FreePointer(void *pPointer);


	bool ToFile(const char *sFileName);

protected:
	XMLWriterState State;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

