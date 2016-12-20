#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include <dmg/dmg.h>
#include <dmg/adc.h>
#include <inttypes.h>

#define SECTORS_AT_A_TIME 0x200

BLKXTable* insertBLKX(AbstractFile* out, AbstractFile* in, uint32_t firstSectorNumber, uint32_t numSectors, uint32_t blocksDescriptor,
			uint32_t checksumType, ChecksumFunc uncompressedChk, void* uncompressedChkToken, ChecksumFunc compressedChk,
			void* compressedChkToken, Volume* volume) {
	BLKXTable* blkx;

	uint32_t roomForRuns;
	uint32_t curRun;
	uint64_t curSector;

	unsigned char* inBuffer;
	unsigned char* outBuffer;
	size_t bufferSize;
	size_t have;
	int ret;

	z_stream strm;


	blkx = (BLKXTable*) malloc(sizeof(BLKXTable) + (2 * sizeof(BLKXRun)));
	roomForRuns = 2;
	memset(blkx, 0, sizeof(BLKXTable) + (roomForRuns * sizeof(BLKXRun)));

	blkx->fUDIFBlocksSignature = UDIF_BLOCK_SIGNATURE;
	blkx->infoVersion = 1;
	blkx->firstSectorNumber = firstSectorNumber;
	blkx->sectorCount = numSectors;
	blkx->dataStart = 0;
	blkx->decompressBufferRequested = 0x208;
	blkx->blocksDescriptor = blocksDescriptor;
	blkx->reserved1 = 0;
	blkx->reserved2 = 0;
	blkx->reserved3 = 0;
	blkx->reserved4 = 0;
	blkx->reserved5 = 0;
	blkx->reserved6 = 0;
	memset(&(blkx->checksum), 0, sizeof(blkx->checksum));
	blkx->checksum.type = checksumType;
	blkx->checksum.size = 0x20;
	blkx->blocksRunCount = 0;

	bufferSize = SECTOR_SIZE * blkx->decompressBufferRequested;

	ASSERT(inBuffer = (unsigned char*) malloc(bufferSize), "malloc");
	ASSERT(outBuffer = (unsigned char*) malloc(bufferSize), "malloc");

	curRun = 0;
	curSector = 0;

	while(numSectors > 0) {
		if(curRun >= roomForRuns) {
			roomForRuns <<= 1;
			blkx = (BLKXTable*) realloc(blkx, sizeof(BLKXTable) + (roomForRuns * sizeof(BLKXRun)));
		}

		blkx->runs[curRun].type = BLOCK_ZLIB;
		blkx->runs[curRun].reserved = 0;
		blkx->runs[curRun].sectorStart = curSector;
		blkx->runs[curRun].sectorCount = (numSectors > SECTORS_AT_A_TIME) ? SECTORS_AT_A_TIME : numSectors;

		memset(&strm, 0, sizeof(strm));
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;

		printf("run %d: sectors=%" PRId64 ", left=%d\n", curRun, blkx->runs[curRun].sectorCount, numSectors);

		ASSERT(deflateInit(&strm, Z_DEFAULT_COMPRESSION) == Z_OK, "deflateInit");

		ASSERT((strm.avail_in = in->read(in, inBuffer, blkx->runs[curRun].sectorCount * SECTOR_SIZE)) == (blkx->runs[curRun].sectorCount * SECTOR_SIZE), "mRead");
		strm.next_in = inBuffer;

		if(uncompressedChk)
			(*uncompressedChk)(uncompressedChkToken, inBuffer, blkx->runs[curRun].sectorCount * SECTOR_SIZE);

		blkx->runs[curRun].compOffset = out->tell(out) - blkx->dataStart;
		blkx->runs[curRun].compLength = 0;

		strm.avail_out = bufferSize;
		strm.next_out = outBuffer;

		ASSERT((ret = deflate(&strm, Z_FINISH)) != Z_STREAM_ERROR, "deflate/Z_STREAM_ERROR");
		if(ret != Z_STREAM_END) {
			ASSERT(FALSE, "deflate");
		}
		have = bufferSize - strm.avail_out;

		if((have / SECTOR_SIZE) > blkx->runs[curRun].sectorCount) {
			blkx->runs[curRun].type = BLOCK_RAW;
			ASSERT(out->write(out, inBuffer, blkx->runs[curRun].sectorCount * SECTOR_SIZE) == (blkx->runs[curRun].sectorCount * SECTOR_SIZE), "fwrite");
			blkx->runs[curRun].compLength += blkx->runs[curRun].sectorCount * SECTOR_SIZE;

			if(compressedChk)
				(*compressedChk)(compressedChkToken, inBuffer, blkx->runs[curRun].sectorCount * SECTOR_SIZE);

		} else {
			ASSERT(out->write(out, outBuffer, have) == have, "fwrite");

			if(compressedChk)
				(*compressedChk)(compressedChkToken, outBuffer, have);

			blkx->runs[curRun].compLength += have;
		}

		deflateEnd(&strm);

		curSector += blkx->runs[curRun].sectorCount;
		numSectors -= blkx->runs[curRun].sectorCount;
		curRun++;
	}

	if(curRun >= roomForRuns) {
		roomForRuns <<= 1;
		blkx = (BLKXTable*) realloc(blkx, sizeof(BLKXTable) + (roomForRuns * sizeof(BLKXRun)));
	}

	blkx->runs[curRun].type = BLOCK_TERMINATOR;
	blkx->runs[curRun].reserved = 0;
	blkx->runs[curRun].sectorStart = curSector;
	blkx->runs[curRun].sectorCount = 0;
	blkx->runs[curRun].compOffset = out->tell(out) - blkx->dataStart;
	blkx->runs[curRun].compLength = 0;
	blkx->blocksRunCount = curRun + 1;

	free(inBuffer);
	free(outBuffer);

	return blkx;
}

