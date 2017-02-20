#ifndef MUSIC_H
#define MUSIC_H

#include <string>
#include <vector>
#include <map>
#include <mutex>

#include <SDL2/SDL.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}



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


class Music {
private:
	static const size_t PART_SIZE = (1 << 20) / 4;//0.25 Mb
	static const size_t MIN_PART_COUNT = 2;
	static const size_t MAX_PART_COUNT = 7;


	static std::map<std::string, double> mixerVolumes;

	static std::vector<Channel*> channels;
	static std::vector<Music*> musics;


	static std::mutex globalMutex;

	static bool needToClear;
	static int startUpdateTime;
	static void realClear();

	static void fillAudio(void *, Uint8 *stream, int globalLen);


	std::mutex mutex;
	std::string url;
	Channel *channel = nullptr;

	bool ended = false;

	int startFadeInTime = 0;
	int startFadeOutTime = 0;

	int fadeIn = 0;
	int fadeOut = 0;

	int audioStream = -1;

	uint8_t *tmpBuffer = (uint8_t *)av_malloc(PART_SIZE);
	uint8_t *buffer = (uint8_t *)av_malloc(PART_SIZE * (MAX_PART_COUNT + 1));
	Uint8 *audioPos = 0;
	Uint32 audioLen = 0;

	AVFormatContext	*formatCtx = nullptr;
	AVCodecContext *codecCtx = nullptr;
	SwrContext *auConvertCtx = nullptr;

	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	AVFrame *frame = av_frame_alloc();


	Music(const std::string &url, Channel *channel, int fadeIn);
	~Music();

	bool initCodec();
	void update();
	void loadNextParts(size_t count);

	int getVolume() const;
	bool isEnded() const;


public:
	static void init();
	static void clear();

	static void registerChannel(const std::string &name, const std::string &mixer, bool loop);
	static void setVolume(double volume, const std::string &channelName);
	static void setMixerVolume(double volume, const std::string &mixer);

	static void play(const std::string &desc);
	static void stop(const std::string &desc);
};

#endif // MUSIC_H
