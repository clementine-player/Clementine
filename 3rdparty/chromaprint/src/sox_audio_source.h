#ifndef FP_SOX_AUDIO_SOURCE_H_
#define FP_SOX_AUDIO_SOURCE_H_

extern "C" {
#include <sox.h>
}
#include <string>

class SoxAudioSource {
public:
	SoxAudioSource(const std::string &file_name);
	virtual ~SoxAudioSource();

	bool Open();
	void Close();

	virtual int SampleRate();
	virtual int Channels();
	virtual std::size_t Length() { return format_->signal.length; }
	virtual std::size_t Read(short *buffer, std::size_t size);

private:
	static bool initialized_;
	std::string file_name_;
	sox_format_t *format_;
	sox_sample_t *buffer_;
};

#endif
