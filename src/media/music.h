#ifndef MUSIC_H
#define MUSIC_H

#include <string>
#include <vector>
#include <map>
#include <inttypes.h>



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


struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;
struct AVPacket;
struct AVFrame;

class Music {
private:
	static void loop();


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

	AVFormatContext	*formatCtx = nullptr;
	AVCodecContext *codecCtx = nullptr;
	SwrContext *auConvertCtx = nullptr;

	AVPacket *packet;
	AVFrame *frame;

	uint8_t *tmpBuffer;
	uint8_t *buffer;


	Music(const std::string &url, Channel *channel, int fadeIn, const std::string &fileName, size_t numLine, const std::string &place);
	~Music();

	bool initCodec();
	void update();
	void loadNextParts(size_t count);


public:
	static void init();
	static void clear();

	static void registerChannel(const std::string &name, const std::string &mixer, bool loop,
								const std::string &fileName, int numLine);
	static bool hasChannel(const std::string &name);

	static void setVolume(double volume, const std::string &channelName,
						  const std::string &fileName, int numLine);
	static void setMixerVolume(double volume, const std::string &mixer,
	                           const std::string &fileName, int numLine);

	static void play(const std::string &desc,
	                 const std::string &fileName, uint32_t numLine);
	static void stop(const std::string &desc,
	                 const std::string &fileName, uint32_t numLine);

	static const std::vector<Channel*>& getChannels();
	static const std::vector<Music*>&   getMusics();
	static const std::map<std::string, double>& getMixerVolumes();


	uint8_t *audioPos = nullptr;
	uint32_t audioLen = 0;


	int getVolume() const;
	bool isEnded() const;

	const Channel* getChannel() const;
	const std::string& getUrl() const;
	const std::string& getFileName() const;
	size_t getNumLine() const;

	int getFadeIn() const;
	int getFadeOut() const;
	int64_t getPos() const;

	void setFadeIn(int v);
	void setFadeOut(int v);
	void setPos(int64_t pos);
};

#endif // MUSIC_H
