/*
 *  dmgfile.h
 *  libdmg-hfsplus
 *
 */

#include <dmg/dmg.h>

io_func* openDmgFile(AbstractFile* dmg);
io_func* openDmgFilePartition(AbstractFile* dmg, int partition);

typedef struct DMG {
	AbstractFile* dmg;
	ResourceKey* resources;
	uint32_t numBLKX;
	BLKXTable** blkx;
	void* runData;
	uint64_t runStart;
	uint64_t runEnd;
	uint64_t offset;
} DMG;
