#include "music.h"

#include <thread>

#include "gv.h"
#include "media/py_utils.h"
#include "utils/utils.h"


bool Music::needToClear = false;
int Music::startUpdateTime = 0;
std::mutex Music::globalMutex;

std::map<std::string, double> Music::mixerVolumes;

std::vector<Channel*> Music::channels;
std::vector<Music*> Music::musics;


void Music::fillAudio(void *, Uint8 *stream, int globalLen) {
	SDL_memset(stream, 0, globalLen);

	std::lock_guard<std::mutex> g(globalMutex);

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		std::lock_guard<std::mutex> g(music->mutex);

		if (!music->audioPos || music->audioLen <= 0) {
			continue;
		}

		Uint32 len = Uint32(globalLen) > music->audioLen ? music->audioLen : globalLen;

		int volume = music->getVolume();
		if (volume > 0) {
			SDL_MixAudio(stream, music->audioPos, len, volume);
		}
		music->audioPos += len;
		music->audioLen -= len;
	}
}
void Music::init() {
	av_register_all();
	av_log_set_level(AV_LOG_ERROR);

	auto loop = [&] {
		while (true) {
			startUpdateTime = Utils::getTimer();

			if (needToClear) {
				needToClear = false;
				realClear();
			}

			for (size_t i = 0; i < musics.size(); ++i) {
				if (GV::exit) return;

				Music *music = musics[i];

				if (music->isEnded()) {
					bool needToDelete = true;

					if (music->channel->loop && music->ended) {//ended, not stopped
						music->ended = false;
						music->audioPos = 0;
						music->audioLen = 0;
						if (av_seek_frame(music->formatCtx, music->audioStream, 0, AVSEEK_FLAG_ANY) < 0) {
							Utils::outMsg("Music::loop", "Seek frame failed");
						}else {
							needToDelete = false;
							music->loadNextParts(MIN_PART_COUNT);
						}
					}

					if (needToDelete) {
						std::lock_guard<std::mutex> g(globalMutex);

						musics.erase(musics.begin() + i, musics.begin() + i + 1);
						--i;
						delete music;
					}
				}else {
					music->update();
				}
			}

			int spend = Utils::getTimer() - startUpdateTime;
			Utils::sleep(10 - spend);
		}
	};


	static SDL_AudioSpec wantedSpec;
	wantedSpec.freq = 44100;
	wantedSpec.format = AUDIO_S16SYS;
	wantedSpec.channels = 2;
	wantedSpec.silence = 0;
	wantedSpec.samples = 0;
	wantedSpec.callback = fillAudio;

	if (SDL_OpenAudio(&wantedSpec, nullptr)) {
		Utils::outMsg("Can't open audio");
		return;
	}
	SDL_PauseAudio(0);

	std::thread(loop).detach();
}

void Music::clear() {
	needToClear = true;
}

void Music::realClear() {
	mixerVolumes.clear();

	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *t = channels[i];
		delete t;
	}
	channels.clear();

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *t = musics[i];
		delete t;
	}
	musics.clear();
}

void Music::registerChannel(const std::string &name, const std::string &mixer, bool loop) {
	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == name) {
			Utils::outMsg("Music::registerChannel", "Channel with name <" + name + "> is there");
			return;
		}
	}

	Channel *t = new Channel(name, mixer, loop);
	channels.push_back(t);

	if (mixerVolumes.find(mixer) == mixerVolumes.end()) {
		mixerVolumes[mixer] = 1;
	}
}
void Music::setVolume(double volume, const std::string &channelName) {
	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == channelName) {
			channel->volume = Utils::inBounds(volume, 0, 1);
			return;
		}
	}

	Utils::outMsg("Music::setVolume", "Channel with name <" + channelName + "> isn't there");
}
void Music::setMixerVolume(double volume, const std::string &mixer) {
	if (mixerVolumes.find(mixer) != mixerVolumes.end()) {
		mixerVolumes[mixer] = Utils::inBounds(volume, 0, 1);
	}else {
		Utils::outMsg("Music::setMixerVolume", "Mixer with name <" + mixer + "> don't uses now");
	}
}


