////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_ServerWindowMap_Entity {
	HWND hWnd;
	SockServ *pSrv;
} CLASSMAPENTITY, *LPCLASSMAPENTITY;

typedef struct _tag_ServerWindowMap_Entities {
	CLASSMAPENTITY *Collection;
	int Count;
} CLASSMAPENTITIES, *LPCLASSMAPENTITIES;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ServerWindowMap {
private:
	CLASSMAPENTITIES Entities;

public:
	ServerWindowMap()
    {
	    Entities.Count = 0;
        Entities.Collection = NULL;
    }

	~ServerWindowMap()
    {
        if(Entities.Count > 0)
        {
		    free(Entities.Collection);
        }
	    Entities.Count = 0;
    }

	void Upsert(HWND hWnd, SockServ *pSrv)
    {
	    Entities.Count++;

	    Entities.Collection = (CLASSMAPENTITY*) realloc(Entities.Collection, sizeof(CLASSMAPENTITY) * Entities.Count);
        Entities.Collection[Entities.Count - 1].hWnd = hWnd;
        Entities.Collection[Entities.Count - 1].pSrv = pSrv;
    }

    SockServ *At(HWND hWnd)
    {
    	for(int i = 0; i < Entities.Count; i++)
        {
	        if(Entities.Collection[i].hWnd == hWnd)
            {
            	return Entities.Collection[i].pSrv;
            }
        }

		return NULL;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
