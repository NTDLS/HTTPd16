////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMEMORY_H_
#define _CMEMORY_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Memory mapping & handing routines.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define malloc(uBytes) pMem->Alloc(uBytes)
#define calloc(uElementCount, uElementSize) pMem->AllocZ(uElementCount, uElementSize)
#define realloc(pAddress, uTotalBytes) pMem->ReAlloc(pAddress, uTotalBytes)
#define free(pMemory) pMem->Free(pMemory)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ONEKILOBYTE                       1024ul
#define ONEMEGABYTE                    1048576ul
#define ONEGIGABYTE                 1073741824ul

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//BOKS Memory Manager Defines:
#define MEMORY_MANAGER_PAGE_SIZE		128ul    //Memory allocation granularity.

#define MEMORY_PAGE_ATTRIB_USED			0x0001ul //Pages that are allocated.
#define MEMORY_PAGE_ATTRIB_ENDOFCHAIN	0x0002ul //Pages that are the tail of a "chain" of pages.
#define MEMORY_PAGE_ATTRIB_MANAGER		0x0004ul //Pages that are used by the memory manager itsself.
#define MEMORY_PAGE_ATTRIB_RESERVED_0 	0x0010ul //Reserved for future use.
#define MEMORY_PAGE_ATTRIB_RESERVED_1 	0x0020ul //Reserved for future use.
#define MEMORY_PAGE_ATTRIB_RESERVED_2	0x0040ul //Reserved for future use.
#define MEMORY_PAGE_ATTRIB_RESERVED_3	0x0080ul //Reserved for future use.
#define MEMORY_PAGE_ATTRIB_RESERVED_4	0x0100ul //Reserved for future use.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Generic Memory Manager Defines:

#define MAX_MEMORY                  4294967296ul // Max memory supported by the OS.
#define BASE_MEMORY_ADDRESS            2097152ul // Base of the manager memory (2 megabyte barrier).

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Memory_Physical_Information{
    DWORD uBase;
	DWORD uPhysicalGrant;
    DWORD uPhysicalRequest;
    DWORD uUnreserved;
} MEMORYPHYSICALINFO, *LPMEMORYPHYSICALINFO;

typedef struct _tag_Memory_Manager_Info {
	char *uAttribs;				//Array of page attributes.
	DWORD uUsed;			//The number of pages that have been allocated (count of non-free pages).
	DWORD uCount;		//The total number of memory manager pages (([Total Mamory] - [Kernel Reserved]) / [Allocation Granularity]).
	DWORD uManagerPages;	//The number of pages reserved my the memory manager for its internal use.
	DWORD uSize;			//Number of bytes in each page.
	DWORD uBaseAddress;	//The base address for all allocations.
} MEMORYMANAGERPAGES, *LPMEMORYMANAGERPAGES;

typedef struct _tag_Memory_Status_Information {
	DWORD uBytesTotal;
	DWORD uBytesUsed;
	DWORD uBytesFree;
} MEMORYSTATUSINFO, *LPMEMORYSTATUSINFO;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMemory {
public:
	void *Alloc(DWORD uBytes);
	void *AllocZ(DWORD uElementCount, DWORD uElementSize);
	void *ReAlloc(void *pAddress, DWORD uTotalBytes);
	bool Free(void *pAddress);
	char *StrDup(const char *str);

	bool Initialize(void);
	void Destroy(void);
	bool TestPhysical(void);

	void GetStatusInfo(MEMORYSTATUSINFO *pMSI);
	void GetPhysicalInfo(MEMORYPHYSICALINFO *pMPI);
	void GetPageInfo(MEMORYMANAGERPAGES *pMMP);
	void *HardwareAlloc(DWORD uElementCount, DWORD uElementSize);

	MEMORYPHYSICALINFO MPI;
	MEMORYMANAGERPAGES Pages;

    CMemory()
    {
    	Initialize();
    }
    ~CMemory()
    {
    	Destroy();
    }

private:
	void ThrowExcept(char *sText);
	char *pBaseMemAddress;		//Base of system memory.
	HGLOBAL hMem;
	void *pMem;
};

extern CMemory *pMem;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

