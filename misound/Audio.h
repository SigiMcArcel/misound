#pragma once
#include <string>
#include <map>
#include <fstream>
#include <string>

#include <sndfile.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "Wave.h"
#include <dirent.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <algorithm>

namespace misound
{
	using namespace std;

	class Wave;

	class Audio
	{
		map<string, misound::Wave> _waves;
		misound::AlsaVolume _volume;
		int getWaves(string dir, vector<string>& waves);
		std::string& replace(std::string& s, const std::string& from, const std::string& to);
	public:
		Audio();
		~Audio();


		bool addWave(const Wave& wave);
		bool addWave(const string& path, const string& name, bool loop);
		bool addWavesFromFolder(const string& folder, bool loop);
		bool isPlaying(const string& name);
		bool playWave(const string& name, bool restart = false);
		bool playWave(const string& name, bool restart = false, bool loop = false);
		bool playWave(const int num, bool restart = false);
		bool stopWave(const string& name);
		bool stopAllWave();
		bool setVolume(int volume);
		bool changeSoundcard(const std::string soundcard);
	};
}

