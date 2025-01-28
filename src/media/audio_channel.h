#ifndef AUDIO_CHANNEL_H
#define AUDIO_CHANNEL_H

#ifndef ALLOW_INCLUDE_AUDIO_CHANNEL_H
#error "Use AudioManager, not AudioChannel"
#endif

#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}



struct Audio {
	std::string path;

	std::string place;//place = fileName & numLine
	std::string fileName;
	uint32_t numLine;

	double volume;
	double fadeIn;
	double fadeOut = -1;

	double startFadeInTime = -1;
	double startFadeOutTime = 0;

	Audio(const std::string &path, double fadeIn, double volume,
	      const std::string &place, const std::string &fileName, uint32_t numLine):
	    path(path),
	    place(place),
	    fileName(fileName),
	    numLine(numLine),
	    volume(volume),
	    fadeIn(fadeIn)
	{}
};



const size_t PART_SIZE = (1 << 10) * 32;//32 Kb
const size_t MIN_PART_COUNT = 2;
const size_t MAX_PART_COUNT = 5;

const size_t SAMPLE_SIZE = 2 * 2;//stereo (2 channels), 16 bits (2 bytes)
const size_t SAMPLES_PER_PART = PART_SIZE / SAMPLE_SIZE;


struct Channel {
private:
	void loadNextFrame();

public:
	std::vector<Audio*> audios;

	std::string name;
	std::string mixer;
	bool loop;
	double volume = 1;

	double curTime = -1;
	bool paused = false;
	bool fullDecoded = false;

	int audioStream = -1;

	AVFormatContext	*formatCtx = nullptr;
	AVCodecContext *codecCtx = nullptr;
	SwrContext *convertCtx = nullptr;

	AVPacket *packet;
	AVFrame *frame;

	uint8_t *tmpBuffer;
	uint8_t *buffer;

	uint8_t *audioPos = nullptr;
	uint32_t audioLen = 0;

	Channel(const std::string& name, const std::string &mixer, bool loop);
	~Channel();

	Audio* getAudio() const;
	void clearQueue(double fadeOut);

	bool initCodec();
	void update(bool minUpdate);

	int getVolume() const;

	void setFadeIn(double v);
	void setFadeOut(double v);

	double getPos() const;
	void   setPos(double sec);

	void addToCurTime(size_t playedBytesCount);
};

#endif // AUDIO_CHANNEL_H
