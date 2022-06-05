////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMEMORY_CPP_
#define _CMEMORY_CPP_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Memory mapping & handing routines.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <STDLib.H>
#include <STDIO.H>
#include <String.H>
#include <StdArg.H>

#include "CMemory.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMemory *pMem = new CMemory();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::GetStatusInfo(MEMORYSTATUSINFO *pMSI)
{
	pMSI->uBytesTotal = MPI.uPhysicalGrant;
	pMSI->uBytesUsed = MPI.uBase + (Pages.uUsed * Pages.uSize);
	pMSI->uBytesFree = pMSI->uBytesTotal - pMSI->uBytesUsed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::GetPhysicalInfo(MEMORYPHYSICALINFO *pMPI)
{
	memcpy(pMPI, &MPI, sizeof(MEMORYPHYSICALINFO));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::GetPageInfo(MEMORYMANAGERPAGES *pMMP)
{
	memcpy(pMMP, &this->Pages, sizeof(MEMORYMANAGERPAGES));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Initialize the memory manager.
*/
bool CMemory::Initialize(void)
{
	DWORD dwRequest = 1024ul * 1024ul * 4ul;

	//Allocate a moveable block of memory (returns a handle) (Integer type = 2 bytes)
	hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwRequest);

    //How much memory did we actually get?
	DWORD dwPhysicalGrant = GlobalSize(hMem);

	//Lock the memory block, returning a pointer to it
	pMem = GlobalLock(hMem);

	// Place the starting address of all usable "user mode" memory at the "2 megabyte barrier".
	pBaseMemAddress = (char *) pMem;
	memset(&this->MPI, 0, sizeof(MEMORYPHYSICALINFO));
	MPI.uBase = (DWORD)pMem;
	MPI.uPhysicalRequest = dwRequest;
	MPI.uPhysicalGrant = dwPhysicalGrant;
	MPI.uUnreserved = dwPhysicalGrant;

	//Initialize the global memory manager structure.
	memset(&this->Pages, 0, sizeof(MEMORYMANAGERPAGES));
	Pages.uSize = MEMORY_MANAGER_PAGE_SIZE;
	Pages.uCount = MPI.uUnreserved / Pages.uSize;
	Pages.uBaseAddress = (DWORD)MPI.uBase;
	Pages.uAttribs = (char *)Pages.uBaseAddress;
	Pages.uManagerPages = ((sizeof(char) * Pages.uCount) / Pages.uSize);
    Pages.uUsed = Pages.uManagerPages;

	if(((sizeof(char) * Pages.uCount) % Pages.uSize) > 0)
	{
		Pages.uManagerPages++;
	}

	//Initialize all of the page attribs to zero.
	memset(Pages.uAttribs, 0, sizeof(char) * Pages.uCount);

	//Reserve memory for internal use by the manager.
	for(DWORD uPage = 0; uPage < Pages.uManagerPages; uPage++)
	{
		Pages.uAttribs[uPage] = MEMORY_PAGE_ATTRIB_USED | MEMORY_PAGE_ATTRIB_MANAGER;
		Pages.uUsed++;
	}
	//Mark the last block in a chain of pages as the "End of Chain".
	Pages.uAttribs[Pages.uManagerPages - 1] |= MEMORY_PAGE_ATTRIB_ENDOFCHAIN;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::Destroy(void)
{
	//Unlock the memory block, destroying the pointer and freeing resources
	GlobalUnlock(hMem);

	//Free the memory block (de-allocate it)
	GlobalFree(hMem);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMemory::ThrowExcept(char *sText)
{
	MessageBox(NULL, sText, "CMemory", 0);
	throw(sText);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Free allocated RAM.
*/
bool CMemory::Free(void *pAddress)
{
	DWORD uPage = (((DWORD)pAddress) - Pages.uBaseAddress) / Pages.uSize;

    //char sText[255];
    //sprintf(sText, "%lu", uPage);
    //MessageBox(NULL, sText, "CMemory", 0);

	//Perform a few basic checks and basic memory protection.
	if(uPage < 0)
	{
		ThrowExcept("Attempt to free sub-base memory page!");
	}
	else if(!(Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_USED))
	{
		ThrowExcept("Attempt to free unused memory page!");
	}
	else if((Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_MANAGER))
	{
		ThrowExcept("Attempt to free memory manager page!");
	}
	else if(uPage > 0
		&& (Pages.uAttribs[uPage - 1] &MEMORY_PAGE_ATTRIB_USED)
		&& !(Pages.uAttribs[uPage - 1] &MEMORY_PAGE_ATTRIB_ENDOFCHAIN))
	{
		ThrowExcept("Attempt to free non-root memory page in page chain!");
	}

	//Free the allocated pages.
	for(; uPage < Pages.uCount; uPage++)
	{
		Pages.uUsed--;
		if(Pages.uAttribs[uPage] & MEMORY_PAGE_ATTRIB_ENDOFCHAIN)
		{
			Pages.uAttribs[uPage] = 0;
			return true;
		}
		Pages.uAttribs[uPage] = 0;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Re-Allocate RAM / Resize allocated block (Will relocate the pages if necessary).
*/
void *CMemory::ReAlloc(void *pAddress, DWORD uTotalBytes)
{
	if(!pAddress)
	{
		return this->Alloc(uTotalBytes);
	}

	DWORD uFirstPage = (((DWORD)pAddress) - Pages.uBaseAddress) / Pages.uSize;
	DWORD uLastPage = uFirstPage;

	//Perform a few basic checks and basic memory protection.
	if(!(Pages.uAttribs[uFirstPage] &MEMORY_PAGE_ATTRIB_USED))
	{
		ThrowExcept("Attempt to realloc unused memory page!");
	}
	else if((Pages.uAttribs[uFirstPage] &MEMORY_PAGE_ATTRIB_MANAGER))
	{
		ThrowExcept("Attempt to realloc memory manager page!");
	}
	else if(uFirstPage > 0
		&& (Pages.uAttribs[uFirstPage - 1] &MEMORY_PAGE_ATTRIB_USED)
		&& !(Pages.uAttribs[uFirstPage - 1] &MEMORY_PAGE_ATTRIB_ENDOFCHAIN))
	{
		ThrowExcept("Attempt to realloc non-root memory page in page chain!");
	}

	//Find the end of the chain of pages.
	for(uLastPage = uFirstPage; uLastPage < Pages.uCount; uLastPage++)
	{
		if(Pages.uAttribs[uLastPage] &MEMORY_PAGE_ATTRIB_ENDOFCHAIN)
		{
			break;
		}
	}

	//Figure out how many pages we need.
	DWORD uCurrentPages = (uLastPage - uFirstPage) + 1;
	DWORD uPagesRequired = (uTotalBytes / Pages.uSize) + ((uTotalBytes % Pages.uSize) > 0);

	//If we already have enough pages, just return the old address.
	if(uPagesRequired <= uCurrentPages)
	{
		return pAddress; //Nothing to do, currently allocated page count is sufficient.
	}

	//Check to see of we can add pages to our current block of pages.
	DWORD uAddPages = (uPagesRequired - uCurrentPages); //How many more pages do we need?
	DWORD uAddPagesFound = 0;
	for(DWORD uOffset = 1; uOffset <= uAddPages; uOffset++)
	{
		if(Pages.uAttribs[uLastPage + uOffset] &MEMORY_PAGE_ATTRIB_USED)
		{
			break; //Not enough contiguous pages after our currently allocated block.
		}

		uAddPagesFound++;
	}

	if(uAddPagesFound == uAddPages)
	{
		//We found contiguous pages after our currently allocated pages.

		//Unflag what used to be the last page.
		Pages.uAttribs[uLastPage] &= ~MEMORY_PAGE_ATTRIB_ENDOFCHAIN;

		//Reserve the additonal pages.
		for(DWORD uOffset = 1; uOffset <= uAddPages; uOffset++)
		{
			Pages.uAttribs[uLastPage + uOffset] = MEMORY_PAGE_ATTRIB_USED;
			Pages.uUsed++;
		}
		Pages.uAttribs[uLastPage + uAddPages] |= (MEMORY_PAGE_ATTRIB_ENDOFCHAIN);
	}
	else{
		//We did not find the required number of contiguous pages, we have to relocate the memory.

		char *pNewAddress = (char *)this->Alloc(uTotalBytes);
		if(pNewAddress)
		{
			//Move our data from the old pages to the new pages.
			memcpy(pNewAddress, pAddress, ((uLastPage - uFirstPage) + 1) * Pages.uSize);
			this->Free(pAddress); //Free the old pages.
			return pNewAddress; //Return the new address.
		}
		else{
			return NULL; //Failed to find a large enough block of pages. The old address is still valid.
		}
	}

	return pAddress;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CMemory::StrDup(const char *str)
{
	int iLength = strlen(str);
	char *sResult = (char*)this->Alloc(iLength);
    memcpy(sResult, str, iLength);
    sResult[iLength] = '\0';
    return sResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocate RAM.
*/
void *CMemory::Alloc(DWORD uBytes)
{
	if(uBytes <= 0)
	{
		return NULL; //Requested an invalid number or bytes.
	}

	//Figure out how many pages we need.
	DWORD uPagesRequired = (uBytes / Pages.uSize) + ((uBytes % Pages.uSize) > 0);
	DWORD uContiguousPages = 0;
	
	//Search for contiguous pages.
	for(DWORD uPage = Pages.uManagerPages; uPage < Pages.uCount; uPage++)
	{
		if(!(Pages.uAttribs[uPage] &MEMORY_PAGE_ATTRIB_USED))
		{
			if(++uContiguousPages == uPagesRequired)
			{
				DWORD uStartingPage = ((uPage - uContiguousPages) + 1);

				//Reserve the memory pages. The last page in the block of is not chained to the next page.
				for(uContiguousPages = 0; uContiguousPages < uPagesRequired; uContiguousPages++)
				{
					this->Pages.uAttribs[uPage - uContiguousPages] = MEMORY_PAGE_ATTRIB_USED;
					Pages.uUsed++;
				}
				Pages.uAttribs[uPage] |= (MEMORY_PAGE_ATTRIB_ENDOFCHAIN);

				return ((char *)Pages.uBaseAddress) + (uStartingPage * Pages.uSize);
			}
		}
		else{
			uContiguousPages = 0; //Found a used page, reset the contiguous count.
		}
	}

	return NULL; //Insufficient memory or contiguous pages.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocate RAM and clear the reserved pages.
*/
void *CMemory::AllocZ(DWORD uElementCount, DWORD uElementSize)
{
	DWORD uBytes = uElementCount * uElementSize;
	void *pMemory = this->Alloc(uBytes);
	if(pMemory)
	{
		memset(pMemory, 0, uBytes);
	}
	return pMemory;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Do not call after user space memory is being used
		This function will clear data stored in RAM.
*/
bool CMemory::TestPhysical(void)
{
    const char sStatChars[5] = {'/', '-', '\\', '|', '\0'};

    unsigned short iStatChar = 0;
	DWORD uStatTick = ONEMEGABYTE * 10;
    DWORD uStatCount = uStatTick;

    bool bResut = true;

    printf("RAM Write\n");

    //Fill and Test RAM.
    for(DWORD iAddr = 0; iAddr < MPI.uUnreserved; iAddr++)
    {
        (*(pBaseMemAddress + iAddr)) = (unsigned char)iAddr;
        if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
        {
            printf("\nWrite fail at %d\n", (pBaseMemAddress + iAddr));
            bResut = false;
            break;
        }

        if(uStatCount++ == uStatTick)
        {
            if(!sStatChars[iStatChar])
            {
                iStatChar = 0;
            }
            uStatCount = 0;
        }
    }


    if(bResut)
    {
        printf("RAM Read\n");

        //Read and Test RAM.
        for(DWORD iAddr = 0; iAddr < MPI.uUnreserved; iAddr++)
        {
            if((*(pBaseMemAddress + iAddr)) != (unsigned char)iAddr)
            {
                printf("\nRead fail at %d\n", (pBaseMemAddress + iAddr));
                bResut = false;
                break;
            }

			//After debugging, we will want to init ram to zero - but for bebugging purposes,
			//	Lets make it easy to see overflow and set to to the AT sign.
            (*(pBaseMemAddress + iAddr)) = (unsigned char)'@'; //Initialize RAM.

            if(uStatCount++ == uStatTick)
            {
                if(!sStatChars[iStatChar])
                {
                    iStatChar = 0;
                }
                uStatCount = 0;
                //Sleep(1);
            }
        }
    }

    return bResut;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

