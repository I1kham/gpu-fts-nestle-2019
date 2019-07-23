#ifndef _SelectionAvailability_h_
#define _SelectionAvailability_h_
#include "../rheaCommonLib/rheaBit.h"

/*****************************************
 * SelectionAvailability
 *
 * Internamente utilizza il set di fn rhea::bit per la manipolazione dei bit di un buffer
 */
class SelectionAvailability
{
public:
                SelectionAvailability()                         { reset(); }

    void        reset ()                                        { rhea::bit::zero(flag, sizeof(flag)); }

    bool        isAvail (u8 selNumberStartingFromOne) const     { return rhea::bit::isSet (flag, sizeof(flag), selNumberStartingFromOne-1); }
    bool        areAllNotAvail() const                          { return (flag[0]==0 && flag[1] == 0); }

    void        setAsAvail (u8 selNumberStartingFromOne)        { assert(selNumberStartingFromOne>0); rhea::bit::set (flag, sizeof(flag), selNumberStartingFromOne-1); }
    void        setAsNotAvail (u8 selNumberStartingFromOne)     { assert(selNumberStartingFromOne>0); rhea::bit::unset (flag, sizeof(flag), selNumberStartingFromOne-1); }

    const u32*  getBitSequence() const                              { return flag; }

private:
    u32         flag[2];    //1 bit per ogni selezione
};

#endif
