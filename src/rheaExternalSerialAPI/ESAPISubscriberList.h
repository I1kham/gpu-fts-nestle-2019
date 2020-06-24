#ifndef _ESAPISubscriberList_h_
#define _ESAPISubscriberList_h_
#include "ESAPIEnumAndDefine.h"

namespace esapi
{
	class SubscriberList
	{
	public:
					SubscriberList();
					~SubscriberList()													{ }

		void			setup (rhea::Allocator *allocator);
		void			unsetup();

		sSubscription*	onSubscriptionRequest (rhea::Allocator *allocator, rhea::ISimpleLogger *logger, const rhea::thread::sMsg &msg);
		void			unsubscribe (rhea::Allocator *allocator, sSubscription *sub);

		sSubscription*	findByOSEvent (const OSEvent &h);

	private:
		rhea::FastArray<sSubscription*>	list;
		
	};


} // namespace esapi

#endif // _ESAPISubscriberList_h_
