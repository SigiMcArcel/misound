#include "Audio.h"
#include <inttypes.h>
#include <pthread.h>
#include "AlsaStream.h"
#include <vector>

static pthread_mutex_t  SHAKE_BufferMutex = PTHREAD_MUTEX_INITIALIZER;
void shakeLock() { pthread_mutex_lock(&SHAKE_BufferMutex); }
void shakeUnlock() { pthread_mutex_unlock(&SHAKE_BufferMutex); }

using namespace std;

misound::Audio::Audio()
	:_waves()
	,_volume(_DefaultSoundCard)
	,_SoundCard(_DefaultSoundCard)
{
}

misound::Audio::Audio(const std::string& soundCard)
	:_waves()
	, _volume(soundCard)
	, _SoundCard(soundCard)
{
}

misound::Audio::Audio(const std::string& soundCard, const std::string& rootPath)
	:_waves()
	, _volume(soundCard)
	, _SoundCard(soundCard)
	, _RootPath(rootPath)
{
	
}

misound::Audio::~Audio()
{
}

bool misound::Audio::playWave(const string& name, bool restart )
{
	auto it = _waves.find(name);
	if (it == _waves.end())
	{
		return false;
	}
	if (!_waves[name].isPlaying() || restart)
	{
		_waves[name].play();
	}
		
	return true;
}

bool misound::Audio::playWave(const string& name, bool restart,bool loop)
{
	auto it = _waves.find(name);
	if (it == _waves.end())
	{
		return false;
	}
	_waves[name].setLoop(loop);
	if (!_waves[name].isPlaying() || restart)
	{
		_waves[name].play();
	}

	return true;
}

bool misound::Audio::playWave(const int num, bool restart )
{
	int index = 0;
	map<string, Wave>::iterator it;
	if ((_waves.size() == 0) || (_waves.size() <= static_cast<size_t>(num)))
	{
		return false;
	}
	for (it = _waves.begin(); it != _waves.end(); it++)
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
	auto it = _waves.find(name);
	if (it == _waves.end())
	{
		return false;
	}
	return _waves[name].isPlaying();
}

bool misound::Audio::stopWave(const string& name)
{
	auto it = _waves.find(name);
	if (it == _waves.end())
	{
		return false;
	}
	_waves[name].stop();
	return true;
}

bool misound::Audio::stopAllWave()
{
	map<string, Wave>::iterator it;
	for (it = _waves.begin(); it != _waves.end(); it++)
	{	
		it->second.stop();
	}
	return true;
}

bool misound::Audio::addWave(const Wave& wave)
{
	_waves.insert(std::make_pair(wave.getName(),wave));
	printf( "cAudio::addWave : add Wave %s from %s count %d\n", wave.getName().c_str(), wave.getPath().c_str(),_waves.size());
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
	return _volume.setVolume(volumePercent);
}

bool misound::Audio::changeSoundcard(const std::string soundcard)
{
	map<string, Wave>::iterator it;
	printf("Audio::changeSoundcard %s\n", soundcard.c_str());
	for (it = _waves.begin(); it != _waves.end(); it++)
	{
		it->second.changeSoundcard(soundcard);
	}
	_volume.setSoundcard(soundcard);
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