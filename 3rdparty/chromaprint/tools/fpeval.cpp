#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/multi_array.hpp>
#include "ext/ffmpeg_decoder.h"
#include "ext/image_utils.h"
#include "audio_processor.h"
#include "fingerprinter.h"
#include "image.h"
#include "integral_image.h"
#include "image_builder.h"
#include "utils.h"
#include "filter.h"
#include "lloyds.h"
#include "classifier.h"
#include "match.h"

using namespace std;
using namespace Chromaprint;
namespace fs = boost::filesystem;

typedef vector<string> string_vector;
typedef vector<double> double_vector;

string_vector FindAudioFiles(const char *dirname)
{
	string_vector result;
	fs::path path(dirname);
	fs::directory_iterator end_iter;
	for (fs::directory_iterator dir_iter(path); dir_iter != end_iter; ++dir_iter) {
		if (fs::is_regular_file(dir_iter->status())) {
			string filename = dir_iter->path().string(); 
			if (boost::ends_with(filename, ".mp3") || boost::ends_with(filename, ".wav")) {
				result.push_back(filename);
			}
		}
	}
	sort(result.begin(), result.end());
	return result;
}

vector<string_vector> GenerateFilePairs(const vector<string> &files)
{
	string last_name;
	vector<string_vector> result;
	for (int i = 0; i < files.size(); i++) {
		string name = fs::basename(files[i]); 
		name = name.substr(0, name.find_first_of('-'));
		if (last_name != name) {
			result.push_back(string_vector());
			last_name = name;
		}
		result.back().push_back(files[i]);
	}
	return result;
}

vector<int> MultiMapValues(const multimap<int, int> &data, const int &key)
{
	pair<multimap<int, int>::const_iterator, multimap<int, int>::const_iterator> range = data.equal_range(key);
	vector<int> result;
	for (multimap<int, int>::const_iterator i = range.first; i != range.second; ++i) {
		result.push_back((*i).second);
	}
	return result;
}

static const int SAMPLE_RATE = 11025;
static const int FRAME_SIZE = 4096;
static const int OVERLAP = FRAME_SIZE - FRAME_SIZE / 3;// 2720;
static const int MIN_FREQ = 28;
static const int MAX_FREQ = 3520;
static const int MAX_FILTER_WIDTH = 16;
/*static const int OVERLAP = FRAME_SIZE - FRAME_SIZE / 2;// 2720;
static const int MIN_FREQ = 300;
static const int MAX_FREQ = 5300;
static const int MAX_FILTER_WIDTH = 20;*/

void PrintRate(const std::string &name, const std::vector<int> &values, int scale)
{
	cout << name << " = [";
	for (int j = 0; j < 32; j++) {
		double rate = double(values[j]) / scale;
		if (j != 0) std::cout << ", ";
		std::cout << rate;
	}
	cout << "]\n";
}

#define FP_TYPE_CHROMA		1
#define FP_TYPE_CENTROID	2
#define FP_TYPE FP_TYPE_CHROMA

int main(int argc, char **argv)
{
	Chromaprint::Fingerprinter fingerprinter;

	vector<string> files = FindAudioFiles(argv[1]);
	vector< string > names[2];
	vector< vector<int32_t> > fingerprints[2];
	for (int i = 0; i < files.size(); i++) {
		cout << " - " << files[i] << "\n";
		Decoder decoder(files[i]);
		if (!decoder.Open()) {
			cerr << "ERROR: " << decoder.LastError() << "\n";
			return 1;
		}
		fingerprinter.Start(decoder.SampleRate(), decoder.Channels());
		decoder.Decode(&fingerprinter, 60);
		vector<int32_t> fp = fingerprinter.Finish();
		int orig = files[i].find("orig") != string::npos ? 1 : 0;
		fingerprints[orig].push_back(fp);
		names[orig].push_back(files[i]);
	}
	int num_files = files.size() / 2;

	typedef boost::multi_array<float, 2> DoubleArray2D;

	float total = 0.0f, diagonal = 0.0f;
	DoubleArray2D confmatrix(boost::extents[num_files][num_files]);
	for (int i = 0; i < num_files; i++) {
		for (int j = 0; j < num_files; j++) {
			float score = match_fingerprints(fingerprints[0][i], fingerprints[1][j]);			
			cout << " - " << names[0][i] << " / " << names[1][j] << " = " << score <<"\n";
			confmatrix[i][j] = score;
			if (i == j) {
				diagonal += score;
			}
			total += score;
		}
	}

	cout << "true positive: " << diagonal / num_files << "\n";
	cout << "false positive: " << (total - diagonal) / (num_files * num_files - num_files) << "\n";
	cout << "score: " << diagonal / total << "\n";
	return 0;
}

