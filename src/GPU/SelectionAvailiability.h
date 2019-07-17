#ifndef _SelectionAvailability_h_
#define _SelectionAvailability_h_
#include "../rheaCommonLib/rheaDataTypes.h"
#include <string.h>

/*****************************************
 * SelectionAvailability
 *
 */
class SelectionAvailability
{
public:
                SelectionAvailability()                         { reset(); }

    void        reset ()                                        { memset(flag,0,sizeof(flag)); }

    bool        isAvail (u8 selNumberStartingFromOne) const
                {
                    assert(selNumberStartingFromOne>0);
                    selNumberStartingFromOne--;
                    return (flag[selNumberStartingFromOne>>5] & (0x00000001 << (selNumberStartingFromOne & 0x1F))) != 0;
                }

    bool        areAllNotAvail() const                          { return (flag[0]==0 && flag[1] == 0); }

    void        setAsAvail (u8 selNumberStartingFromOne)
                {
                    assert(selNumberStartingFromOne>0);
                    selNumberStartingFromOne--;
                    const u8 byte = selNumberStartingFromOne>>5;
                    const u8 bit = selNumberStartingFromOne & 0x1F;
                    flag[byte] |=  (0x00000001 << bit);
                }

    void        setAsNotAvail (u8 selNumberStartingFromOne)
                {
                    assert(selNumberStartingFromOne>0);
                    selNumberStartingFromOne--;
                    const u8 byte = selNumberStartingFromOne>>5;
                    const u8 bit = selNumberStartingFromOne & 0x1F;
                    flag[byte] &=  (~(0x00000001 << bit));
                }
private:
    u32 flag[2];
};

#endif
