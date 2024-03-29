#ifndef AUDIO_H
#define AUDIO_H

#ifndef ALLOW_INCLUDE_AUDIO_H
#error "Use AudioManager, not Audio"
#endif

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}


const size_t PART_SIZE = (1 << 10) * 32;//32 Kb
const size_t MIN_PART_COUNT = 2;
const size_t MAX_PART_COUNT = 5;

const size_t SAMPLE_SIZE = 2 * 2;//stereo (2 channels), 16 bits (2 bytes)
const size_t SAMPLES_PER_PART = PART_SIZE / SAMPLE_SIZE;


struct Channel {
	std::string name;
	std::string mixer;
	bool loop;
	double volume;

	Channel(const std::string& name, const std::string &mixer, bool loop):
	    name(name),
	    mixer(mixer),
	    loop(loop),
	    volume(1)
	{}
};

struct Audio {
	std::string url;
	Channel *channel = nullptr;

	double startFadeInTime = 0;
	double startFadeOutTime = 0;

	double fadeIn = 0;
	double fadeOut = 0;
	double relativeVolume;
	double curTime = -1;

	std::string place;//place = fileName & numLine
	std::string fileName;
	uint32_t numLine;

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

	bool paused = false;

	bool decoded = false;


	Audio(const std::string &url, Channel *channel, double fadeIn, double volume,
	      const std::string &place, const std::string &fileName, uint32_t numLine);
	~Audio();

	bool initCodec();
	void update();
	void loadNextPart();

	int getVolume() const;
	bool isEnded() const;

	double getFadeIn() const;
	double getFadeOut() const;
	double getPos() const;

	void setFadeIn(double v);
	void setFadeOut(double v);
	void setPos(double sec);

	void addToCurTime(size_t playedBytesCount);
};

#endif // AUDIO_H
