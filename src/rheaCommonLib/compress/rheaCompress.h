#ifndef _rheaCompress_h_
#define _rheaCompress_h_
#include "../rhea.h"
#include "../string/rheaUTF8String.h"
#include "../rheaArray.h"

namespace rhea
{

	bool	compressFromMemory (const u8 *src, u32 numBytesToCompress, u8 compressionLevel, u8 *dst, u32 *in_out_sizeOfDst);
				/*	comprime i primi [numBytesToCompress] di [src] e li mette in [dst]
					[dst] è stato preallocato da qualunco ed è un buffer di dimensione [in_out_sizeOfDst]
					Se la fn ritorna true, allora in [in_out_sizeOfDst] c'è il numero di btye della versione compressa di [src] messa in [dst]

					[compressionLevel]:
						0 = no compression
						1 = best speed
						9 = best compression
					*/

	bool	compressFromMemory (const u8 *src, u32 numBytesToCompress, u8 compressionLevel, rhea::Allocator *allocator, u8 *out_dst, u32 *out_compressedSize);
				/* come sopra, ma il buffer [dst] viene allocato all'interno della fn usando [allocator]
				*/

	u32		compressCalcEstimatedCompressedSize (u32 srcLenInBytes);
				/* ritorna una stima molto abbondante delle dimensioni compresse di un buffer grosso [srcLenInBytes] bytes */

	bool	decompressFromMemory (const u8 *compressedSRC, u32 numBytesCompressed, u8 *out_decompressed, u32 *in_out_sizeOfDecompressed);
				/* decomprime i primi [numBytesCompressed] di [compressedSRC] e li mette in [out_decompressed];
					[out_decompressed] deve essere preallocato dall'esterno e la sua dimensione iniziale è [in_out_sizeOfDecompressed].
					Se la fn ritorn true, allora [in_out_sizeOfDecompressed] contiene il num di byte utilizzati per la decompressione
				*/


	/*****************************************************
	 * CompressUtility
	 *
	 * uitility per la creazione di file-archivio contenenti n file compressi
	 */
	class CompressUtility
	{
	public:
						CompressUtility();
						~CompressUtility();

		void			begin (const u8* const utf8_fullDstFileNameAndPath, u8 compressionLevel);
		bool				addFile (const u8* const utf8_fullSRCFileNameAndPath, const u8* const utf8_fullDSTFileNameAndPath);

		void				excludeFolder (const u8* const utf8_fullSRCFolderPathNoSlash);
		bool				addFilesInFolder (const u8* const utf8_fullSRCFolderPathNoSlash, const u8* const utf8_fullDSTFolderNameAndPathNoSlash, bool bRecurseSubFolder);
								//aggiunge tutti i file del folder tranne quelli presenti nella lista degli excludeFolder

		void			end();


		static bool		decompresAll (const u8* const utf8_fullSRCFileNameAndPath, const u8* const pathDestNoSlash);
						/* prende un arhivio generato da begin() add..() end() e lo decomprime interamente nella
							directory di destinazione
						*/

	private:
		FILE		*fHeader;
		FILE		*fData;
		u8			*tempBuffer;
		u32			sizeOfTempBuffer;
		Allocator	*localAllocator;
		u8			compressionLevel;
		u8			*utf8_fullDstFileNameAndPath;
		u32			numFiles;
		rhea::Array<rhea::utf8::String> excludedFolderList;
	};

} //namespace rhea
#endif	//#define _rheaCompress_h_
