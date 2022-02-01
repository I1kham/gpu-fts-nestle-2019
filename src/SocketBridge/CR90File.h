#ifndef _CR90File_h_
#define _CR90File_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../rheaCommonLib/rheaRandom.h"
#include "SocketBridgeEnumAndDefine.h"
#include "SocketBridgeFileTEnumAndDefine.h"

namespace socketbridge
{
    class CR90File
    {
    public:
                    CR90File();
                    ~CR90File();

        void        load();
        void        save() const;
        u16         getValue (u16 index) const;
        void        setValue (u16 index, u16 value);
    
        u16         getNumCellX() const { return CELLX; }
        u16         getNumCellY() const { return CELLY; }

    private:
        static const u8 CELLX = 16;
        static const u8 CELLY = 15;

    private:
        void        priv_free();
        void        priv_new();

    private:
        rhea::Allocator *localAllocator;
        u16             *values;

    };

} // namespace socketbridge

#endif // _CR90File_h_
