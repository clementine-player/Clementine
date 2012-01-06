/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2010  Lukas Lalinsky <lalinsky@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef FFMPEG_DECODER_H_
#define FFMPEG_DECODER_H_

#include <algorithm>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include "sox_audio_source.h"
#include "audio_consumer.h"

class Decoder
{
public:
	Decoder(const std::string &file_name);
	~Decoder();

	bool Open();
	void Decode(Chromaprint::AudioConsumer *consumer, int max_length = 0);

	int Channels()
	{
		return m_source->Channels();
	}

	int SampleRate()
	{
		return m_source->SampleRate();
	}

	std::string LastError()
	{
		return "";
	//	return m_error;
	}

private:
	SoxAudioSource *m_source;
	short m_buffer[1024];
};

inline Decoder::Decoder(const std::string &file_name)
{
	m_source = new SoxAudioSource(file_name);
	m_source->Open();
}

inline Decoder::~Decoder()
{
	delete m_source;
}

inline bool Decoder::Open()
{
	return true;
}

#include <stdio.h>

inline void Decoder::Decode(Chromaprint::AudioConsumer *consumer, int max_length)
{
	int remaining = SampleRate() * Channels() * max_length;
	while (true) {
		int res = m_source->Read(m_buffer, size_t(1024));
		if (res <= 0) {
			break;
		}
		if (max_length) {
			res = std::min(res, remaining);
		}
		consumer->Consume(m_buffer, res);
		if (max_length) {
			remaining -= res;
			if (res <= 0) {
				break;
			}
		}
	}
}

#endif