#define DEFAULT_BUFFER_SIZE (1 * 1024 * 1024)

void extractBLKX(AbstractFile* in, AbstractFile* out, BLKXTable* blkx) {
	unsigned char* inBuffer;
	unsigned char* outBuffer;
	unsigned char zero;
	size_t bufferSize;
	size_t have;
	off_t initialOffset;
	int i;
	int ret;

	z_stream strm;

	bufferSize = SECTOR_SIZE * blkx->decompressBufferRequested;

	ASSERT(inBuffer = (unsigned char*) malloc(bufferSize), "malloc");
	ASSERT(outBuffer = (unsigned char*) malloc(bufferSize), "malloc");

	initialOffset =	out->tell(out);
	ASSERT(initialOffset != -1, "ftello");

	zero = 0;

	for(i = 0; i < blkx->blocksRunCount; i++) {
		ASSERT(in->seek(in, blkx->dataStart + blkx->runs[i].compOffset) == 0, "fseeko");
		ASSERT(out->seek(out, initialOffset + (blkx->runs[i].sectorStart * SECTOR_SIZE)) == 0, "mSeek");

		if(blkx->runs[i].sectorCount > 0) {
			ASSERT(out->seek(out, initialOffset + (blkx->runs[i].sectorStart + blkx->runs[i].sectorCount) * SECTOR_SIZE - 1) == 0, "mSeek");
			ASSERT(out->write(out, &zero, 1) == 1, "mWrite");
			ASSERT(out->seek(out, initialOffset + (blkx->runs[i].sectorStart * SECTOR_SIZE)) == 0, "mSeek");
		}

		if(blkx->runs[i].type == BLOCK_TERMINATOR) {
			break;
		}

		if( blkx->runs[i].compLength == 0) {
			continue;
		}

		printf("run %d: start=%" PRId64 " sectors=%" PRId64 ", length=%" PRId64 ", fileOffset=0x%" PRIx64 "\n", i, initialOffset + (blkx->runs[i].sectorStart * SECTOR_SIZE), blkx->runs[i].sectorCount, blkx->runs[i].compLength, blkx->runs[i].compOffset);

		switch(blkx->runs[i].type) {
		        case BLOCK_ADC:
                        {
                                size_t bufferRead = 0;
				do {
					ASSERT((strm.avail_in = in->read(in, inBuffer, blkx->runs[i].compLength)) == blkx->runs[i].compLength, "fread");
					strm.avail_out = adc_decompress(strm.avail_in, inBuffer, bufferSize, outBuffer, &have);
					ASSERT(out->write(out, outBuffer, have) == have, "mWrite");
					bufferRead+=strm.avail_out;
				} while (bufferRead < blkx->runs[i].compLength);
                                break;
                        }

			case BLOCK_ZLIB:
				strm.zalloc = Z_NULL;
				strm.zfree = Z_NULL;
				strm.opaque = Z_NULL;
				strm.avail_in = 0;
				strm.next_in = Z_NULL;

				ASSERT(inflateInit(&strm) == Z_OK, "inflateInit");

				ASSERT((strm.avail_in = in->read(in, inBuffer, blkx->runs[i].compLength)) == blkx->runs[i].compLength, "fread");
				strm.next_in = inBuffer;

				do {
					strm.avail_out = bufferSize;
					strm.next_out = outBuffer;
					ASSERT((ret = inflate(&strm, Z_NO_FLUSH)) != Z_STREAM_ERROR, "inflate/Z_STREAM_ERROR");
					if(ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_STREAM_END) {
						ASSERT(FALSE, "inflate");
					}
					have = bufferSize - strm.avail_out;
					ASSERT(out->write(out, outBuffer, have) == have, "mWrite");
				} while (strm.avail_out == 0);

				ASSERT(inflateEnd(&strm) == Z_OK, "inflateEnd");
				break;
			case BLOCK_RAW:
				if(blkx->runs[i].compLength > bufferSize) {
					uint64_t left = blkx->runs[i].compLength;
					void* pageBuffer = malloc(DEFAULT_BUFFER_SIZE);
					while(left > 0) {
						size_t thisRead;
						if(left > DEFAULT_BUFFER_SIZE) {
							thisRead = DEFAULT_BUFFER_SIZE;
						} else {
							thisRead = left;
						}
						ASSERT((have = in->read(in, pageBuffer, thisRead)) == thisRead, "fread");
						ASSERT(out->write(out, pageBuffer, have) == have, "mWrite");
						left -= have;
					}
					free(pageBuffer);
				} else {
					ASSERT((have = in->read(in, inBuffer, blkx->runs[i].compLength)) == blkx->runs[i].compLength, "fread");
					ASSERT(out->write(out, inBuffer, have) == have, "mWrite");
				}
				break;
			case BLOCK_IGNORE:
				break;
			case BLOCK_COMMENT:
				break;
			case BLOCK_TERMINATOR:
				break;
			default:
				break;
		}
	}

	free(inBuffer);
	free(outBuffer);
}
