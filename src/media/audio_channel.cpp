#define ALLOW_INCLUDE_AUDIO_CHANNEL_H
#include "audio_channel.h"

#include <SDL2/SDL_audio.h>

#include "media/audio_manager.h"
#include "utils/scope_exit.h"
#include "utils/utils.h"



Channel::Channel(const std::string& name, const std::string &mixer, bool loop):
    name(name),
    mixer(mixer),
    loop(loop),

    packet(av_packet_alloc()),
    frame(av_frame_alloc()),

    tmpBuffer((uint8_t *)av_malloc(PART_SIZE)),
    buffer((uint8_t *)av_malloc(PART_SIZE * (MAX_PART_COUNT + 1)))
{}

Channel::~Channel() {
	for (Audio *audio : audios) {
		delete audio;
	}
	audios.clear();

	audioPos = nullptr;
	audioLen = 0;

	av_freep(&buffer);
	av_freep(&tmpBuffer);

	av_packet_free(&packet);
	av_frame_free(&frame);

	avcodec_free_context(&codecCtx);
	avformat_close_input(&formatCtx);
	swr_free(&convertCtx);
}

Audio* Channel::getAudio() const {
	if (!audios.empty()) {
		return audios.front();
	}
	return nullptr;
}
void Channel::clearQueue(double fadeOut) {
	if (!audios.empty()) {
		for (size_t i = 1; i < audios.size(); ++i) {
			delete audios[i];
		}
		audios.erase(audios.begin() + 1, audios.end());
		setFadeOut(fadeOut);
	}
}


bool Channel::initCodec() {
	fullDecoded = true;

	Audio *audio = getAudio();
	const std::string &path = audio->path;
	const std::string &place = audio->place;

	//played from start, not loading
	if (audio->startFadeInTime < 0) {
		audio->startFadeInTime = AudioManager::startUpdateTime;
	}

	//deinit prev data
	avcodec_free_context(&codecCtx);
	avformat_close_input(&formatCtx);
	swr_free(&convertCtx);


	formatCtx = avformat_alloc_context();
	if (int error = avformat_open_input(&formatCtx, path.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("Channel::initCodec: avformat_open_input",
			              "File <" + path + "> not found\n\n" + place);
		}else {
			Utils::outMsg("Channel::initCodec: avformat_open_input",
			              "Failed to open input stream in file <" + path + ">\n\n" + place);
		}
		return true;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("Channel::initCodec: avformat_find_stream_info",
		              "Failed to read stream information in file <" + path + ">\n\n" + place);
		return true;
	}

	audioStream = -1;
	for (uint32_t i = 0; i < formatCtx->nb_streams; ++i) {
		if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = int(i);
			break;
		}
	}
	if (audioStream == -1) {
		Utils::outMsg("Channel::initCodec",
		              "Failed to find audio-stream in file <" + path + ">\n\n" + place);
		return true;
	}

	AVCodecParameters *codecPars = formatCtx->streams[audioStream]->codecpar;
	const AVCodec *codec = avcodec_find_decoder(codecPars->codec_id);
	if (!codec) {
		Utils::outMsg("Channel::initCodec: avcodec_find_decoder",
		              "Codec for file <" + path + "> not found\n\n" + place);
		return true;
	}

	codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, codecPars);

	if (avcodec_open2(codecCtx, codec, nullptr)) {
		Utils::outMsg("Channel::initCodec: avcodec_open2",
		              "Failed to open codec for file <" + path + ">\n\n" + place);
		return true;
	}

	AVChannelLayout in, out;
	av_channel_layout_default(&in, codecCtx->ch_layout.nb_channels);
	av_channel_layout_default(&out, 2); //2 = stereo

	convertCtx = nullptr;
	if (swr_alloc_set_opts2(&convertCtx,
	                        &out, AV_SAMPLE_FMT_S16, 44100,
	                        &in, codecCtx->sample_fmt, codecCtx->sample_rate,
	                        0, nullptr))
	{
		Utils::outMsg("Channel::initCodec: swr_alloc_set_opts2",
		              "Failed to create SwrContext for convert stream of file <" + path + ">\n\n" + place);
		return true;
	}
	if (swr_init(convertCtx)) {
		Utils::outMsg("Channel::initCodec: swr_init",
		              "Failed to init SwrContext for convert stream of file <" + path + ">\n\n" + place);
		return true;
	}

	fullDecoded = false;

	return false;
}


void Channel::update(bool minUpdate) {
	Audio *audio = getAudio();
	if (!audio) return;

	if (audioStream == -1) {
		if (initCodec()) return;
	}

	auto removeFirst = [&]() {
		delete audios.front();
		audios.erase(audios.begin());
		fullDecoded = false;
		curTime = -1;
		audioStream = -1;
		audioLen = 0;

		if (!audios.empty()) {
			initCodec();
		}
	};

	if (audio->fadeOut >= 0) {
		double dtime = AudioManager::startUpdateTime - audio->startFadeOutTime;
		if (dtime >= audio->fadeOut) {
			removeFirst();
			if (audios.empty()) return;
		}
	}

	if (audioLen >= PART_SIZE * MIN_PART_COUNT) return;

	size_t size = PART_SIZE * (minUpdate ? MIN_PART_COUNT : MAX_PART_COUNT);
	while (!audios.empty() && audioLen < size) {
		loadNextFrame();

		if (fullDecoded) {
			if (!audioLen) {
				removeFirst();
			}else {
				break;
			}
		}
	}
}

