#include <algorithm>
#include <vector>
#include "audio_consumer.h"

class AudioBuffer : public Chromaprint::AudioConsumer
{
public:
	void Consume(short *input, int length)
	{
		//cout << "AudioBuffer::Consume(" << length << ")\n";
		int last_size = m_data.size();
		//cout << "got " << input[0] << " at index " << last_size << "\n";
		m_data.resize(last_size + length);
		std::copy(input, input + length, m_data.begin() + last_size);
	}

	const std::vector<short> &data() { return m_data; }

private:
	std::vector<short> m_data;
};
