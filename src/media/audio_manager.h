#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <iosfwd>
#include <map>
#include <mutex>
#include <string>

typedef struct _object PyObject;

struct AudioManager {
	static std::recursive_mutex mutex;
	static double startUpdateTime;
	static std::map<std::string, double> mixerVolumes;


	static void init();
	static void clear();

	static void registerChannel(const std::string &name, const std::string &mixer, bool loop,
	                            const std::string &fileName, size_t numLine);
	static bool hasChannel(const std::string &name);

	static PyObject* getAudioLen(const std::string &url);
	static void setMixerVolume(double volume, const std::string &mixer,
	                           const std::string &fileName, size_t numLine);

	static PyObject* getVolumeOnChannel(const std::string &channelName,
	                                    const std::string &fileName, size_t numLine);
	static void      setVolumeOnChannel(double volume, const std::string &channelName,
	                                    const std::string &fileName, size_t numLine);

	static PyObject* getPosOnChannel(const std::string &channelName,
	                                 const std::string &fileName, size_t numLine);
	static void      setPosOnChannel(double sec, const std::string &channelName,
	                                 const std::string &fileName, size_t numLine);

	static PyObject* getPauseOnChannel(const std::string &channelName,
	                                   const std::string &fileName, size_t numLine);
	static void      setPauseOnChannel(bool value, const std::string &channelName,
	                                   const std::string &fileName, size_t numLine);

	static void play(const std::string &channelName, std::string path,
	                 double fadeOut, double fadeIn, double volume,
	                 const std::string &fileName, uint32_t numLine);
	static void queue(const std::string &channelName, std::string path,
	                  double fadeIn, double volume,
	                  const std::string &fileName, uint32_t numLine);
	static void stop(const std::string &channelName, double fadeOut,
	                 const std::string &fileName, uint32_t numLine);

	static void  playWithParsing(const std::string &desc, const std::string &fileName, uint32_t numLine);
	static void queueWithParsing(const std::string &desc, const std::string &fileName, uint32_t numLine);
	static void  stopWithParsing(const std::string &desc, const std::string &fileName, uint32_t numLine);

	static PyObject* getPlaying(const std::string &channelName,
	                            const std::string &fileName, size_t numLine);

	static void save(std::ofstream &infoFile);
	static void load(std::ifstream &infoFile);
};

#endif // AUDIO_MANAGER_H
