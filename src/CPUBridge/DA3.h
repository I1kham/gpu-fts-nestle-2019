#ifndef _DA3_h_
#define _DA3_h_
#include "../rheaCommonLib/rhea.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"

#define DA3_GET_SET_U8(ParamName,Location) \
    u8          get##ParamName() const              { return readU8(Location); }\
    void        set##ParamName(u8 u)                { writeU8(Location, u); }\


#define DA3_GET_SET_U16(ParamName,Location) \
    u16         get##ParamName() const              { return readU16(Location); }\
    void        set##ParamName(u16 u)               { writeU16(Location,u); }\

/****************************************************
 *
 */
class DA3
{
public:
					DA3();
					~DA3()																						{ free(); }

    bool			loadInMemory (rhea::Allocator *allocator, const char *fullFilePathAndName, cpubridge::eCPUMachineType machineType, u8 machineModel);
    void            reload();
	void			save (const char *fullFilePathAndName);
	void			free();

	u8				readU8(u32 location) const;
	void			writeU8(u32 location, u8 value);

	u16				readU16(u32 location) const;
	void			writeU16(u32 location, u16 value);

                    //============================= query generiche
    bool            isInstant() const                                                               { return (blob[LOC_MACHINE_TYPE] == 0); }
    bool            isEpresso() const                                                               { return !isInstant(); }
    u8              getModelCode() const                                                            { return blob[LOC_MACHINE_MODEL]; }
    u8              getNumProdotti() const                                                          { if (isEpresso()) return 6; return 10; }
    u16             getDecounterLimit (cpubridge::eCPUProgrammingCommand_decounter d) const;
    void            setDecounterLimit (cpubridge::eCPUProgrammingCommand_decounter d, u16 value);

                    //============================= MAINTENANCE
                    DA3_GET_SET_U8(ActivateH20Filter,7090)
                    DA3_GET_SET_U16(WaterFilterDecounter,LOC_DECOUNTER_WATER_FILTER)
                    DA3_GET_SET_U16(DecounterCoffeeBrewer,LOC_DECOUNTER_COFFEE_BREWER)
                    DA3_GET_SET_U16(DecounterCoffeeGround,LOC_DECOUNTER_COFFEE_GROUND)

                    //============================= PRODUCT QTY
    u16             getProductQty_cannisterCapacity (u8 iProdotto_1_10) const;
    void            setProductQty_cannisterCapacity (u8 iProdotto_1_10, u16 value);
    u16             getProductQty_warningAt (u8 iProdotto_1_10) const;
    void            setProductQty_warningAt (u8 iProdotto_1_10, u16 value);
    u8              getProductQty_enableStop (u8 iProdotto_1_10) const;
    void            setProductQty_enableStop (u8 iProdotto_1_10, u8 value);

                    //============================= MILKER
                    DA3_GET_SET_U16(Milker_steamTemp,7618)
                    DA3_GET_SET_U16(Milker_enableRinsing,7622)
                    DA3_GET_SET_U16(Milker_rinsingPeriod,7620)
                    DA3_GET_SET_U16(Milker_rinsingDelay,7624)
                    DA3_GET_SET_U16(Milker_enableSensor,7624)
                    DA3_GET_SET_U8(Milker_showCleanBtnAtStartup,7628)




private:
    static const u32    LOC_MACHINE_TYPE = 9465;
    static const u32    LOC_MACHINE_MODEL = 9466;
    static const u32    LOC_DECOUNTER_PROD = 7334;
    static const u32    LOC_DECOUNTER_WATER_FILTER = 7394;
    static const u32    LOC_DECOUNTER_COFFEE_BREWER = 7396;
    static const u32    LOC_DECOUNTER_COFFEE_GROUND = 7398;
    
private:
	rhea::Allocator *allocator;
    char            *fullFilePathAndName;
	u8				*blob;
	u32				sizeOfBlob;

private:
    u32             priv_getDecounterLimitLocation (cpubridge::eCPUProgrammingCommand_decounter d) const;
};


#endif // _DA3_h_
