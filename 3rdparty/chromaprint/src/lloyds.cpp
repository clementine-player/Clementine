#include <math.h>
#include <assert.h>
#include <algorithm>
//#include <iostream>
#include "lloyds.h"
#include <limits>

using namespace std;

template <typename T>
T min(const vector<T> &array)
{
	T m = array.size() ? array[0] : 0;
	for (int i = 1; i < array.size(); i++) {
		if (array[i] < m) {
			m = array[i];
		}
	}
	return m;
}

template <typename T>
T max(const vector<T> &array)
{
	T m = array.size() ? array[0] : 0;
	for (int i = 1; i < array.size(); i++) {
		if (array[i] > m) {
			m = array[i];
		}
	}
	return m;
}

inline double sqr(double a)
{
	return a * a;
}

/*template<class T>
ostream &operator<<(ostream &stream, const vector<T> &vec)
{
	for (int i = 0; i < vec.size(); i++) {
		if (i != 0)
			stream << ", ";
		stream << vec[i];
	}
	return stream;
}*/

vector<double> lloyds(vector<double> &sig, int len)
{
	vector<double> x(len-1);
	vector<double> q(len);

	sort(sig.begin(), sig.end());

	// Set initial endpoints
	double sig_min = sig[0];
	double sig_max = sig[sig.size()-1];

	// Initial parameters
	for (int i = 0; i < len; i++) {
		q[i] = i * (sig_max - sig_min) / (len - 1) + sig_min;
	}
	for (int i = 0; i < len - 1; i++) {
		x[i] = (q[i] + q[i+1]) / 2;
	}

	double reldist = 1.0, dist = 1.0;
	double stop_criteria = max(numeric_limits<double>::epsilon() * fabs(sig_max), 1e-7);
	double iteration = 0;
	while (reldist > stop_criteria) {
		iteration++;
		reldist = dist;
		dist = 0.0;

		size_t sig_it = 0;
		for (int i = 0; i < len; i++) {
			double sum = 0.0;
			int cnt = 0;
			while (sig_it < sig.size() && (i == len - 1 || sig[sig_it] < x[i])) {
				sum += sig[sig_it];
				dist += sqr(sig[sig_it] - q[i]);
				++cnt;
				++sig_it;
			}
			if (cnt) {
				q[i] = sum / cnt;
			}
			else if (i == 0) {
				q[i] = (sig_min + x[i]) / 2.0;
			}
			else if (i == len - 1) {
				q[i] = (x[i-1] + sig_max) / 2.0;
			}
			else {
				q[i] = (x[i-1] + x[i]) / 2.0;
			}
		}
	
		dist /= sig.size();
		reldist = fabs(reldist - dist);
	
		// Set the endpoints in between the updated quanta
		for (int i = 0; i < len - 1; i++) {
			x[i] = (q[i] + q[i+1]) / 2.0;
		}
	}

	return x;
}
