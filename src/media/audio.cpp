#define ALLOW_INCLUDE_AUDIO_H
#include "audio.h"

#include <SDL2/SDL_audio.h>

#include "media/audio_manager.h"
#include "utils/utils.h"


Audio::Audio(const std::string &url, Channel *channel, double fadeIn, double volume,
             const std::string &fileName, size_t numLine, const std::string &place):
    url(url),
    channel(channel),
    startFadeInTime(Utils::getTimer()),
    fadeIn(fadeIn),
    relativeVolume(volume),
    fileName(fileName),
    numLine(numLine),
    place(place),

    packet(av_packet_alloc()),
    frame(av_frame_alloc()),

    tmpBuffer((uint8_t *)av_malloc(PART_SIZE)),
    buffer((uint8_t *)av_malloc(PART_SIZE * (MAX_PART_COUNT + 1)))
{}

Audio::~Audio() {
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


bool Audio::initCodec() {
	formatCtx = avformat_alloc_context();
	if (int error = avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("Audio::initCodec: avformat_open_input",
			              "File <" + url + "> not found\n\n" + place);
		}else {
			Utils::outMsg("Audio::initCodec: avformat_open_input",
			              "Failed to open input stream in file <" + url + ">\n\n" + place);
		}
		return true;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("Audio::initCodec: avformat_find_stream_info",
		              "Failed to read stream information in file <" + url + ">\n\n" + place);
		return true;
	}

	for (uint32_t i = 0; i < formatCtx->nb_streams; ++i) {
		if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = int(i);
			break;
		}
	}
	if (audioStream == -1) {
		Utils::outMsg("Audio::initCodec",
		              "Failed to find audio-stream in file <" + url + ">\n\n" + place);
		return true;
	}

	AVCodecParameters *codecPars = formatCtx->streams[audioStream]->codecpar;
	const AVCodec *codec = avcodec_find_decoder(codecPars->codec_id);
	if (!codec) {
		Utils::outMsg("Audio::initCodec: avcodec_find_decoder",
		              "Codec for file <" + url + "> not found\n\n" + place);
		return true;
	}

	codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, codecPars);

	if (avcodec_open2(codecCtx, codec, nullptr)) {
		Utils::outMsg("Audio::initCodec: avcodec_open2",
		              "Failed to open codec for file <" + url + ">\n\n" + place);
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
		Utils::outMsg("Audio::initCodec: swr_alloc_set_opts2",
		              "Failed to create SwrContext for convert stream of file <" + url + ">\n\n" + place);
		return true;
	}
	if (swr_init(convertCtx)) {
		Utils::outMsg("Audio::initCodec: swr_init",
		              "Failed to init SwrContext for convert stream of file <" + url + ">\n\n" + place);
		return true;
	}

	return false;
}



void Audio::update() {
	if (audioLen >= PART_SIZE * MIN_PART_COUNT) return;

	while (audioLen < PART_SIZE * MAX_PART_COUNT && !decoded) {
		loadNextPart();
	}
}
void Audio::loadNextPart() {
	while (true) {
		int readError = av_read_frame(formatCtx, packet);
		if (readError < 0) {
			if (readError != AVERROR_EOF) {
				Utils::outMsg("Audio::loadNextPart: av_read_frame",
				              "Failed to read frame from file <" + url + ">\n\n" + place);
				continue;
			}

			if (!channel->loop) {
				decoded = true;
				return;
			}

			curTime = -1;
			if (av_seek_frame(formatCtx, audioStream, 0, AVSEEK_FLAG_ANY) < 0) {
				Utils::outMsg("Audio::loadNextPart",
				              "Error on restart, playing from:\n" + place);
				return;
			}
			continue;
		}

		if (packet->stream_index != audioStream) {
			av_packet_unref(packet);
			continue;
		}

		int sendError = avcodec_send_packet(codecCtx, packet);
		if (sendError && sendError != AVERROR(EAGAIN)) {
			Utils::outMsg("Audio::loadNextPart: avcodec_send_packet",
			              "Failed to send packet to codec of file <" + url + ">\n\n" + place);
			return;
		}

		int reveiveError = !sendError ? avcodec_receive_frame(codecCtx, frame) : 0;
		if (reveiveError && reveiveError != AVERROR(EAGAIN)) {
			av_packet_unref(packet);
			Utils::outMsg("Audio::loadNextPart: avcodec_receive_frame",
			              "Failed to receive frame from codec of file <" + url + ">\n\n" + place);
			return;
		}

		int sampleCount = 0;
		if (!sendError && !reveiveError) {
			sampleCount = swr_convert(convertCtx,
			                          &tmpBuffer, SAMPLES_PER_PART,
			                          const_cast<const uint8_t**>(frame->data), frame->nb_samples);
			if (sampleCount < 0) {
				Utils::outMsg("Audio::loadNextPart: swr_convert",
				              "Failed to convert frame of file <" + url + ">\n\n" + place);
				av_packet_unref(packet);
				av_frame_unref(frame);
				return;
			}
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

		av_frame_unref(frame);
		av_packet_unref(packet);
		break;
	}
}

int Audio::getVolume() const {
	double max = SDL_MIX_MAXVOLUME * relativeVolume * channel->volume * AudioManager::mixerVolumes[channel->mixer];

	//fadeIn
	if (fadeIn > 0 && startFadeInTime + fadeIn > AudioManager::startUpdateTime) {
		return int(max * (AudioManager::startUpdateTime - startFadeInTime) / fadeIn);
	}

	//fadeOut
	if (fadeOut > 0 && startFadeOutTime + fadeOut > AudioManager::startUpdateTime) {
		return int(max - max * (AudioManager::startUpdateTime - startFadeOutTime) / fadeOut);
	}

	//usual
	return int(max);
}
bool Audio::isEnded() const {
	return !audioLen || (startFadeOutTime > 0 && startFadeOutTime + fadeOut < AudioManager::startUpdateTime);
}

double Audio::getFadeIn() const {
	return std::max(fadeIn + startFadeInTime - AudioManager::startUpdateTime, 0.0);
}
double Audio::getFadeOut() const {
	return std::max(fadeOut + startFadeOutTime - AudioManager::startUpdateTime, 0.0);
}
double Audio::getPos() const {
	return curTime;
}

void Audio::setFadeIn(double v) {
	fadeIn = v;
	startFadeInTime = AudioManager::startUpdateTime;
}
void Audio::setFadeOut(double v) {
	fadeOut = v;
	startFadeOutTime = AudioManager::startUpdateTime;
}
void Audio::setPos(double sec) {
	std::lock_guard g(AudioManager::mutex);

	curTime = -1;

	audioPos = buffer;
	audioLen = 0;

	auto k = formatCtx->streams[audioStream]->time_base;
	int64_t ts = int64_t(sec / k.num * k.den);

	if (av_seek_frame(formatCtx, audioStream, ts, AVSEEK_FLAG_ANY) < 0) {
		Utils::outMsg("Audio::setPos", "Failed to rewind file <" + url + ">");
	}else {
		update();
	}
}

void Audio::addToCurTime(size_t playedBytesCount) {
	size_t sampleCount = playedBytesCount / SAMPLE_SIZE;
	curTime += double(sampleCount) / 44100;
}
