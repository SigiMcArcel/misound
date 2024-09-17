#pragma once
#include <fstream>
#include <string>
#include <sndfile.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "Alsa.h"

using namespace std;
namespace misound
{
	typedef enum WaveType_t
	{
		file,
		sine,
		square,
		triangle,
		sawTooth
	}WaveType;

	class Wave
	{
		SNDFILE* _waveFile;
	protected:
		bool			_error;
		unsigned char* _waveData;
		SF_INFO			_info;
		bool            _loop;
		unsigned long   _samplePosition;
		unsigned long   _waveSize;
		bool _playing;
		string _name;
		string _path;
		misound::SoundFormat _format;
		misound::AlsaStream* _stream;

	public:

		Wave(const string& path, const string& name, bool loop);
		Wave(const string& name, bool loop);
		Wave();
		Wave(const Wave& other);


		~Wave();

		void play();
		void stop();
		bool changeSoundcard(const std::string soundcard);

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
}

