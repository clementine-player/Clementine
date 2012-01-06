#include <string>
#include <iostream>

using namespace std;

#include "ext/ffmpeg_decoder.h"
#include "ext/audio_dumper.h"
#include "audio_processor.h"

int main(int argc, char **argv)
{
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " FILENAME\n";
		return 1;
	}

	string file_name(argv[1]);
	cout << "Loading file " << file_name << "\n";

	Decoder decoder(file_name);
	if (!decoder.Open()) {
		cerr << "ERROR: " << decoder.LastError() << "\n";
		return 2;
	}

	AudioDumper dumper("resampled.raw");
	Chromaprint::AudioProcessor processor(11025, &dumper);
	processor.Reset(decoder.SampleRate(), decoder.Channels());

	decoder.Decode(&processor);
	processor.Flush();

	return 0;
}

