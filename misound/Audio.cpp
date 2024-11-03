#include "Audio.h"
#include <inttypes.h>
#include <pthread.h>
#include "AlsaStream.h"
#include <vector>
#include <fstream>
#include <iostream>


static pthread_mutex_t  SHAKE_BufferMutex = PTHREAD_MUTEX_INITIALIZER;
void shakeLock() { pthread_mutex_lock(&SHAKE_BufferMutex); }
void shakeUnlock() { pthread_mutex_unlock(&SHAKE_BufferMutex); }

using namespace std;

misound::Audio::Audio()
	:_Waves()
	,_Volume(_DefaultSoundCard)
	,_SoundCard(_DefaultSoundCard)
{
}

misound::Audio::Audio(const std::string& soundCard)
	:_Waves()
	, _Volume(soundCard)
	, _SoundCard(soundCard)
	, _RootPath("/usr/share/misound/sounds")
{
	_ScaleSets["default"] = VolumeScaleSet(0.0, 100.0, misound::VolumeScaleMode::linear, false);
	getVolumeSettings(_DefaultConfigPath);
}

misound::Audio::Audio(const std::string& soundCard, const std::string& rootPath)
	:_Waves()
	, _Volume(soundCard)
	, _SoundCard(soundCard)
	, _RootPath(rootPath)
{
	_ScaleSets["default"] = VolumeScaleSet(0.0, 100.0, misound::VolumeScaleMode::linear, false);
	getVolumeSettings(_DefaultConfigPath);
}

misound::Audio::Audio(const std::string& soundCard, const std::string& rootPath, double volumeMin, double volumeMax, misound::VolumeScaleMode scaleMode)
	:_Waves()
	, _Volume(soundCard)
	, _SoundCard(soundCard)
	, _RootPath(rootPath)
{
	_ScaleSets["default"] = VolumeScaleSet(volumeMin, volumeMax, scaleMode, false);
}

misound::Audio::~Audio()
{
}

bool misound::Audio::playWave(const string& name, bool restart )
{
	auto it = _Waves.find(name);
	if (it == _Waves.end())
	{
		return false;
	}
	if (!_Waves[name].isPlaying() || restart)
	{
		_Waves[name].play();
	}
		
	return true;
}

bool misound::Audio::playWave(const string& name, bool restart,bool loop)
{
	auto it = _Waves.find(name);
	if (it == _Waves.end())
	{
		return false;
	}
	_Waves[name].setLoop(loop);
	if (!_Waves[name].isPlaying() || restart)
	{
		_Waves[name].play();
	}

	return true;
}

bool misound::Audio::playWave(const int num, bool restart )
{
	int index = 0;
	map<string, Wave>::iterator it;
	if ((_Waves.size() == 0) || (_Waves.size() <= static_cast<size_t>(num)))
	{
		return false;
	}
	for (it = _Waves.begin(); it != _Waves.end(); it++)
	{
		if (index == num)
		{
			if (!it->second.isPlaying() || restart)
			{
				it->second.play();
			}
		}
		index++;
	}
	return true;
}

bool misound::Audio::isPlaying(const string& name)
{
	auto it = _Waves.find(name);
	if (it == _Waves.end())
	{
		return false;
	}
	return _Waves[name].isPlaying();
}

bool misound::Audio::stopWave(const string& name)
{
	auto it = _Waves.find(name);
	if (it == _Waves.end())
	{
		return false;
	}
	_Waves[name].stop();
	return true;
}

bool misound::Audio::stopAllWave()
{
	map<string, Wave>::iterator it;
	for (it = _Waves.begin(); it != _Waves.end(); it++)
	{	
		it->second.stop();
	}
	return true;
}

bool misound::Audio::addWave(const Wave& wave)
{
	_Waves.insert(std::make_pair(wave.getName(),wave));
	//printf( "cAudio::addWave : add Wave %s from %s count %d\n", wave.getName().c_str(), wave.getPath().c_str(),_waves.size());
	return true;
}
 
bool misound::Audio::addWave(const string & path, const string & name,bool loop = false)
{
	Wave w(path, name, _SoundCard, loop);
	
	if (w.error() > 0)
	{
		return false;
	}
	addWave(w);
	return true;
}

