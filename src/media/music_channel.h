#ifndef MUSICCHANNEL_H
#define MUSICCHANNEL_H

#include <vector>

#include <SDL2/SDL_mixer.h>

#include "utils/string.h"


class MusicChannel {
private:
	static std::vector<MusicChannel*> channels;

	Mix_Chunk *sound = nullptr;

public:
	static void clear();
	static void make(const std::string &name, const std::string &mixer, bool loop);
	static void play(const std::string &desc);
	static void stop(const std::string &desc);
	static void setVolume(float value, const std::string &channelName);

	String name;
	String mixer;
	bool loop;
	int num;

	size_t volume;//0..127

	MusicChannel(const String &name, const String &mixer, bool loop);
	~MusicChannel();
};

#endif // MUSICCHANNEL_H
