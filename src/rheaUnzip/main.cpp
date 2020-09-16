#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/compress/rheaCompress.h"
#include <direct.h>


#define VERSION "1.0, 2020-09-16"

//*****************************************************
int main (int argc, char *argv[])
{
	HINSTANCE hInst = NULL;
	rhea::init("rheaUnzip", &hInst);

	while (1)
	{
		if (argc < 2)
		{
			printf ("rheaUnzip, version " VERSION "\n");
			printf ("---------------------------------------------\n");
			printf ("  usage: rheaUnzip srcFile [dstFolder]\n");
			printf ("         [dstFolder] is optional. If not specified, [srcFile] will be unzipped in the current folder.\n\n");
			break;
		}

		char currentFolder[512];
		_getcwd (currentFolder, sizeof(currentFolder));
		//printf ("folder:%s\n", currentFolder);


		//src zip
		char src[512];
		bool bHasPath = false;
		if (strlen(argv[1]) >= 2)
		{
			if (argv[1][1] == ':')
				bHasPath = true;
		}
		
		if (bHasPath)
			strcpy_s (src, sizeof(src), argv[1]);
		else
			sprintf_s (src, sizeof(src), "%s\\%s", currentFolder, argv[1]);
		printf ("  src: %s\n", src);
		
		if (!rhea::fs::fileExists((const u8*)src))
		{
			printf ("  error, file %s does not exists\n", src);
			break;
		}
		
		//dst folder
		char dst[512];
		if (argc < 3)
			strcpy_s (dst, sizeof(dst), currentFolder);
		else
		{
			bHasPath = false;
			if (strlen(argv[2]) >= 2)
			{
				if (argv[2][1] == ':')
					bHasPath = true;
			}

			if (bHasPath)
				strcpy_s (dst, sizeof(dst), argv[2]);
			else
				sprintf_s (dst, sizeof(dst), "%s\\%s", currentFolder, argv[2]);
		}
		printf ("  dst folder: %s\n", dst);

		if (!rhea::fs::folderExists((const u8*)dst))
		{
			printf ("  error, folder %s does not exists\n", dst);
			break;
		}

		if (!rhea::CompressUtility::decompresAll ((const u8*)src, (const u8*)dst))
			printf ("  error decompressing\n");
		else
			printf ("  done!\n");
		break;
	}



    rhea::deinit();
	return 0;
}