bool misound::Audio::addWave(const string& name, bool loop = false)
{
	if (_RootPath == "")
	{
		printf("cAudio::addWave : add Wave %s error: Wavepath not set\n", name.c_str());
		return false;
	}
	std::string path(_RootPath);
	path.append("/");
	path.append(name);
	path.append(".wav");
	Wave w(path, name, _SoundCard, loop);
	
	if (w.error() > 0)
	{
		return false;
	}
	addWave(w);
	return true;
}

bool misound::Audio::addWavesFromFolder(const string & folder,bool loop)
{
	
	vector<string> waves = vector<string>();
	if (getWaves(folder, waves) < 0)
	{
		return false;
	}
	for (std::vector<int>::size_type i = 0; i  != waves.size(); i++)
	{
		string name = waves[i];
		string path;
		path.append(folder);
		path.append(name);
		replace(name, ".wav", "");
		
		Wave* w = new Wave(path, name, _SoundCard, loop);
		addWave(*w);
	}
	return true;
}

bool misound::Audio::setVolume(double volumePercent)
{
	return _Volume.setVolume(volumePercent);
}

bool misound::Audio::changeSoundcard(const std::string soundcard)
{
	map<string, Wave>::iterator it;
	
	for (it = _Waves.begin(); it != _Waves.end(); it++)
	{
		it->second.changeSoundcard(soundcard);
	}
	if (_ScaleSets.find(soundcard) != _ScaleSets.end()) 
	{
		_Volume.setSoundcard(soundcard,_ScaleSets[soundcard]._VolumeMin, _ScaleSets[soundcard]._VolumeMax, _ScaleSets[soundcard]._ScaleMode);
	}
	else 
	{
		_Volume.setSoundcard(soundcard, _ScaleSets["default"]._VolumeMin, _ScaleSets["default"]._VolumeMax, _ScaleSets["default"]._ScaleMode);
	}
	
	return true;
}

int misound::Audio::getWaves(string dir, vector<string> &waves)
{
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dir.c_str())) == NULL) 
	{
		cout << "Error(" << errno << ") opening " << dir << endl;
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) 
	{
		if (string(dirp->d_name).find(".wav") != std::string::npos)
		{
			printf("found wave %s in %s\n", dirp->d_name, dir.c_str());
			waves.push_back(string(dirp->d_name));
		}
	}
	closedir(dp);
	return 0;
}

std::string& misound::Audio::replace(std::string& s, const std::string& from, const std::string& to)
{
	if (!from.empty())
		for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size())
			s.replace(pos, from.size(), to);
	return s;
}

void misound::Audio::getVolumeSettings(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		return;
	}

	Json::CharReaderBuilder readerBuilder;
	Json::Value root;
	std::string errs;

	// Dateiinhalt in einen Stringstream einlesen
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string jsonString = buffer.str();

	if (!Json::parseFromStream(readerBuilder, buffer, &root, &errs)) 
	{
		std::cerr << "Fehler beim Parsen des JSON: " << errs << std::endl;
		return;
	}

	for (const auto& key : root.getMemberNames()) 
	{
		if (root[key].isObject())
		{
			std::string name;
			misound::VolumeScaleSet volumeSet;
			for (const auto& subKey : root[key].getMemberNames()) {
				if (subKey == "name")
				{
					if (root[key][subKey].isString())
					{
						name = root[key][subKey].asString();
						volumeSet._FromFile = true;
					}
				}
				if (subKey == "min")
				{
					if (root[key][subKey].isDouble())
					{
						volumeSet._VolumeMin = root[key][subKey].asDouble();
					}
				}
				if (subKey == "max")
				{
					if (root[key][subKey].isDouble())
					{
						volumeSet._VolumeMax = root[key][subKey].asDouble();
					}
				}
				if (subKey == "mode")
				{
					if (root[key][subKey].isString())
					{
						std::string mode = root[key][subKey].asString();
						if (mode == "linear")
						{
							volumeSet._ScaleMode = misound::VolumeScaleMode::linear;
						}
						if (mode == "log")
						{
							volumeSet._ScaleMode = misound::VolumeScaleMode::log;
						}
					}
				}
				
			}
			if (name != "")
			{
				_ScaleSets[name] = volumeSet;
			}
			
		}
	}

}
