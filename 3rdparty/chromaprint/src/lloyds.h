#ifndef CHROMAPRINT_EXT_LLOYDS_H_
#define CHROMAPRINT_EXT_LLOYDS_H_

#include <vector>

std::vector<double> lloyds(std::vector<double> &sig, int len);

template<class Iterator>
std::vector<double> lloyds(Iterator first, Iterator last, int len)
{
	std::vector<double> sig(first, last);
	return lloyds(sig, len);
}

#endif
