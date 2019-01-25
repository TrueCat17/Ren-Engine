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
	static const size_t PART_SIZE = (1 << 10) * 32;//32 Kb
	static const size_t MIN_PART_COUNT = 2;
	static const size_t MAX_PART_COUNT = 5;


	static std::map<std::string, double> mixerVolumes;

	static std::vector<Channel*> channels;
	static std::vector<Music*> musics;


	static std::mutex globalMutex;

	static bool needToClear;
	static int startUpdateTime;
	static void realClear();

	static void fillAudio(void *, Uint8 *stream, int globalLen);


	std::string url;
	Channel *channel = nullptr;

	bool ended = false;

	int startFadeInTime = 0;
	int startFadeOutTime = 0;

	int fadeIn = 0;
	int fadeOut = 0;
	int64_t lastFramePts = 0;

	std::string fileName;
	size_t numLine;
	std::string place;//place = fileName & numLine

	int audioStream = -1;

	uint8_t *tmpBuffer = (uint8_t *)av_malloc(PART_SIZE);
	uint8_t *buffer = (uint8_t *)av_malloc(PART_SIZE * (MAX_PART_COUNT + 1));
	Uint8 *audioPos = nullptr;
	Uint32 audioLen = 0;

	AVFormatContext	*formatCtx = nullptr;
	AVCodecContext *codecCtx = nullptr;
	SwrContext *auConvertCtx = nullptr;

	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	AVFrame *frame = av_frame_alloc();


	Music(const std::string &url, Channel *channel, int fadeIn, const std::string &fileName, size_t numLine, const std::string &place);
	~Music();

	bool initCodec();
	void update();
	void loadNextParts(size_t count);

	int getVolume() const;
	bool isEnded() const;


public:
	static void init();
	static void clear();

	static void registerChannel(const std::string &name, const std::string &mixer, bool loop,
								const std::string &fileName, int numLine);
	static bool hasChannel(const std::string &name);

	static void setVolume(double volume, const std::string &channelName,
						  const std::string &fileName, int numLine);
	static void setMixerVolume(double volume, const std::string &mixer,
							   const std::string& fileName, int numLine);

	static void play(const std::string &desc,
					   const std::string& fileName, size_t numLine);
	static void stop(const std::string &desc,
					 const std::string& fileName, size_t numLine);

	static const std::vector<Channel*>& getChannels() { return channels; }
	static const std::vector<Music*>&   getMusics()   { return musics; }
	static const std::map<std::string, double>& getMixerVolumes() { return mixerVolumes; }


	const Channel* getChannel() const { return channel; }
	const std::string& getUrl() const { return url; }
	const std::string& getFileName() const { return fileName; }
	int getNumLine() const { return numLine; }

	int getFadeIn() const { return std::max(fadeIn + startFadeInTime - startUpdateTime, 0); }
	int getFadeOut() const { return std::max(fadeOut + startFadeOutTime - startUpdateTime, 0); }
	int64_t getPos() const { return lastFramePts; }

	void setFadeIn(int v) { fadeIn = v; startFadeInTime = startUpdateTime; }
	void setFadeOut(int v) { fadeOut = v; startFadeOutTime = startUpdateTime; }
	void setPos(int64_t pos);
};

#endif // MUSIC_H
