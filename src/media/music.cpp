#include "music.h"

#include <thread>

#include "gv.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/math.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::mutex globalMutex;
static bool needToClear = false;


int Music::startUpdateTime = 0;

std::map<std::string, double> Music::mixerVolumes;

std::vector<Channel*> Music::channels;
std::vector<Music*> Music::musics;


void Music::fillAudio(void *, Uint8 *stream, int globalLen) {
	std::lock_guard g(globalMutex);

	SDL_memset(stream, 0, size_t(globalLen));

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		if (!music->audioPos || music->audioLen <= 0) continue;

		Uint32 len = Uint32(globalLen) > music->audioLen ? music->audioLen : Uint32(globalLen);

		int volume = music->getVolume();
		if (volume > 0) {
			SDL_MixAudio(stream, music->audioPos, len, volume);
		}
		music->audioPos += len;
		music->audioLen -= len;
	}
}
void Music::init() {
#if !FF_API_NEXT
	av_register_all();
#endif
	av_log_set_level(AV_LOG_ERROR);

	auto loop = [&] {
		while (true) {
			startUpdateTime = Utils::getTimer();

			{
				std::lock_guard g(globalMutex);

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
							music->audioPos = nullptr;
							music->audioLen = 0;
							if (av_seek_frame(music->formatCtx, music->audioStream, 0, AVSEEK_FLAG_ANY) < 0) {
								Utils::outMsg("Music::loop",
											  "Ошибка при перемотке к началу, запущено:\n" + music->place);
							}else {
								needToDelete = false;
								music->loadNextParts(MIN_PART_COUNT);
							}
						}

						if (needToDelete) {
							musics.erase(musics.begin() + int(i));
							--i;
							delete music;
						}
					}else {
						music->update();
					}
				}
			}
			const int spend = Utils::getTimer() - startUpdateTime;
			Utils::sleep(10 - spend, false);
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
		Utils::outMsg("Music::init: SDL_OpenAudio", SDL_GetError());
		return;
	}
	SDL_PauseAudio(0);

	std::thread(loop).detach();
}

void Music::clear() {
	if (startUpdateTime) {//updated, => SDL_OpenAudio is OK, => clearing in Music::init::loop
		needToClear = true;
	}else {//error on SDL_OpenAudio, => clearing now (Music::init::loop not started)
		std::lock_guard g(globalMutex);
		realClear();
	}
}

void Music::realClear() {
	mixerVolumes.clear();

	for (Channel *t : channels) {
		delete t;
	}
	channels.clear();

	for (Music *t : musics) {
		delete t;
	}
	musics.clear();
}

void Music::registerChannel(const std::string &name, const std::string &mixer, bool loop,
							const std::string &fileName, int numLine)
{
	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == name) {
			const std::string place = "Файл <" + fileName + ">\n"
			                          "Строка " + std::to_string(numLine);
			Utils::outMsg("Music::registerChannel",
			              "Канал с именем <" + name + "> уже существует\n\n" + place);
			return;
		}
	}

	Channel *t = new Channel(name, mixer, loop);
	channels.push_back(t);

	if (mixerVolumes.find(mixer) == mixerVolumes.end()) {
		mixerVolumes[mixer] = 1;
	}
}
bool Music::hasChannel(const std::string &name) {
	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == name) {
			return true;
		}
	}
	return false;
}


void Music::setVolume(double volume, const std::string &channelName,
					  const std::string &fileName, int numLine)
{
	const std::string place = "Файл <" + fileName + ">\n"
	                          "Строка " + std::to_string(numLine);

	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == channelName) {
			channel->volume = Math::inBounds(volume, 0, 1);
			return;
		}
	}

	Utils::outMsg("Music::setVolume",
				  "Канал с именем <" + channelName + "> не существует\n\n" + place);
}
void Music::setMixerVolume(double volume, const std::string &mixer,
						   const std::string &fileName, int numLine)
{
	const std::string place = "Файл <" + fileName + ">\n"
	                          "Строка " + std::to_string(numLine);

	if (mixerVolumes.find(mixer) != mixerVolumes.end()) {
		mixerVolumes[mixer] = Math::inBounds(volume, 0, 1);
	}else {
		Utils::outMsg("Music::setMixerVolume",
		              "Микшер с именем <" + mixer + "> сейчас не используется никаким каналом\n\n" + place);
	}
}


