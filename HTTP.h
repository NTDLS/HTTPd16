//int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, SYSTEMTIME *pOutST, int *iFurthestExtent);
int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, char *&sBuf, int *iFurthestExtent);
int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen,
								const char *sTag, char *sBuf, int iMaxBuf, bool bPartialOk, int *iFurthestExtent);
int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen,
								const char *sTag, char *sBuf, int iMaxBuf, int *iFurthestExtent);
bool DoesHeaderContainTag(const char *sBuf, const int iBufLen, const char *sTag);
bool DoesHeaderContainTag(const char *sBuf, const int iBufLen,
								 const char *sTag, int *iHeaderEndPos);
bool IsValueEqual(const char *sValue1, const char *sValue2);
bool DoesValueContain(const char *sIn, const char *sValue);
