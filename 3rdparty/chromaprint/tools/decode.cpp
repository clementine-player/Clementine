#include <string>
#include <iostream>

using namespace std;

#include "ext/ffmpeg_decoder.h"
#include "ext/audio_dumper.h"

int main(int argc, char **argv)
{
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " FILENAME\n";
		return 1;
	}

	string file_name(argv[1]);
	Decoder decoder(file_name);
	if (!decoder.Open()) {
		cerr << "ERROR: " << decoder.LastError() << "\n";
		return 2;
	}

	AudioDumper dumper(argv[2]);
	decoder.Decode(&dumper);

	return 0;
}

