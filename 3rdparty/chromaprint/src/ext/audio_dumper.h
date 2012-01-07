#ifndef CHROMAPRINT_EXT_AUDIO_DUMPER_H_
#define CHROMAPRINT_EXT_AUDIO_DUMPER_H_

#include <stdio.h>
#include <string>
#include "audio_consumer.h"

class AudioDumper : public Chromaprint::AudioConsumer
{
public:
	AudioDumper(const std::string &file_name)
	{
		m_file = fopen(file_name.c_str(), "wb");
	}

	~AudioDumper()
	{
		fclose(m_file);
	}

	void Consume(short *input, int length)
	{
		fwrite(input, sizeof(short), length, m_file);
	}

private:
	FILE *m_file;
};

#endif
