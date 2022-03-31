#include "cAudio.h"
#include <inttypes.h>
#include <pthread.h>
#include "cAlsa.h"

static pthread_mutex_t  SHAKE_BufferMutex = PTHREAD_MUTEX_INITIALIZER;
void shakeLock() { pthread_mutex_lock(&SHAKE_BufferMutex); }
void shakeUnlock() { pthread_mutex_unlock(&SHAKE_BufferMutex); }

using namespace std;

cAudio::cAudio()
	:_waves()
	,_volume(20)
{
}


cAudio::~cAudio()
{
}



bool cAudio::playWave(const string& name, bool restart )
{
	
	if (_waves.count(name) == 0)
	{
		return false;
	}
	if (!_waves[name].isPlaying() || restart)
	{
		_waves[name].play();
	}
		
	return true;
}

bool cAudio::playWave(const string& name, bool restart,bool loop)
{
	
	if (_waves.count(name) == 0)
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

bool cAudio::playWave(const int num, bool restart )
{
	int index = 0;
	map<string, cWave>::iterator it;
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

bool cAudio::isPlaying(const string& name)
{
	if (_waves.count(name) == 0)
	{
		return false;
	}
	return _waves[name].isPlaying();
}

bool cAudio::stopWave(const string& name)
{
	if (_waves.count(name) == 0)
	{
		return false;
	}
	_waves[name].stop();
	return true;
}

bool cAudio::stopAllWave()
{
	map<string, cWave>::iterator it;
	for (it = _waves.begin(); it != _waves.end(); it++)
	{	
		it->second.stop();
	}
	return true;
}

bool cAudio::addWave(const cWave& wave)
{
	_waves[wave.getName()] = wave;
	printf( "cAudio::addWave : add Wave %s\n", wave.getName().c_str());
	//cWave w = _waves[wave.getName()];
	return true;
}
 
bool cAudio::addWave(const string & path, const string & name,bool loop = false)
{
	cWave* w = new cWave(path, name, loop);
	if (w->error() > 0)
	{
		return false;
	}
	addWave(*w);
	return true;
}

bool cAudio::addWavesFromFolder(const string & folder,bool loop)
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
		
		cWave* w = new cWave(path, name, loop);
		addWave(*w);
	}
	return true;
}

bool cAudio::setVolume(int volume)
{
	return _volume.setVolume(volume);
}


int cAudio::getWaves(string dir, vector<string> &waves)
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

std::string& cAudio::replace(std::string& s, const std::string& from, const std::string& to)
{
	if (!from.empty())
		for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size())
			s.replace(pos, from.size(), to);
	return s;
}