void Music::play(const std::string &desc) {
	std::vector<String> args = Utils::getArgs(desc);
	if (args.size() != 2 && args.size() != 4) {
		Utils::outMsg("Music::play",
					  "Команда play ожидает 2 или 4 параметра:\n"
					  "channelName fileName ['fadein' time]\n"
					  "Получено: <" + desc + ">");
		return;
	}

	String channelName = Utils::clear(args[0]);
	String url = Utils::ROOT + PyUtils::exec("CPP_EMBED: music.cpp", 0, args[1], true);

	int fadeIn = 0;
	if (args.size() > 2) {
		if (args[2] != "fadein") {
			Utils::outMsg("Music::play",
						  "3-м параметром ожидалось слово <fadein>\n"
						  "Строка <" + desc + ">");
			return;
		}
		String fadeInStr = PyUtils::exec("CPP_EMBED: music.cpp", 0, "float(" + args[3] + ")", true);
		if (!fadeInStr.isNumber()) {
			Utils::outMsg("Music::play",
						  "Значением параметра fadein ожидалось число, получено <" + args[3] + ">\n" +
						  "Строка <" + desc + ">");
			return;
		}
		fadeIn = fadeInStr.toDouble() * 1000;
	}


	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		if (music->channel->name == channelName) {
			{
				std::lock_guard<std::mutex> g(globalMutex);
				musics.erase(musics.begin() + i, musics.begin() + i + 1);
			}
			delete music;
			break;
		}
	}

	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == channelName) {
			Music *music = new Music(url, channel, fadeIn);
			if (music->initCodec()){
				delete music;
			}else {
				music->loadNextParts(MIN_PART_COUNT);

				std::lock_guard<std::mutex> g(globalMutex);
				musics.push_back(music);
			}
			return;
		}
	}

	Utils::outMsg("Music::play", "Channel with name <" + channelName + "> not found");
}
void Music::stop(const std::string &desc) {
	std::vector<String> args = Utils::getArgs(desc);
	if (args.size() != 1 && args.size() != 3) {
		Utils::outMsg("Music::stop",
					  "Команда stop ожидает 1 или 3 параметра:\n"
					  "channelName ['fadeout' time]\n"
					  "Получено: <" + desc + ">");
		return;
	}

	String channelName = Utils::clear(args[0]);

	int fadeOut = 0;
	if (args.size() > 1) {
		if (args[1] != "fadeout") {
			Utils::outMsg("Music::stop",
						  "3-м параметром ожидалось слово <fadeout>\n"
						  "Строка <" + desc + ">");
			return;
		}
		String fadeOutStr = PyUtils::exec("CPP_EMBED: music.cpp", 0, "float(" + args[2] + ")", true);
		if (!fadeOutStr.isNumber()) {
			Utils::outMsg("Music::stop",
						  "Значением параметра fadeout ожидалось число, получено <" + args[2] + ">\n" +
						  "Строка <" + desc + ">");
			return;
		}
		fadeOut = fadeOutStr.toDouble() * 1000;
	}

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		if (music->channel->name == channelName) {
			music->fadeOut = fadeOut;
			music->startFadeOutTime = Utils::getTimer();
			return;
		}
	}
	Utils::outMsg("Music::stop", "On channel with name <" + channelName + "> nothing playing now");
}




Music::Music(const std::string &url, Channel *channel, int fadeIn):
	url(url),
	channel(channel),
	startFadeInTime(Utils::getTimer()),
	fadeIn(fadeIn)
{ }


