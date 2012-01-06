#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/dynamic_bitset.hpp>
#include "ext/ffmpeg_decoder.h"
#include "ext/image_utils.h"
#include "audio_processor.h"
#include "chroma.h"
#include "spectral_centroid.h"
#include "chroma_normalizer.h"
#include "chroma_resampler.h"
#include "chroma_filter.h"
#include "fft.h"
#include "audio_processor.h"
#include "image.h"
#include "integral_image.h"
#include "image_builder.h"
#include "utils.h"
#include "filter.h"
#include "lloyds.h"
#include "classifier.h"

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
			if (boost::ends_with(filename, ".mp3") || boost::ends_with(filename, ".wma") || boost::ends_with(filename, ".wav")) {
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

static const int TRAINING_SET_SIZE = 60000;

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
	Chromaprint::ImageBuilder image_builder;
#if FP_TYPE == FP_TYPE_CHROMA
	Chromaprint::ChromaNormalizer chroma_normalizer(&image_builder);
	static const double kChromaFilterCoeffs[] = { 0.25, 0.75, 1.0, 0.75, 0.25 };
	Chromaprint::ChromaFilter chroma_filter(kChromaFilterCoeffs, 5, &chroma_normalizer);
	Chromaprint::Chroma chroma(MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, &chroma_filter);
	//chroma.set_interpolate(true);
	Chromaprint::FFT fft(FRAME_SIZE, OVERLAP, &chroma);
#elif FP_TYPE == FP_TYPE_CHROMA
	Chromaprint::SpectralCentroid centroid(16, MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, &image_builder);
	Chromaprint::FFT fft(FRAME_SIZE, OVERLAP, &centroid);
#endif
	Chromaprint::AudioProcessor processor(SAMPLE_RATE, &fft);

	cout << "Loading audio files\n";
	vector<string_vector> files = GenerateFilePairs(FindAudioFiles(argv[1]));
	vector<int> groups;
	multimap<int, int> reverse_groups;
	vector<Chromaprint::Image *> images;
	vector<Chromaprint::IntegralImage *> integral_images;
	for (int i = 0; i < files.size(); i++) {
		//cout << i << ".\n";
		for (int j = 0; j < files[i].size(); j++) {
			cout << " - " << files[i][j] << "\n";
			Decoder decoder(files[i][j]);
			if (!decoder.Open()) {
				cerr << "ERROR: " << decoder.LastError() << "\n";
				return 1;
			}
#if FP_TYPE == FP_TYPE_CHROMA
			Image *image = new Image(12);
#elif FP_TYPE == FP_TYPE_CENTROID
			Image *image = new Image(16);
#endif
			processor.Reset(decoder.SampleRate(), decoder.Channels());
			fft.Reset();
#if FP_TYPE == FP_TYPE_CHROMA
			chroma.Reset();
			chroma_filter.Reset();
			chroma_normalizer.Reset();
#elif FP_TYPE == FP_TYPE_CENTROID
			centroid.Reset();
#endif
			image_builder.Reset(image);
			decoder.Decode(&processor);
			processor.Flush();
			//ExportTextImage(image, files[i][j] + ".img.txt");
			//Chromaprint::ExportImage(image_builder.image(), files[i][j] + ".img.png");
			reverse_groups.insert(make_pair(i, images.size()));
			groups.push_back(i);
			images.push_back(image);
			IntegralImage *int_image = new IntegralImage(image) ;
		//	ExportTextImage(int_image, files[i][j] + ".int_img.txt");
			integral_images.push_back(int_image);
			//ExportTextImage(int_image, files[i][j] + ".img.txt");
	//		return 1;
		}
		cout << images.size() << "\r";
		cout.flush();
	}

	cout << "Training data set:\n";
	cout << " - File groups: " << files.size() << "\n";
	cout << " - Files: " << images.size() << "\n";
	files.clear();

	bool labels[TRAINING_SET_SIZE];
	int data1[TRAINING_SET_SIZE];
	int data2[TRAINING_SET_SIZE];
	int data1_pos[TRAINING_SET_SIZE];
	int data2_pos[TRAINING_SET_SIZE];

	int i = 0;
	srand(3);
	// Find matching pairs
	for (; i < TRAINING_SET_SIZE / 2; i++) {
		int x1 = rand() % images.size();
		vector<int> group = MultiMapValues(reverse_groups, groups[x1]);
		int x2;
		do {
			x2 = group[rand() % group.size()];
		} while (x1 == x2);
		size_t min_length = min(images[x1]->NumRows(), images[x2]->NumRows());
		int pos = rand() % (min_length - 30);
		//cout << "+ " << x1 << " " << x2 << " - " << pos << "\n";
		data1[i] = x1;
		data2[i] = x2;
		data1_pos[i] = pos;
		data2_pos[i] = pos;
		labels[i] = true;
	}
	// Find non-matching pairs
	for (; i < TRAINING_SET_SIZE; i++) {
		int x2, x1 = rand() % images.size();
		int pos2, pos1 = rand() % (images[x1]->NumRows() - 30);
		vector<int> group = MultiMapValues(reverse_groups, groups[x1]);
		set<int> group_set(group.begin(), group.end());
		do {
			x2 = rand() % images.size();
			pos2 = rand() % (images[x2]->NumRows() - 30);
		} while (group_set.count(x2) && abs(pos1 - pos2) < 50);
		//cout << "- " << x1 << " " << x2 << " - " << pos1 << " " << pos2 << "\n";
		data1[i] = x1;
		data2[i] = x2;
		data1_pos[i] = pos1;
		data2_pos[i] = pos2;
		labels[i] = false;
	}

	const float maxWidth = 16.0;
	const float widthScale = 1.0;
	const float widthIncrement = 1.0;

	const int numFilters = 6;
	const int maxHeight = images[0]->NumColumns();
	const int filterWidthInc[] = { 1, 1, 2, 2, 1, 3 };
	const int filterHeightInc[] = { 1, 2, 1, 2, 3, 1 };
	const int kNumCandidateQuantizers = 24;

	double weights[TRAINING_SET_SIZE];
	for (int i = 0; i < TRAINING_SET_SIZE; i++) {
		weights[i] = 1.0 / TRAINING_SET_SIZE;
	}

	cout << "Computing filter responses\n";
	std::vector<Filter> filters;
	double *values_buffer = new double[100 * TRAINING_SET_SIZE * 2];

	Filter flt;
	FILE *f = fopen("filters.tmp", "wb");
	for (int filter = 0; filter < numFilters; filter++) {
		flt.set_type(filter);
		for (int y = 0; y < maxHeight; y++) {
			flt.set_y(y);
			for (int h = filterHeightInc[filter]; h < maxHeight - y; h += filterHeightInc[filter]) {
				flt.set_height(h);
				int pw = 0;
				for (int wf = filterWidthInc[filter]; wf <= maxWidth; wf += filterWidthInc[filter]) {
					int w = wf;
					if (pw == w)
						continue;
					flt.set_width(w);
					fill(values_buffer, values_buffer + TRAINING_SET_SIZE * 2, 0.0);
					for (int i = 0; i < TRAINING_SET_SIZE; i++) {
						double value1 = flt.Apply(integral_images[data1[i]], data1_pos[i]);
						double value2 = flt.Apply(integral_images[data2[i]], data2_pos[i]);
						//cout << flt << " " << value1 << " [" << data1[i] << ":" << data1_pos[i] << "] " << value2 << " [" << data2[i] << ":" << data2_pos[i] << "]\n";
						values_buffer[2*i+0] = value1;
						values_buffer[2*i+1] = value2;
					}
					fwrite(values_buffer, sizeof(double), TRAINING_SET_SIZE * 2, f);
					filters.push_back(flt);
				}
			}
		}
	}
	cout << "Filters: " << filters.size() << "\n";

	fclose(f);
	f = fopen("filters.tmp", "rb");

	cout << "Computing quantizers\n";
	double *all_candidate_quantizers = new double[filters.size() * kNumCandidateQuantizers];
	double *quantizer_ptr = all_candidate_quantizers;
	fseek(f, 0, SEEK_SET);
	double *values_ptr = values_buffer + 100 * TRAINING_SET_SIZE * 2;
	for (int filter_i = 0; filter_i < filters.size(); filter_i++) {
		int num_values = TRAINING_SET_SIZE * 2;
		if (values_ptr >= values_buffer + 100 * TRAINING_SET_SIZE * 2) {
			fread(values_buffer, sizeof(double), 100 * num_values, f);
			values_ptr = values_buffer;
		}
		double_vector candidate_quantizers = lloyds(values_ptr, values_ptr + num_values, kNumCandidateQuantizers);
		copy(candidate_quantizers.begin(), candidate_quantizers.end(), quantizer_ptr);
		quantizer_ptr += kNumCandidateQuantizers;
		values_ptr += num_values;
		cout << filter_i << "\r";
		cout.flush();
	}

	cout << "Running AdaBoost\n";
	vector<Classifier> best_classifiers;
	vector<double> best_classifiers_alpha;

	int iteration = 0;
next_iteration:

	double min_error = 1.0;
	Classifier best_classifier;
	boost::dynamic_bitset<> best_results(TRAINING_SET_SIZE);
	fseek(f, 0, SEEK_SET);
	quantizer_ptr = all_candidate_quantizers;
	values_ptr = values_buffer + 100 * TRAINING_SET_SIZE * 2;
	for (int filter_i = 0; filter_i < filters.size(); filter_i++) {
		std::size_t num_values = TRAINING_SET_SIZE * 2;
		if (values_ptr >= values_buffer + 100 * TRAINING_SET_SIZE * 2) {
			fread(values_buffer, sizeof(double), 100 * num_values, f);
			values_ptr = values_buffer;
		}
	
		Quantizer quantizer(0.0, 0.0, 0.0);
		for (int ti0 = 0; ti0 < kNumCandidateQuantizers - 3; ti0++) {
			quantizer.set_t0(quantizer_ptr[ti0]);
			for (int ti1 = ti0 + 1; ti1 < kNumCandidateQuantizers - 2; ti1++) {
				quantizer.set_t1(quantizer_ptr[ti1]);
				for (int ti2 = ti1 + 1; ti2 < kNumCandidateQuantizers - 1; ti2++) {
					quantizer.set_t2(quantizer_ptr[ti2]);
					double error = 0.0;
					for (int i = 0; i < TRAINING_SET_SIZE; i++) {
						int q1 = quantizer.Quantize(values_ptr[i*2 + 0]);
						int q2 = quantizer.Quantize(values_ptr[i*2 + 1]);
						bool match = (q1 == q2) == labels[i];
						if (!match) {
							error += weights[i];
						}
					}
					if (error < min_error) {
						best_classifier = Classifier(filters[filter_i], quantizer);
						min_error = error;
						for (int i = 0; i < TRAINING_SET_SIZE; i++) {
							int q1 = quantizer.Quantize(values_ptr[i*2 + 0]);
							int q2 = quantizer.Quantize(values_ptr[i*2 + 1]);
							bool match = (q1 == q2) == labels[i];
							best_results[i] = match;
						}
					}
				}
			}
		}
		quantizer_ptr += kNumCandidateQuantizers;
		values_ptr += num_values;
		cout << filter_i << " [" << min_error << "]       \r";
		cout.flush();
	}

	double weight_sum = 0.0;
	double alpha = 0.5 * log((1.0 - min_error) / min_error);
	for (int i = 0; i < TRAINING_SET_SIZE; i++) {
		weights[i] *= exp(-alpha * (best_results[i] ? 1.0 : -1.0));
		weight_sum += weights[i];
	}
	for (int i = 0; i < TRAINING_SET_SIZE; i++) {
		weights[i] /= weight_sum;
	}

	best_classifiers.push_back(best_classifier);
	best_classifiers_alpha.push_back(alpha);

	int wrong = 0;
	int counts[2];
	counts[0] = 0;
	counts[1] = 0;
	std::vector<int> bit_tp(32, 0);
	std::vector<int> bit_fp(32, 0);
	for (int i = 0; i < TRAINING_SET_SIZE; i++) {
		double value = 0.0;
		int bit_error = 0;
		for (int j = 0; j < best_classifiers.size(); j++) {
			int q1 = best_classifiers[j].Classify(integral_images[data1[i]], data1_pos[i]);
			int q2 = best_classifiers[j].Classify(integral_images[data2[i]], data2_pos[i]);
			int e = abs(q1 - q2);
			bit_error += e == 3 ? 1 : e;
			value += best_classifiers_alpha[j] * (q1 == q2 ? 1.0 : -1.0);
		}
		bool match = value > 0.0;
		counts[labels[i]]++;
		if (labels[i] != match) {
			wrong += 1.0;
		}
		for (int j = 0; j < 32; j++) {
			bool bit_match = bit_error <= j;
			if (labels[i] && bit_match) {
				bit_tp[j]++;
			}
			if (!labels[i] && bit_match) {
				bit_fp[j]++;
			}
		}
	}

	++iteration;
	cout << iteration << ". best classifier is " << best_classifier << " with error " << min_error << " (alpha " << alpha << "), final error is " << double(wrong) / TRAINING_SET_SIZE << "\n";
	PrintRate("TP", bit_tp, counts[1]);
	PrintRate("FP", bit_fp, counts[0]);

	if (iteration < 16)
		goto next_iteration;

	return 0;
}

