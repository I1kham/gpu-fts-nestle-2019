#ifndef _rheaHandleUID88_h_
#define _rheaHandleUID88_h_
#include "rheaDataTypes.h"

namespace rhea
{
    /*=====================================================================================
     * UID88
     * 8bit counter, 8bit index
     *====================================================================================*/
    struct UID88
    {
                            UID88 ()								{ _handle=u16MAX; }
                            UID88 (const UID88 &b)                  { _handle=b._handle; }

        void				init (u32 idx, u32 ct) 					{ assert(idx<=0xFF && ct<=0xFF); _handle = idx | (ct<<8); }
        void				initFromU32 (u32 p)						{ _handle = (u16)(p & 0x0000FFFF); }
        void				setIndex (u32 idx)  					{ assert(idx<=0xFF); _handle = (_handle & 0xFF00) | idx; }
        void				setCounter (u32 ct) 					{ assert(ct <=0xFF); _handle = (_handle & 0x00FF) | ((ct & 0x000000FF) << 8); }
        void				incCounter () 							{ _handle += 0x0100; }
        u8					getIndex  () const						{ return (_handle & 0x00FF); }
        u8					getCounter () const                     { return (_handle & 0xFF00) >> 8; }
        void				setInvalid () 							{ _handle = u16MAX; }
        bool				isInvalid ()  const						{ return (_handle == u16MAX); }

        bool				operator== (const UID88 &b) const		{ return _handle==b._handle; }
        bool				operator!= (const UID88 &b) const		{ return _handle!=b._handle; }
        u32					asU32() const							{ return (u32)_handle; }

        static UID88		createInvalid()							{ UID88 u; u.setInvalid(); return u; }

    public:
         volatile u16		_handle;
    };
} //namespace rhea


//handle che usa UID88
#define RHEATYPEDEF_HANDLE88(NomeTipo)\
    typedef struct s##NomeTipo\
    {\
                s##NomeTipo()								{ setInvalid(); }\
                s##NomeTipo(const s##NomeTipo &b)			{ uid = b.uid; }\
    \
        void	init (u32 idx, u32 ct) 						{ uid.init (idx, ct); }\
        void	init (const rhea::UID88 &id)                { uid=id; }\
        void	initFromU32 (u32 id)						{ uid.initFromU32(id); }\
        void	setIndex (u32 idx) 							{ uid.setIndex(idx); }\
        void	setCounter (u32 ct)							{ uid.setCounter(ct); }\
        void	incCounter () 								{ uid.incCounter(); }\
        u8		getIndex  ()  const							{ return uid.getIndex(); }\
        u8		getCounter ()  const						{ return uid.getCounter(); }\
        void	setInvalid () 								{ uid.setInvalid(); }\
        bool	isInvalid () const							{ return uid.isInvalid(); }\
    \
        bool	operator== (const s##NomeTipo &b) const		{ return uid==b.uid; }\
        bool	operator!= (const s##NomeTipo &b) const		{ return uid!=b.uid; }\
        u32		asU32() const								{ return uid.asU32(); }\
    \
        static s##NomeTipo		createInvalid()				{ s##NomeTipo u; u.setInvalid(); return u; }\
    private:\
        rhea::UID88	uid;\
    } NomeTipo;



#endif // _rheaHandleUID88_h_