void Channel::loadNextFrame() {
	const Audio *audio = getAudio();
	const std::string &path = audio->path;
	const std::string &place = audio->place;

	ScopeExit se([&]() {
		av_packet_unref(packet);
		av_frame_unref(frame);
	});

	if (int error = av_read_frame(formatCtx, packet)) {
		if (error != AVERROR_EOF) {
			Utils::outMsg("Channel::loadNextFrame: av_read_frame",
			              "Failed to read frame from file <" + path + ">\n\n" + place);
			return;
		}

		curTime = -1;

		if (!loop || audios.size() > 1) {
			fullDecoded = true;
			return;
		}

		avcodec_send_packet(codecCtx, nullptr);
		if (av_seek_frame(formatCtx, audioStream, 0, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD) < 0) {
			Utils::outMsg("Channel::loadNextFrame",
			              "Error on restart, playing from:\n" + place);
			return;
		}
		avcodec_flush_buffers(codecCtx);
		return;
	}

	if (packet->stream_index != audioStream) return;

	if (int error = avcodec_send_packet(codecCtx, packet)) {
		if (error != AVERROR(EAGAIN)) {
			Utils::outMsg("Channel::loadNextFrame: avcodec_send_packet",
			              "Failed to send packet to codec of file <" + path + ">\n\n" + place);
		}
		return;
	}

	if (int error = avcodec_receive_frame(codecCtx, frame)) {
		if (error != AVERROR(EAGAIN)) {
			Utils::outMsg("Channel::loadNextFrame: avcodec_receive_frame",
			              "Failed to receive frame from codec of file <" + path + ">\n\n" + place);
		}
		return;
	}

	int sampleCount = swr_convert(convertCtx,
	                              &tmpBuffer, SAMPLES_PER_PART,
	                              frame->data, frame->nb_samples);
	if (sampleCount < 0) {
		Utils::outMsg("Channel::loadNextFrame: swr_convert",
		              "Failed to convert frame of file <" + path + ">\n\n" + place);
		return;
	}

	if (curTime < 0) {
		auto k = formatCtx->streams[audioStream]->time_base;
		curTime = double(frame->pts) * k.num / k.den;
	}

	if (audioPos && audioLen && audioPos != buffer) {
		memmove(buffer, audioPos, audioLen);
	}
	Uint32 lastLen = Uint32(sampleCount) * SAMPLE_SIZE;
	memcpy(buffer + audioLen, tmpBuffer, lastLen);

	audioPos = buffer;
	audioLen += lastLen;

	if (curTime >= 0) {
		return;
	}
}

int Channel::getVolume() const {
	const Audio *audio = getAudio();

	double max = SDL_MIX_MAXVOLUME * audio->volume * volume * AudioManager::mixerVolumes[mixer];

	double dtimeFadeIn  = AudioManager::startUpdateTime - audio->startFadeInTime;
	double dtimeFadeOut = AudioManager::startUpdateTime - audio->startFadeOutTime;

	bool useFadeIn  = audio->fadeIn  > 0 && dtimeFadeIn  < audio->fadeIn;
	bool useFadeOut = audio->fadeOut > 0 && dtimeFadeOut < audio->fadeOut;

	double fadeInVolume  = useFadeIn  ? (      max * dtimeFadeIn  / audio->fadeIn)  : max;
	double fadeOutVolume = useFadeOut ? (max - max * dtimeFadeOut / audio->fadeOut) : max;

	return int(std::min(fadeInVolume, fadeOutVolume));
}

double Channel::getPos() const {
	return curTime;
}

void Channel::setFadeIn(double v) {
	Audio *audio = getAudio();
	if (audio) {
		audio->fadeIn = v;
		audio->startFadeInTime = AudioManager::startUpdateTime;
	}
}
void Channel::setFadeOut(double v) {
	Audio *audio = getAudio();
	if (audio && (audio->fadeOut < 0 || audio->fadeOut > v)) {
		audio->fadeOut = v;
		audio->startFadeOutTime = AudioManager::startUpdateTime;
	}
}
void Channel::setPos(double sec) {
	std::lock_guard g(AudioManager::mutex);

	const Audio *audio = getAudio();

	audioPos = buffer;
	audioLen = 0;

	if (!formatCtx) return;//on error in initCodec

	auto k = formatCtx->streams[audioStream]->time_base;
	int64_t ts = int64_t(sec / k.num * k.den);

	if (av_seek_frame(formatCtx, audioStream, ts, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD) < 0) {
		Utils::outMsg("Channel::setPos", "Failed to rewind file <" + audio->path + ">");
		return;
	}
	avcodec_flush_buffers(codecCtx);

	curTime = -1;
	while (curTime < 0) {
		ScopeExit se([&]() {
			av_packet_unref(packet);
			av_frame_unref(frame);
		});

		if (av_read_frame(formatCtx, packet) < 0) break;

		if (packet->stream_index != audioStream) continue;

		int sendError = avcodec_send_packet(codecCtx, packet);
		if (sendError == AVERROR(EAGAIN)) continue;
		if (sendError < 0) break;

		int receiveError = avcodec_receive_frame(codecCtx, frame);
		if (receiveError == AVERROR(EAGAIN)) continue;
		if (receiveError < 0) break;

		if (packet->pts >= ts) {
			curTime = double(frame->pts) * k.num / k.den;
		}
	}

	update(true);
}

void Channel::addToCurTime(size_t playedBytesCount) {
	size_t sampleCount = playedBytesCount / SAMPLE_SIZE;
	curTime += double(sampleCount) / 44100;
}
