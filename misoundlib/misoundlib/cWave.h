#pragma once
#include <fstream>
#include <string>
#include <sndfile/sndfile.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cAlsa.h"

using namespace std;

typedef enum WaveType_t
{
	file,
	sine,
	square,
	triangle,
	sawTooth
}WaveType;

class cWave
{
	SNDFILE *		_waveFile;
protected:
	bool			_error;
	unsigned char*   _waveData;
	SF_INFO			_info;
	bool            _loop;
	unsigned long   _samplePosition;
	unsigned long   _waveSize;
	bool _playing;
	string _name;
	cAlsaStream* _stream;

public:
	
	cWave(const string& path,const string& name, bool loop);
	cWave(const string& name, bool loop);
	cWave();
	cWave(const cWave& other);
	

	~cWave();
	
	void play();
	void stop();

	bool isPlaying() const
	{
		return _stream->playing();
	}

	unsigned long  samplePosition() const;
	
   unsigned long size() const;
	
	int error();
	
	const std::string& getName() const { return _name; }
	bool loop() const { return _loop; }
	void setLoop(bool loop)
	{
		_loop = loop;
	}
	unsigned char* waveData() const { return _waveData; }
	const SF_INFO& info();

};

