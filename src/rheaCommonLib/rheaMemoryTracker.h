#ifndef _rheaMemoryTracker_h_
#define _rheaMemoryTracker_h_
#include "rheaDataTypes.h"
#include "OS/OS.h"

namespace rhea
{
	class MemoryTracker
	{
	public:
		struct sRecord
		{
			const void 	*p;
			u32			allocID;
			u32			allocatorID;
			sRecord		*next;
		};

	public:
							MemoryTracker ();
							~MemoryTracker();

		void				onAlloc (u32 allocatorID, const char *allocatorName, const void *p, u32 allocatedSizeInByte, const char *debug_filename, u32 debug_lineNumber);
		void				onDealloc(u32 allocatorID, const char *allocatorName, const void *p, u32 allocatedSizeInByte);

		sRecord*			getRoot() const { return root; }


	private:
		FILE				*f;
		OSCriticalSection	cs;
		sRecord				*root;
		u32					allocID;
	};
} // namespace rhea




#endif // _rheaMemoryTracker_h_