bool Music::initCodec() {
	av_init_packet(packet);

	formatCtx = avformat_alloc_context();
	if (int error = avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("Music::initCodec", "File <" + url + "> not found");
			return true;
		}
		Utils::outMsg("Music::initCodec", "Couldn't open input stream");
		return true;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("Music::initCodec", "Couldn't find stream information");
		return true;
	}

	for (size_t i = 0; i < formatCtx->nb_streams; ++i) {
		if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioStream = i;
			break;
		}
	}
	if (audioStream == -1) {
		Utils::outMsg("Music::initCodec", "Didn't find a audio stream");
		return true;
	}

	AVCodecParameters *codecPars = formatCtx->streams[audioStream]->codecpar;
	AVCodec *codec = avcodec_find_decoder(codecPars->codec_id);
	if (!codec) {
		Utils::outMsg("Music::initCodec", "Codec not found");
		return true;
	}

	codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, codecPars);

	if (avcodec_open2(codecCtx, codec, nullptr)) {
		Utils::outMsg("Music::initCodec", "Can't open codec");
		return true;
	}

	uint64_t outChannelLayout = AV_CH_LAYOUT_STEREO;
	AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
	int64_t inChannelLayout = av_get_default_channel_layout(codecCtx->channels);

	auConvertCtx = swr_alloc_set_opts(nullptr, outChannelLayout, outSampleFmt, 44100,
									  inChannelLayout, codecCtx->sample_fmt, codecCtx->sample_rate, 0, nullptr);
	if (!auConvertCtx) {
		Utils::outMsg("Music::initCodec", "Allocate convert context failed");
		return true;
	}
	if (swr_init(auConvertCtx)) {
		Utils::outMsg("Music::initCodec", "Init convert context failed");
		return true;
	}

	return false;
}

Music::~Music() {
	audioPos = 0;
	audioLen = 0;

	av_free(packet);
	av_free(frame);
	packet = nullptr;
	frame = nullptr;

	av_free(buffer);
	av_free(tmpBuffer);
	buffer = nullptr;
	tmpBuffer = nullptr;

	avcodec_close(codecCtx);
	avformat_close_input(&formatCtx);
	codecCtx = nullptr;
	formatCtx = nullptr;

	swr_free(&auConvertCtx);
	auConvertCtx = nullptr;
}


void Music::update() {
	if (audioLen < Uint32(PART_SIZE * MIN_PART_COUNT)) {
		loadNextParts(MAX_PART_COUNT);
	}
}
void Music::loadNextParts(size_t count) {
	while (audioLen < PART_SIZE * count && !av_read_frame(formatCtx, packet)) {
		if (packet->stream_index != audioStream) {
			av_packet_unref(packet);
			continue;
		}

		int sendError = avcodec_send_packet(codecCtx, packet);
		if (sendError && sendError != AVERROR(EAGAIN)) {
			Utils::outMsg("Music::loadNextParts", "Send packet failed");
			return;
		}

		int reveiveError = !sendError ? avcodec_receive_frame(codecCtx, frame) : 0;
		if (reveiveError && reveiveError != AVERROR(EAGAIN)) {
			av_packet_unref(packet);
			Utils::outMsg("Music::loadNextParts", "Receive frame failed");
			return;
		}

		int countSamples = 0;
		if (!sendError && !reveiveError) {
			countSamples = swr_convert(auConvertCtx,
											&tmpBuffer, PART_SIZE,
											(const uint8_t **)frame->data, frame->nb_samples);
			if (countSamples < 0) {
				Utils::outMsg("Music::loadNextParts", "Swr convert failed");
				av_packet_unref(packet);
				av_frame_unref(frame);
				return;
			}
		}

		{
			std::lock_guard<std::mutex> g(mutex);

			int lastLen = countSamples * 2 * 2;//stereo (2 channels), 16 bits (2 bytes)

			if (audioPos && audioLen && audioPos != buffer) {
				uint8_t *old = new uint8_t[audioLen]();
				memcpy(old, audioPos, audioLen);

				memcpy(buffer, old, audioLen);
				delete[] old;
			}
			memcpy(buffer + audioLen, tmpBuffer, lastLen);

			audioPos = buffer;
			audioLen += lastLen;
		}

		av_frame_unref(frame);
		av_packet_unref(packet);
	}

	if (!audioLen) {
		ended = true;
	}
}

int Music::getVolume() const {
	int max = SDL_MIX_MAXVOLUME * channel->volume * mixerVolumes[channel->mixer];

	//fadeIn
	if (fadeIn && (startFadeInTime + fadeIn > startUpdateTime)) {
		return max * (startUpdateTime - startFadeInTime) / fadeIn;
	}

	//fadeOut
	if (fadeOut && startFadeOutTime + fadeOut > startUpdateTime) {
		return max * (1 - double(startUpdateTime - startFadeOutTime) / fadeOut);
	}

	//usual
	return max;
}
bool Music::isEnded() const {
	return ended || (startFadeOutTime && (startFadeOutTime + fadeOut < startUpdateTime));
}