void Music::play(const std::string &desc,
				 const std::string &fileName, size_t numLine)
{
	std::vector<std::string> args = Algo::getArgs(desc);
	if (args.size() != 2 && args.size() != 4) {
		Utils::outMsg("Music::play",
					  "Команда play ожидает 2 или 4 параметра:\n"
					  "channelName fileName ['fadein' time]\n"
					  "Получено: <" + desc + ">");
		return;
	}

	std::string channelName = Algo::clear(args[0]);
	std::string url = PyUtils::exec(fileName, numLine, args[1], true);
	for (size_t i = 0; i < url.size(); ++i) {
		if (url[i] == '\\') {
			url[i] = '/';
		}
	}

	const std::string place = "Файл <" + fileName + ">\n"
	                          "Строка " + std::to_string(numLine) + ": <" + desc + ">";

	int fadeIn = 0;
	if (args.size() > 2) {
		if (args[2] != "fadein") {
			Utils::outMsg("Music::play",
						  "3-м параметром ожидалось слово <fadein>\n\n" + place);
			return;
		}
		std::string fadeInStr = PyUtils::exec(fileName, numLine, args[3], true);
		if (!String::isNumber(fadeInStr)) {
			Utils::outMsg("Music::play",
						  "Значением параметра fadein ожидалось число, получено <" + args[3] + ">\n\n" + place);
			return;
		}
		fadeIn = int(String::toDouble(fadeInStr) * 1000);
	}


	std::lock_guard g(globalMutex);

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		if (music->channel->name == channelName) {
			musics.erase(musics.begin() + int(i));
			delete music;

			break;
		}
	}

	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == channelName) {
			Music *music = new Music(url, channel, fadeIn, fileName, numLine, place);
			if (music->initCodec()){
				delete music;
			}else {
				music->loadNextParts(MIN_PART_COUNT);
				musics.push_back(music);
			}
			return;
		}
	}

	Utils::outMsg("Music::play", "Канал с именем <" + channelName + "> не найден\n\n" + place);
}
void Music::stop(const std::string &desc,
				 const std::string& fileName, size_t numLine)
{
	const std::string place = "Файл <" + fileName + ">\n"
	                          "Строка " + std::to_string(numLine) + ": <" + desc + ">";

	std::vector<std::string> args = Algo::getArgs(desc);
	if (args.size() != 1 && args.size() != 3) {
		Utils::outMsg("Music::stop",
					  "Команда stop ожидает 1 или 3 параметра:\n"
					  "channelName ['fadeout' time]\n\n" + place);
		return;
	}

	std::string channelName = Algo::clear(args[0]);

	int fadeOut = 0;
	if (args.size() > 1) {
		if (args[1] != "fadeout") {
			Utils::outMsg("Music::stop",
						  "3-м параметром ожидалось слово <fadeout>\n\n" + place);
			return;
		}
		std::string fadeOutStr = PyUtils::exec(fileName, numLine, args[2], true);
		if (!String::isNumber(fadeOutStr)) {
			Utils::outMsg("Music::stop",
						  "Значением параметра fadeout ожидалось число, получено <" + args[2] + ">\n\n" + place);
			return;
		}
		fadeOut = int(String::toDouble(fadeOutStr) * 1000);
	}

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		if (music->channel->name == channelName) {
			music->fadeOut = fadeOut;
			music->startFadeOutTime = Utils::getTimer();
			return;
		}
	}
	Utils::outMsg("Music::stop", "На канале с именем <" + channelName + "> сейчас ничего не проигрывается\n\n" + place);
}




Music::Music(const std::string &url, Channel *channel, int fadeIn,
			 const std::string &fileName, size_t numLine, const std::string &place):
	url(url),
	channel(channel),
	startFadeInTime(Utils::getTimer()),
	fadeIn(fadeIn),
	fileName(fileName),
	numLine(numLine),
	place(place)
{ }


