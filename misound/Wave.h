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
	private:
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
		misound::AlsaStream _stream;
		std::string _SoundCard;


		const std::string _DefaultSoundCard = "plug:dmix0";

	public:

		Wave(const std::string& path, const std::string& name, const std::string& soundCard, bool loop);
		Wave();
		Wave(const Wave& other);


		~Wave();

		void play();
		void stop();

		bool isPlaying()
		{
			return _stream.playing();
		}

		unsigned long  samplePosition() const;

		unsigned long size() const;

		int error();

		const std::string& getName() const { return _name; }
		const std::string& getPath() const { return _path; }
		bool loop() const { return _loop; }
		void setLoop(bool loop)
		{
			_loop = loop;
		}
		unsigned char* waveData() const { return _waveData; }
		const SF_INFO& info();

		virtual void changeSoundcard(const std::string& device)
		{
			_SoundCard = device;
			_stream.changeSoundcard(_SoundCard);
		}

	};
}

