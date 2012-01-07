#include <algorithm>
#include <limits>
#include "sox_audio_source.h"

using namespace std;

static const size_t kBufferSize = 2 * 4096;
bool SoxAudioSource::initialized_ = false;

SoxAudioSource::SoxAudioSource(const string &file_name)
	: format_(0), file_name_(file_name)
{
	if (!initialized_) {
		sox_init();
		initialized_ = true;
	}
	buffer_ = new sox_sample_t[kBufferSize];
}

SoxAudioSource::~SoxAudioSource()
{
	Close();
	delete[] buffer_;
}

bool SoxAudioSource::Open()
{
	Close();
	format_ = sox_open_read(file_name_.c_str(), NULL, NULL, NULL);
	return format_ != 0;
}

void SoxAudioSource::Close()
{
	if (format_) {
		sox_close(format_);
		format_ = 0;
	}
}

int SoxAudioSource::SampleRate()
{
	return static_cast<int>(format_->signal.rate);
}

int SoxAudioSource::Channels()
{
	return format_->signal.channels;
}

std::size_t SoxAudioSource::Read(short *buffer, std::size_t size)
{
	size_t remaining = size;
	short *ptr = buffer;
	while (remaining > 0) {
		size_t res = sox_read(format_, buffer_, std::min(remaining, kBufferSize));
		if (res == SOX_EOF || res == 0) {
			break;
		}
		int clips = 0;
		for (size_t i = 0; i < res; i++) {
			// XXX assumes that short is 16-bit
			SOX_SAMPLE_LOCALS;
			*ptr++ = SOX_SAMPLE_TO_SIGNED_16BIT(buffer_[i], clips);
		}
		remaining -= res;
	}
	//cout << "read " << size - remaining << "\n";
	return size - remaining;
}

