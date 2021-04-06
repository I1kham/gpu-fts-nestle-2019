#include "rhea.h"
#include "rheaMemoryTracker.h"
#include "rheaDateTime.h"
#include "rheaCriticalSection.h"

using namespace rhea;


//*****************************************************
MemoryTracker::MemoryTracker ()
{
	rhea::criticalsection::init(&cs);
	allocID = 0;
	root = NULL;

	char fullFilename[256];
	sprintf_s (fullFilename, sizeof(fullFilename), "%s/memoryTracker.txt", rhea::getPhysicalPathToAppFolder());
	f = fopen(fullFilename, "wt");
	//fprintf(f, "%16.16s ALLOC %05d 0x%08X %012d\n", fromWho, s->allocID, PTR_TO_INT(p), allocatedSizeInByte);
	//fprintf(f, "---------------- ALLOC ----- ---------- ---------- ------------
	

	char dateTimeNow[24];
	DateTime dt;
	dt.setNow();
	dt.formatAs_YYYYMMDDHHMMSS(dateTimeNow, sizeof(dateTimeNow));
	fprintf(f, "%s\n", dateTimeNow);
	fprintf(f, "--ALLOCATOR NAME -WHAT ---ID --PTR FROM --------TO --------SIZE FILENAME AND LINE\n");
}

//*****************************************************
MemoryTracker::~MemoryTracker()
{
	rhea::criticalsection::close(cs);

	while (root)
	{
		sRecord *p = root;
		root = root->next;
		free(p);
	}
	rhea::fs::fileClose(f);
}

//*****************************************************
void MemoryTracker::onAlloc(u32 allocatorID, const char *allocatorName, const void *p, u32 allocatedSizeInByte, const char *debug_filename, u32 debug_lineNumber)
{
	rhea::criticalsection::enter(cs);
		sRecord *s = (sRecord*)malloc(sizeof(sRecord));
		s->p = p;
		s->allocatorID = allocatorID;
		s->allocID = allocID++;;
		s->next = root;
		root = s;

		IntPointer from = PTR_TO_INT(p);
		IntPointer to = from + allocatedSizeInByte;
        fprintf (f, "%16.16s ALLOC %05d 0x%08X 0x%08X % 12d ", allocatorName, s->allocID, (u32)from, (u32)to, allocatedSizeInByte);

		if (debug_filename != NULL)
			fprintf(f, "%s [line:%d]\n", debug_filename, debug_lineNumber);
		else
			fprintf(f, "? [line:%d]\n", debug_lineNumber);
		fflush(f);

		
	rhea::criticalsection::leave(cs);
}

//*****************************************************
void MemoryTracker::onDealloc (u32 allocatorID, const char *allocatorName, const void *p, u32 allocatedSizeInByte)
{
	rhea::criticalsection::enter(cs);
	sRecord *s = root;
	while (s)
	{
		if (s->p == p)
		{
			assert(s->allocatorID == allocatorID);
			IntPointer from = PTR_TO_INT(p);
			IntPointer to = from + allocatedSizeInByte;
            fprintf(f, "%16.16s DEALL %05d 0x%08X 0x%08X % 12d\n", allocatorName, s->allocID, (u32)from, (u32)to, allocatedSizeInByte);
			fflush(f);

			//verifico che non sia già stato deleted
			assert((s->allocID & 0x80000000) == 0);

			//lo marco come deletato
			s->allocID |= 0x80000000;

			rhea::criticalsection::leave(cs);
			return;
		}

		s = s->next;
	}
	rhea::criticalsection::leave(cs);


	//non ho trovato *p nella lista delle allocazioni!
	DBGBREAK;

}

//*****************************************************
void MemoryTracker::finalReport (u32 allocatorID, const char *allocatorName)
{
    rhea::criticalsection::enter(cs);

    fprintf (f, "\n\n================= FINAL REPORT FOR allocator [%d][%s]===============\n", allocatorID, allocatorName);
    fprintf (f, "List of undeleted pointer:\n");
    fprintf(f, "ALLOC.ID PTR-FROM\n");
    sRecord *s = root;
    while (s)
    {
        if (s->allocatorID == allocatorID)
        {
            //verifico che non sia già stato deleted
            if ( (s->allocID & 0x80000000) == 0)
            {
                IntPointer from = PTR_TO_INT(s->p);
                fprintf(f, "%08d 0x%08X\n", s->allocID, (u32)from);
            }
        }
        s = s->next;
    }
    fflush(f);
    rhea::criticalsection::leave(cs);
}
