#include "CR90File.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace socketbridge;


//***************************************************
CR90File::CR90File()
{
	localAllocator = rhea::getScrapAllocator();
	values = NULL;
}

//***************************************************
CR90File::~CR90File()
{
	priv_free();
}

//***************************************************
void CR90File::priv_free()
{
	if (NULL != values)
	{
		RHEAFREE(localAllocator, values);
		values = NULL;
	}
}

//***************************************************
void CR90File::priv_new()
{
	const u32 size = sizeof(u16) * (2 + CELLX * CELLY);
	values = RHEAALLOCT(u16*, localAllocator, size);
	memset (values, 0, size);
	values[0] = CELLX;
	values[1] = CELLY;

    priv_newLOG();
}

//***************************************************
void CR90File::priv_newLOG()
{
    rhea::DateTime dt;
    dt.setNow();
    char ts[64];
    dt.formatAs_YYYYMMDDHHMMSS (ts, sizeof(ts), ' ', '-', ':');

    u8 fname[256];
    rhea::string::utf8::spf (fname, sizeof(fname), "%s/current/CR90-log.txt", rhea::getPhysicalPathToAppFolder());
    FILE *f = rhea::fs::fileOpenForWriteText (fname);
    fprintf (f, "%s File created\n", ts);
    rhea::fs::fileClose(f);
}

//***************************************************
void CR90File::resetFile()
{
    priv_new();
    save();
}

//***************************************************
void CR90File::load()
{
	priv_free();

	u8 fname[512];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/current/CR90-data.dat", rhea::getPhysicalPathToAppFolder());

	u32 fsize;
	u8 *buffer8 = rhea::fs::fileCopyInMemory (fname, localAllocator, &fsize);
	if (NULL == buffer8)
	{
		priv_new();
		save();
	}
	else
	{
		u16 *buff16 = reinterpret_cast<u16*>(buffer8);
		if (buff16[0] == CELLX && buff16[1] == CELLY)
			values = buff16;
		else
		{
			priv_new();
			save();
		}
	}

    rhea::string::utf8::spf (fname, sizeof(fname), "%s/current/CR90-log.txt", rhea::getPhysicalPathToAppFolder());
    if (!rhea::fs::fileExists(fname))
        priv_newLOG();

}

//***************************************************
void CR90File::save() const
{
	u8 fname[512];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/current/CR90-data.dat", rhea::getPhysicalPathToAppFolder());
	FILE *f = rhea::fs::fileOpenForWriteBinary (fname);
	rhea::fs::fileWrite (f, values, sizeof(u16) * (2 + CELLX * CELLY));
	rhea::fs::fileClose(f);

	//salvo anche in formato ASCII
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/current/CR90-data.txt", rhea::getPhysicalPathToAppFolder());
	u32 ct = 2;
	f = rhea::fs::fileOpenForWriteText (fname);
	for (u16 y = 0; y < CELLY; y++)
	{
		for (u16 x = 0; x < CELLX; x++)
		{
			if (0 == values[ct])
				fprintf (f, "---- ");
			else
				fprintf (f, "%04d ", values[ct]);
			ct++;
		}
		fprintf (f, "\n");
	}
	rhea::fs::fileClose(f);
}

//***************************************************
u16 CR90File::getValue (u16 index) const
{
	if (index < CELLX * CELLY)
		return values[2 + index];

	DBGBREAK;
	return 0;
}

//***************************************************
void CR90File::setValue (u16 index, u16 value)
{
	if (index < CELLX * CELLY)
		values[2 + index] = value;
	else
	{
		DBGBREAK;
	}

    //logga data e ora del click
    if (value != 0)
    {
        const u16 row = index / CELLX;
        const u16 col = index - (row * CELLX);

        rhea::DateTime dt;
        dt.setNow();

        char ts[64];
        dt.formatAs_YYYYMMDDHHMMSS (ts, sizeof(ts), ' ', '-', ':');

        u8 fname[512];
        rhea::string::utf8::spf (fname, sizeof(fname), "%s/current/CR90-log.txt", rhea::getPhysicalPathToAppFolder());
        FILE *f = rhea::fs::fileOpenForAppendText (fname);
        if (NULL != f)
        {
            fprintf (f, "%s (%d,%d) = %d\n", ts, row, col, value);
            rhea::fs::fileClose(f);
        }

    }
}
