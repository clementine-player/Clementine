#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
//#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fileref.h>
#include <tag.h>
#include "chromaprint.h"
#include "fingerprinter.h"
#include "fingerprinter_configuration.h"
#include "fingerprint_compressor.h"
#include "base64.h"
#include "ext/ffmpeg_decoder.h"

using namespace std;

static const int kChromaprintAlgorithm = CHROMAPRINT_ALGORITHM_DEFAULT;

typedef vector<string> string_vector;

void FindFiles(const string &dirname, string_vector *result, time_t changed_since)
{
	DIR *dirp = opendir(dirname.c_str());
	if (!dirp) {
		if (errno == ENOTDIR) {
			result->push_back(dirname);
		}
		return;
	}
	while (dirp) {
		struct dirent *dp;
		if ((dp = readdir(dirp)) != NULL) {
			struct stat sp;
			string filename = dirname + '/' + string(dp->d_name); 
			stat(filename.c_str(), &sp);
			if (S_ISREG(sp.st_mode)) {
				//cerr << "file " << filename << " mtime=" << sp.st_mtime << " ch=" << changed_since << "\n";
				if (!changed_since || sp.st_mtime >= changed_since) {
					result->push_back(filename);
				}
			}
			if (S_ISDIR(sp.st_mode) && dp->d_name[0] != '.') {
				FindFiles(filename, result, changed_since);
			}
		}
		else {
			break;
		}
    }
    closedir(dirp);
}

string_vector FindFiles(const string_vector &files, time_t changed_since)
{
	string_vector result;
	for (size_t i = 0; i < files.size(); i++) {
		FindFiles(files[i], &result, changed_since);
	}
	sort(result.begin(), result.end());
	return result;
}

#define DISPATCH_TAGLIB_FILE(type, file) \
	{ \
		type *tmp = dynamic_cast<type *>(file); \
		if (tmp) { \
			return ExtractMBIDFromFile(tmp); \
		} \
	}

#include <xiphcomment.h>
#include <apetag.h>
#include <vorbisfile.h>
#include <oggflacfile.h>
#include <speexfile.h>
#include <flacfile.h>
#include <mpcfile.h>
#include <wavpackfile.h>
#ifdef TAGLIB_WITH_ASF
#include <asffile.h>
#endif
#ifdef TAGLIB_WITH_MP4
#include <mp4file.h>
#endif
#include <mpegfile.h>
#include <id3v2tag.h>
#include <uniquefileidentifierframe.h>

string ExtractMBIDFromXiphComment(TagLib::Ogg::XiphComment *tag)
{
	string key = "MUSICBRAINZ_TRACKID"; 
	if (tag && tag->fieldListMap().contains(key)) {
		return tag->fieldListMap()[key].front().to8Bit(true);
	}
	return string();
}

string ExtractMBIDFromAPETag(TagLib::APE::Tag *tag)
{
	string key = "MUSICBRAINZ_TRACKID";
	if (tag && tag->itemListMap().contains(key)) {
		return tag->itemListMap()[key].toString().to8Bit(true);
	}
	return string();
}

string ExtractMBIDFromFile(TagLib::Ogg::Vorbis::File *file)
{
	return ExtractMBIDFromXiphComment(file->tag());
}

string ExtractMBIDFromFile(TagLib::Ogg::FLAC::File *file)
{
	return ExtractMBIDFromXiphComment(file->tag());
}

string ExtractMBIDFromFile(TagLib::Ogg::Speex::File *file)
{
	return ExtractMBIDFromXiphComment(file->tag());
}

string ExtractMBIDFromFile(TagLib::FLAC::File *file)
{
	return ExtractMBIDFromXiphComment(file->xiphComment());
}

string ExtractMBIDFromFile(TagLib::MPC::File *file)
{
	return ExtractMBIDFromAPETag(file->APETag());
}

string ExtractMBIDFromFile(TagLib::WavPack::File *file)
{
	return ExtractMBIDFromAPETag(file->APETag());
}

/*string ExtractMBIDFromFile(TagLib::APE::File *file)
{
	return ExtractMBIDFromAPETag(file->APETag());
}*/

#ifdef TAGLIB_WITH_ASF
string ExtractMBIDFromFile(TagLib::ASF::File *file)
{
	string key = "MusicBrainz/Track Id";
	TagLib::ASF::Tag *tag = file->tag();
	if (tag && tag->attributeListMap().contains(key)) {
		return tag->attributeListMap()[key].front().toString().to8Bit(true);
	}
	return string();
}
#endif

#ifdef TAGLIB_WITH_MP4
string ExtractMBIDFromFile(TagLib::MP4::File *file)
{
	string key = "----:com.apple.iTunes:MusicBrainz Track Id";
	TagLib::MP4::Tag *tag = file->tag();
	if (tag && tag->itemListMap().contains(key)) {
		return tag->itemListMap()[key].toStringList().toString().to8Bit(true);
	}
	return string();
}
#endif

string ExtractMBIDFromFile(TagLib::MPEG::File *file)
{
	TagLib::ID3v2::Tag *tag = file->ID3v2Tag();
	if (!tag) {
		return string();
	}
	TagLib::ID3v2::FrameList ufid = tag->frameListMap()["UFID"];
	if (!ufid.isEmpty()) {
		for (TagLib::ID3v2::FrameList::Iterator i = ufid.begin(); i != ufid.end(); i++) {
			TagLib::ID3v2::UniqueFileIdentifierFrame *frame = dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame *>(*i);
			if (frame && frame->owner() == "http://musicbrainz.org") {
				TagLib::ByteVector id = frame->identifier();
				return string(id.data(), id.size());
			}
		}
	}
	return string();
}

