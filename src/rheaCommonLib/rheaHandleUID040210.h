#ifndef _rheaHandleUID040210_h_
#define _rheaHandleUID040210_h_
#include "rheaDataTypes.h"

namespace rhea
{
    /*=====================================================================================
     * UID040210
     * 4bit counter, 2bit reserved, 10bit index
     *
     * i "reserved bit" non sono presi in considerazione durante le operazioni di == e !=
     *====================================================================================*/
    struct UID040210
    {
                            UID040210 ()                                { setInvalid(); }
                            UID040210 (const UID040210 &b)              { _handle=b._handle; }

        void				init (u32 idx, u32 ct, u32 reserved=0)      { setIndex(idx); setCounter(ct); setReserved(reserved); }
        void				initFromU32 (u32 p)                         { _handle = (u16)(p & 0x0000FFFF); }
        void				setIndex (u32 idx)                          { assert(idx<1024); _handle = (_handle & 0xFC00) | (idx & 0x03FF); }
        void				setCounter (u32 ct)                         { assert(ct<16);    _handle = (_handle & 0x0FFF) | ((ct & 0x0F) << 12); }
        void				incCounter ()                               { _handle += 0x1000; }
        void				setReserved (u32 r)                         { assert(r<4); _handle |= ((r & 0x03) << 10); }
        u8					getIndex  () const                          { return (_handle & 0x03FF); }
        u8					getCounter () const                         { return (_handle & 0xF000) >> 12; }
        u8                  getReservedBit() const                      { return ((_handle & 0x0C00) >> 10); }

        void				setInvalid ()                               { _handle = u16MAX; }
        bool				isInvalid ()  const                         { return (_handle == u16MAX); }

        bool				operator== (const UID040210 &b) const		{ return (_handle & 0xF3FF) == (b._handle & 0xF3FF); }
        bool				operator!= (const UID040210 &b) const		{ return (_handle & 0xF3FF) != (b._handle & 0xF3FF); }
        u32					asU32() const                               { return (u32)_handle; }

        static UID040210    createInvalid()                             { UID040210 u; u.setInvalid(); return u; }

    public:
         volatile u16		_handle;
    };
} //namespace rhea


//handle che usa UID040210
#define RHEATYPEDEF_HANDLE040210(NomeTipo)\
    typedef struct s##NomeTipo\
    {\
                s##NomeTipo()                                   { setInvalid(); }\
                s##NomeTipo(const s##NomeTipo &b)               { uid = b.uid; }\
    \
        void	init (u32 idx, u32 ct, u32 reserved=0)          { uid.init (idx, ct, reserved); }\
        void	init (const rhea::UID040210 &id)                { uid=id; }\
        void	initFromU32 (u32 id)                            { uid.initFromU32(id); }\
        void	setIndex (u32 idx)                              { uid.setIndex(idx); }\
        void	setCounter (u32 ct)                             { uid.setCounter(ct); }\
        void	incCounter ()                                   { uid.incCounter(); }\
        void    setReserved (u32 r)                             { uid.setReserved(r); }\
        u8		getIndex  ()  const                             { return uid.getIndex(); }\
        u8		getCounter ()  const                            { return uid.getCounter(); }\
        u8      getReservedBit() const                          { return uid.getReservedBit(); }\
        void	setInvalid ()                                   { uid.setInvalid(); }\
        bool	isInvalid () const                              { return uid.isInvalid(); }\
    \
        bool	operator== (const s##NomeTipo &b) const         { return uid==b.uid; }\
        bool	operator!= (const s##NomeTipo &b) const         { return uid!=b.uid; }\
        u32		asU32() const                                   { return uid.asU32(); }\
    \
        static s##NomeTipo		createInvalid()                 { s##NomeTipo u; u.setInvalid(); return u; }\
    private:\
        rhea::UID040210	uid;\
    } NomeTipo;



#endif // _rheaHandleUID040210_h_

