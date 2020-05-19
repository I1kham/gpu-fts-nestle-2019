#include "DA3.h"
#include "../rheaCommonLib/rheaUtils.h"


//*********************************************
DA3::DA3()
{
	allocator = NULL;
	blob = NULL;
	sizeOfBlob = 0;
    fullFilePathAndName = NULL;
}

//*********************************************
void DA3::free()
{
	if (allocator && blob)
	{
		RHEAFREE(allocator, blob);
        RHEAFREE(allocator, fullFilePathAndName);
	}
	allocator = NULL;
	blob = NULL;
	sizeOfBlob = 0;
    fullFilePathAndName = NULL;
}


//*********************************************
bool DA3::loadInMemory(rhea::Allocator *allocatorIN, const u8* const utf8_fullFilePathAndNameIN, cpubridge::eCPUMachineType machineType, u8 machineModel)
{
	free();

	sizeOfBlob = 0;
	blob = rhea::fs::fileCopyInMemory (utf8_fullFilePathAndNameIN, allocatorIN, &sizeOfBlob);
	if (NULL == blob)
		return false;
	allocator = allocatorIN;
    fullFilePathAndName = rhea::string::utf8::allocStr(allocator, utf8_fullFilePathAndNameIN);
    
    blob[LOC_MACHINE_TYPE] = (u8)machineType;
    blob[LOC_MACHINE_MODEL] = machineModel;
	return true;
}

//*********************************************
void DA3::reload()
{
    if (NULL == blob)
		return;

    const u8 machineType = blob[LOC_MACHINE_TYPE];
    const u8 machineModel = blob[LOC_MACHINE_MODEL];
    
    FILE *f = rhea::fs::fileOpenForReadBinary (fullFilePathAndName);
    if (NULL == f)
        return;
    
    u32 CHUNK = 1024;
	u32 ct = 0;
    u32 fsize = sizeOfBlob;
	while (fsize >= CHUNK)
	{
		fread (&blob[ct], CHUNK, 1, f);
		fsize -= CHUNK;
		ct += CHUNK;
	}

	if (fsize)
		fread(&blob[ct], fsize, 1, f);    
    fclose(f);
    
    blob[LOC_MACHINE_TYPE] = (u8)machineType;
    blob[LOC_MACHINE_MODEL] = machineModel;
}


//*********************************************
void DA3::save(const u8* const utf8_fullFilePathAndName)
{
	if (NULL == blob)
		return;

	const u32 SIZE = 1024;
	FILE *f = rhea::fs::fileOpenForWriteBinary (utf8_fullFilePathAndName);
	if (NULL == f)
		return;

	u32 fileSize = sizeOfBlob;
	u32 ct = 0;
	while (fileSize >= SIZE)
	{
		fwrite(&blob[ct], SIZE, 1, f);
		ct += SIZE;
		fileSize -= SIZE;
	}
	if (fileSize)
		fwrite(&blob[ct], fileSize, 1, f);
	fclose(f);
}

//*********************************************
u8 DA3::readU8(u32 location) const
{
	if (NULL == blob) return 0;
	if (location >= sizeOfBlob) return 0;
	return blob[location];
}

//*********************************************
void DA3::writeU8(u32 location, u8 value)
{
	if (NULL == blob) return;
	if (location >= sizeOfBlob) return;
	blob[location] = value;
}

//*********************************************
u16	DA3::readU16(u32 location) const
{
	if (NULL == blob) return 0;
	if (location >= sizeOfBlob-1) return 0;
	return rhea::utils::bufferReadU16_LSB_MSB(&blob[location]);
}

//*********************************************
void DA3::writeU16(u32 location, u16 value)
{
	if (NULL == blob) return;
	if (location >= sizeOfBlob - 1) return;
	rhea::utils::bufferWriteU16_LSB_MSB(&blob[location], value);
}

//*********************************************
u32 DA3::priv_getDecounterLimitLocation (cpubridge::eCPUProgrammingCommand_decounter d) const
{
    if (d == cpubridge::eCPUProgrammingCommand_decounter_waterFilter)
        return LOC_DECOUNTER_WATER_FILTER;
    if (d == cpubridge::eCPUProgrammingCommand_decounter_coffeeBrewer)
        return LOC_DECOUNTER_COFFEE_BREWER;
    if (d == cpubridge::eCPUProgrammingCommand_decounter_coffeeGround)
        return LOC_DECOUNTER_COFFEE_GROUND;

    if (d >= cpubridge::eCPUProgrammingCommand_decounter_prodotto1 && d <= cpubridge::eCPUProgrammingCommand_decounter_prodotto10)
    {
        const u8 i = (u8)d - (u8)cpubridge::eCPUProgrammingCommand_decounter_prodotto1;
        return (LOC_DECOUNTER_PROD +i*2);
    }
    return 0;
}

//*********************************************
u16 DA3::getDecounterLimit (cpubridge::eCPUProgrammingCommand_decounter d) const
{
    u32 loc = priv_getDecounterLimitLocation(d);
    if (0 == loc)
        return 0;
    return readU16(loc);
}

//*********************************************
void DA3::setDecounterLimit(cpubridge::eCPUProgrammingCommand_decounter d, u16 value)
{
    u32 loc = priv_getDecounterLimitLocation(d);
    if (0 == loc)
        return;
    return writeU16(loc, value);
}

//*********************************************
u16 DA3::getProductQty_cannisterCapacity (u8 iProdotto_1_10) const
{
    u8 u = (u8)cpubridge::eCPUProgrammingCommand_decounter_prodotto1;
    u += (iProdotto_1_10-1);
    return getDecounterLimit ((cpubridge::eCPUProgrammingCommand_decounter)u);
}

//*********************************************
void DA3::setProductQty_cannisterCapacity (u8 iProdotto_1_10, u16 value)
{
    u8 u = (u8)cpubridge::eCPUProgrammingCommand_decounter_prodotto1;
    u += (iProdotto_1_10-1);
    setDecounterLimit ((cpubridge::eCPUProgrammingCommand_decounter)u, value);
}

//*********************************************
u16 DA3::getProductQty_warningAt (u8 iProdotto_1_10) const
{
    const u32 loc = LOC_DECOUNTER_PROD +2*getNumProdotti() + (iProdotto_1_10-1)*2;
    return readU16(loc);
}
//*********************************************
void DA3::setProductQty_warningAt (u8 iProdotto_1_10, u16 value)
{
    const u32 loc = LOC_DECOUNTER_PROD +2*getNumProdotti() + (iProdotto_1_10-1)*2;
    writeU16(loc, value);
}
//*********************************************
u8 DA3::getProductQty_enableStop (u8 iProdotto_1_10) const
{
    const u32 loc = LOC_DECOUNTER_PROD +4*getNumProdotti() + (iProdotto_1_10-1);
    return readU8(loc);
}
//*********************************************
void DA3::setProductQty_enableStop (u8 iProdotto_1_10, u8 value)
{
    const u32 loc = LOC_DECOUNTER_PROD +4*getNumProdotti() + (iProdotto_1_10-1);
    writeU8(loc, value);
}






