string ExtractMusicBrainzTrackID(TagLib::File *file)
{
	DISPATCH_TAGLIB_FILE(TagLib::FLAC::File, file);
	DISPATCH_TAGLIB_FILE(TagLib::Ogg::Vorbis::File, file);
	DISPATCH_TAGLIB_FILE(TagLib::Ogg::FLAC::File, file);
	DISPATCH_TAGLIB_FILE(TagLib::Ogg::Speex::File, file);
	DISPATCH_TAGLIB_FILE(TagLib::MPC::File, file);
	DISPATCH_TAGLIB_FILE(TagLib::WavPack::File, file);
#ifdef TAGLIB_WITH_ASF
	DISPATCH_TAGLIB_FILE(TagLib::ASF::File, file);
#endif
#ifdef TAGLIB_WITH_MP4
	DISPATCH_TAGLIB_FILE(TagLib::MP4::File, file);
#endif
	DISPATCH_TAGLIB_FILE(TagLib::MPEG::File, file);
	return string();
}

bool ReadTags(const string &filename, bool ignore_missing_mbid)
{
	TagLib::FileRef file(filename.c_str(), true);
	if (file.isNull())
		return false;
	TagLib::Tag *tags = file.tag();	
	TagLib::AudioProperties *props = file.audioProperties();
	if (!tags || !props)
		return false;
	//cout << "ARTIST=" << tags->artist().to8Bit(true) << "\n";
	//cout << "TITLE=" << tags->title().to8Bit(true) << "\n";
	//cout << "ALBUM=" << tags->album().to8Bit(true) << "\n";
	int length = props->length();
	if (!length)
		return false;
	string mbid = ExtractMusicBrainzTrackID(file.file());
	if (mbid.size() != 36 && !ignore_missing_mbid)
		return false;
	if (mbid.size() == 36)
		cout << "MBID=" << mbid << "\n";
	cout << "LENGTH=" << length << "\n";
	cout << "BITRATE=" << props->bitrate() << "\n";
	return true;
}

string ExtractExtension(const string &filename)
{
	size_t pos = filename.find_last_of('.');
	if (pos == string::npos) {
		return string();
	}
	return boost::to_upper_copy(filename.substr(pos + 1));
}

string EncodeFingerprint(const vector<uint32_t> &fp)
{
	string res;
	res.resize(fp.size());
	return res;
}

bool ProcessFile(Chromaprint::Fingerprinter *fingerprinter, const string &filename, bool ignore_missing_mbid, int audio_length)
{
	if (!ReadTags(filename, ignore_missing_mbid))
		return false;
	Decoder decoder(filename);
	if (!decoder.Open())
		return false;
	if (!fingerprinter->Start(decoder.SampleRate(), decoder.Channels()))
		return false;
	cerr << filename << "\n";
	cout << "FILENAME=" << filename << "\n";
	cout << "FORMAT=" << ExtractExtension(filename) << "\n";
	decoder.Decode(fingerprinter, audio_length);
	vector<int32_t> fp = fingerprinter->Finish();
	/*cout << "FINGERPRINT1=";
	for (int i = 0; i < fp.size(); i++) {
		cout << fp[i] << ", ";
	}
	cout << "\n";*/
	cout << "FINGERPRINT=" << Chromaprint::Base64Encode(Chromaprint::CompressFingerprint(fp, kChromaprintAlgorithm)) << "\n\n";
	return true;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " [OPTIONS] FILE...\n";
		cerr << "Options:\n";
		cerr << " -nombid          Do not require a MBID embedded in the file\n";
		cerr << " -since DATE      Process only files modified since the given date\n";
		cerr << " -length SECONDS  Length of the audio data used for fingerprinting (default 120)\n";
		return 1;
	}

	string_vector files;
	char *changed_since_str = NULL;
	int audio_length = 120;
	bool ignore_missing_mbid = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-nombid") == 0) {
			ignore_missing_mbid = true;
		}
		else if (strcmp(argv[i], "-since") == 0 && i + 1 < argc) {
			changed_since_str = argv[++i];
		}
		else if (strcmp(argv[i], "-length") == 0 && i + 1 < argc) {
			audio_length = atoi(argv[++i]);
		}
		else {
			files.push_back(argv[i]);
		}
	}

	time_t changed_since = 0;
	if (changed_since_str) {
		struct tm tm;
		memset(&tm, 0, sizeof(tm));
		if (strptime(changed_since_str, "%Y-%m-%d %H:%M", &tm) == NULL) {
			if (strptime(changed_since_str, "%Y-%m-%d", &tm) == NULL) {
				cerr << "ERROR: Invalid date, the expected format is 'YYYY-MM-DD' or 'YYYY-MM-DD HH:MM'\n";
				return 1;
			}
		}
		tm.tm_isdst = -1;
		changed_since = mktime(&tm);
		//cerr << "Calculating fingerprints for files in " << files << " that were changed since " << changed_since_str << "\n";
	}
	else {
		//cerr << "Calculating fingerprints for all files in " << files << "\n";
	}

	Chromaprint::Fingerprinter fingerprinter(Chromaprint::CreateFingerprinterConfiguration(kChromaprintAlgorithm));
	files = FindFiles(files, changed_since);
	for (string_vector::iterator it = files.begin(); it != files.end(); it++) {
		ProcessFile(&fingerprinter, *it, ignore_missing_mbid, audio_length);
	}

	return 0;
}

