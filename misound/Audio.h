#pragma once
#include <string>
#include <map>
#include <fstream>
#include <string>

#include <sndfile.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <json/json.h>
#include "Wave.h"
#include <dirent.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include "AlsaVolume.h"


namespace misound
{
	using namespace std;

	class Wave;

	class AudioGetWaveInterface
	{
	public:
		virtual const Wave& getWave(const std::string& wave) = 0;
	};

	struct VolumeScaleSet
	{
		double _VolumeMin;
		double _VolumeMax;
		misound::VolumeScaleMode _ScaleMode;
		bool _FromFile;

		VolumeScaleSet()
			:_VolumeMin(0.0)
			,_VolumeMax(100.0)
			,_ScaleMode(misound::VolumeScaleMode::percentToAlsa)
			,_FromFile(false)
		{

		}
		VolumeScaleSet(double volumeMin, double volumeMax, misound::VolumeScaleMode scalemode,bool fromFile)
			:_VolumeMin(volumeMin)
			,_VolumeMax(volumeMax)
			,_ScaleMode(scalemode)
			,_FromFile(fromFile)
		{

		}
	};

	class AudioInterface
	{
	public:
		virtual bool addWave(const Wave& wave) = 0;
		virtual bool addWave(const string& name, bool loop) = 0;
		virtual bool addWave(const string& path, const string& name, bool loop) = 0;
		virtual bool addWavesFromFolder(const string& folder, bool loop) = 0;
		virtual bool isPlaying(const string& name) = 0;
		virtual bool playWave(const string& name, bool restart = false) = 0;
		virtual bool playWave(const string& name, bool restart = false, bool loop = false) = 0;
		virtual bool playWave(const int num, bool restart = false) = 0;
		virtual bool stopWave(const string& name) = 0;
		virtual bool stopAllWave() = 0;
		virtual bool setVolume(double volumePercent) = 0;
		virtual bool changeSoundcard(const std::string soundcard) = 0;
	};

	class Audio : public AudioInterface
	{
		map<string, misound::Wave> _Waves;
		misound::AlsaVolume _Volume;
		std::string _SoundCard;
		std::string _RootPath;
		std::map<string, misound::VolumeScaleSet> _ScaleSets;
		const std::string _DefaultSoundCard = "plug:dmix0";
		const std::string _DefaultConfigPath = "/usr/share/misound/conf.d/misound.json";
		const std::string _DefaultRootPath = "/usr/share/misound/sounds";

		int getWaves(string dir, vector<string>& waves);
		std::string& replace(std::string& s, const std::string& from, const std::string& to);
		void getVolumeSettings(const std::string& path);
	public:
		Audio();
		Audio(const Audio& other) = delete;
		Audio operator=(const Audio& other) = delete;
		Audio(const std::string& soundCard);
		Audio(const std::string& soundCard, const std::string& rootPath);
		Audio(const std::string& soundCard, const std::string& rootPath, double volumeMin, double volumeMax, misound::VolumeScaleMode scalemode);
		~Audio();

		misound::AlsaVolume& getVolumeControl()
		{
			return _Volume;
		}
		
		virtual bool addWave(const Wave& wave);
		virtual bool addWave(const string& name, bool loop);
		virtual bool addWave(const string& path, const string& name, bool loop);
		virtual bool addWavesFromFolder(const string& folder, bool loop);
		virtual bool isPlaying(const string& name);
		virtual bool playWave(const string& name, bool restart = false);
		virtual bool playWave(const string& name, bool restart = false, bool loop = false);
		virtual bool playWave(const int num, bool restart = false);
		virtual bool stopWave(const string& name);
		virtual bool stopAllWave();
		virtual bool setVolume(double volumePercent);
		virtual bool changeSoundcard(const std::string soundcard);
	};
}

