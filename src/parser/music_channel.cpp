#include "music_channel.h"

#include "utils/utils.h"


std::vector<MusicChannel*> MusicChannel::channels;

void MusicChannel::clear() {
	for (MusicChannel *i : channels) {
		delete i;
	}
	channels.clear();
}

void MusicChannel::make(const std::string& name, const std::string& mixer, bool loop) {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->name == name) {
			delete channels[i];
			channels.erase(channels.begin() + i, channels.begin() + i + 1);
			break;
		}
	}

	MusicChannel *musicChannel = new MusicChannel(name, mixer, loop);
	channels.push_back(musicChannel);
	Mix_ReserveChannels(channels.size());
}
void MusicChannel::setVolume(float value, const std::string& channelName) {
	for (size_t i = 0; i < channels.size(); ++i) {
		MusicChannel *channel = channels[i];
		if (channel->name == channelName) {
			channel->volume = value * 128;
			if (channel->sound) {
				channel->sound->volume = value * 128;
			}
			return;
		}
	}
	Utils::outMsg("MusicChannel::setVolume", "Канал <" + channelName + "> не найден");
}

void MusicChannel::play(const std::string &desc) {
	std::vector<String> words = String(desc).split(" ");

	String soundCodeName = words[1];
	String soundName = Utils::execPython(soundCodeName, true);

	String soundChannelName = words[0];
	MusicChannel *soundChannel = nullptr;
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->name == soundChannelName) {
			soundChannel = channels[i];
			break;
		}
	}

	if (!soundChannel) {
		Utils::outMsg("NusicChannel::play", "Канал <" + soundChannelName + "> не найден");
		return;
	}

	SDL_RWops *sdlRWops = SDL_RWFromFile((Utils::ROOT + soundName).c_str(), "rb");
	if (!sdlRWops) {
		Utils::outMsg("SDL_RWFromFile", SDL_GetError());
		return;
	}

	Mix_Chunk *sound = Mix_LoadWAV_RW(sdlRWops, 1);
	if (!sound) {
		SDL_FreeRW(sdlRWops);
		Utils::outMsg("Mix_LoadWAV_RW", Mix_GetError());
		return;
	}

	sound->volume = soundChannel->volume;
	if (Mix_FadeInChannel(soundChannel->num, sound, soundChannel->loop ? -1 : 0, 0) == -1) {
		Utils::outMsg("Mix_PlayChannel", Mix_GetError());
		Mix_FreeChunk(sound);
		return;
	}

	if (soundChannel->sound) {
		Mix_FreeChunk(soundChannel->sound);
	}
	soundChannel->sound = sound;
}
void MusicChannel::stop(const String &desc) {
	std::vector<String> words = desc.split(" ");
	String channelName = words[0];

	MusicChannel *soundChannel = nullptr;
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->name == channelName) {
			soundChannel = channels[i];
			break;
		}
	}

	if (!soundChannel) {
		Utils::outMsg("MusicChannel::stop", "Канал <" + channelName + "> не найден");
		return;
	}

	if (soundChannel->sound) {
		Mix_FreeChunk(soundChannel->sound);
		soundChannel->sound = nullptr;
	}
	Mix_HaltChannel(soundChannel->num);
}

MusicChannel::MusicChannel(const String &name, const String &mixer, bool loop):
	name(name), mixer(mixer), loop(loop), num(channels.size()), volume(127) {}

MusicChannel::~MusicChannel() {
	stop(name);
}
