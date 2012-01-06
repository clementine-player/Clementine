#include <vector>
#include <algorithm>
#include <stdint.h>

/* fingerprint matcher settings */
#define ACOUSTID_MAX_BIT_ERROR 2
#define ACOUSTID_MAX_ALIGN_OFFSET 120

#define BITCOUNT(x)  __builtin_popcount(x)

inline float
match_fingerprints(const std::vector<int32_t> &a, const std::vector<int32_t> &b)
{
	int i, j, topcount;
	int maxsize = std::max(a.size(), b.size());
	int numcounts = maxsize * 2 + 1;
	int *counts = (int*)malloc(sizeof(int) * numcounts);

	memset(counts, 0, sizeof(int) * numcounts);
	for (i = 0; i < a.size(); i++) {
		int jbegin = std::max(0, i - ACOUSTID_MAX_ALIGN_OFFSET);
		int jend = std::min(b.size(), size_t(i + ACOUSTID_MAX_ALIGN_OFFSET));
		for (j = jbegin; j < jend; j++) {
			int biterror = BITCOUNT(a[i] ^ b[j]);
			if (biterror <= ACOUSTID_MAX_BIT_ERROR) {
				int offset = i - j + maxsize;
				counts[offset]++;			
			}
		}
	}

	topcount = 0;
	for (i = 0; i < numcounts; i++) {
		if (counts[i] > topcount) {
			topcount = counts[i];
		}
	}

	free(counts);

	return (float)topcount / std::min(a.size(), b.size());
}