bool Music::initCodec() {
	av_init_packet(packet);

	formatCtx = avformat_alloc_context();
	if (int error = avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("Music::initCodec: avformat_open_input",
						  "Файл <" + url + "> не найден\n\n" + place);
			return true;
		}
		Utils::outMsg("Music::initCodec: avformat_open_input",
					  "Не удалось открыть поток в файле <" + url + ">\n\n" + place);
		return true;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("Music::initCodec: avformat_find_stream_info",
					  "Не удалось найти информацию о потоке в файле <" + url +">\n\n" + place);
		return true;
	}

	for (size_t i = 0; i < formatCtx->nb_streams; ++i) {
		if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioStream = int(i);
			break;
		}
	}
	if (audioStream == -1) {
		Utils::outMsg("Music::initCodec",
					  "Не удалось найти аудио-поток в файле <" + url + ">\n\n" + place);
		return true;
	}

	AVCodecParameters *codecPars = formatCtx->streams[audioStream]->codecpar;
	AVCodec *codec = avcodec_find_decoder(codecPars->codec_id);
	if (!codec) {
		Utils::outMsg("Music::initCodec: avcodec_find_decoder",
					  "Кодек для файла <" + url + "> не найден\n\n" + place);
		return true;
	}

	codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, codecPars);

	if (avcodec_open2(codecCtx, codec, nullptr)) {
		Utils::outMsg("Music::initCodec: avcodec_open2",
					  "Не удалось открыть кодек для файла <" + url + ">\n\n" + place);
		return true;
	}

	int64_t outChannelLayout = AV_CH_LAYOUT_STEREO;
	AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
	int64_t inChannelLayout = av_get_default_channel_layout(codecCtx->channels);

	auConvertCtx = swr_alloc_set_opts(nullptr, outChannelLayout, outSampleFmt, 44100,
									  inChannelLayout, codecCtx->sample_fmt, codecCtx->sample_rate, 0, nullptr);
	if (!auConvertCtx) {
		Utils::outMsg("Music::initCodec: swr_alloc_set_opts",
					  "Не удалось создать SwrContext для конвертации потока файла <" + url + ">\n\n" + place);
		return true;
	}
	if (swr_init(auConvertCtx)) {
		Utils::outMsg("Music::initCodec: swr_init",
					  "Не удалось инициализировать SwrContext для конвертации потока файла <" + url + ">\n\n" + place);
		return true;
	}

	return false;
}
void Music::setPos(int64_t pos) {
	std::lock_guard g(globalMutex);

	audioPos = buffer;
	audioLen = 0;

	if (av_seek_frame(formatCtx, audioStream, pos, AVSEEK_FLAG_FRAME) < 0) {
		Utils::outMsg("Music::setPos", "Не удалось перемотать файл <" + url + ">");
	}else {
		loadNextParts(MIN_PART_COUNT);
	}
}


Music::~Music() {
	audioPos = nullptr;
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
			Utils::outMsg("Music::loadNextParts: avcodec_send_packet",
						  "Не удалось отправить пакет кодеку файла <" + url + ">\n\n" + place);
			return;
		}

		int reveiveError = !sendError ? avcodec_receive_frame(codecCtx, frame) : 0;
		if (reveiveError && reveiveError != AVERROR(EAGAIN)) {
			av_packet_unref(packet);
			Utils::outMsg("Music::loadNextParts: avcodec_receive_frame",
						  "Не удалось принять кадр (AVFrame) потока файла <" + url + ">\n\n" + place);
			return;
		}
		lastFramePts = frame->pts;

		int countSamples = 0;
		if (!sendError && !reveiveError) {
			countSamples = swr_convert(auConvertCtx,
									   &tmpBuffer, PART_SIZE,
									   (const uint8_t **)frame->data, frame->nb_samples);
			if (countSamples < 0) {
				Utils::outMsg("Music::loadNextParts: swr_convert",
							  "Не удалось конвертировать кадр (AVFrame) потока файла <" + url + ">\n\n" + place);
				av_packet_unref(packet);
				av_frame_unref(frame);
				return;
			}
		}

		Uint32 lastLen = Uint32(countSamples) * 2 * 2;//stereo (2 channels), 16 bits (2 bytes)

		if (audioPos && audioLen && audioPos != buffer) {
			uint8_t *old = new uint8_t[audioLen]();
			memcpy(old, audioPos, audioLen);

			memcpy(buffer, old, audioLen);
			delete[] old;
		}
		memcpy(buffer + audioLen, tmpBuffer, lastLen);

		audioPos = buffer;
		audioLen += lastLen;

		av_frame_unref(frame);
		av_packet_unref(packet);
	}

	if (!audioLen) {
		ended = true;
	}
}

int Music::getVolume() const {
	int max = int(SDL_MIX_MAXVOLUME * channel->volume * mixerVolumes[channel->mixer]);

	//fadeIn
	if (fadeIn && (startFadeInTime + fadeIn > startUpdateTime)) {
		return max * (startUpdateTime - startFadeInTime) / fadeIn;
	}

	//fadeOut
	if (fadeOut && startFadeOutTime + fadeOut > startUpdateTime) {
		return int(max * (1 - double(startUpdateTime - startFadeOutTime) / fadeOut));
	}

	//usual
	return max;
}
bool Music::isEnded() const {
	return ended || (startFadeOutTime && (startFadeOutTime + fadeOut < startUpdateTime));
}
