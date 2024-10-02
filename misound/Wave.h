#pragma once
#include <fstream>
#include <string>
#include <sndfile.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <memory> 
#include "Alsa.h"

using namespace std;
namespace misound
{
	class Wave 
	{	
	private:
		SNDFILE* _WaveFile;
		WaveData_t		_WaveData;
		SF_INFO			_Info;
		bool            _Loop;
		unsigned long   _SamplePosition;
		unsigned long   _WaveSize;
		bool _Playing;
		string _Name;
		string _Path;
		misound::SoundFormat _Format;
		misound::AlsaStream _Stream;
		std::string _SoundCard;
		bool _Error;

		bool setup();


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
			return _Stream.playing();
		}

		unsigned long  samplePosition() const;

		unsigned long size() const;

		int error();

		const std::string& getName() const { return _Name; }
		const std::string& getPath() const { return _Path; }
		bool loop() const { return _Loop; }
		void setLoop(bool loop)
		{
			_Loop = loop;
		}
		WaveData_t waveData() const { return _WaveData; }
		const SF_INFO& info();

		virtual void changeSoundcard(const std::string& device)
		{
			_SoundCard = device;
			_Stream.changeSoundcard(_SoundCard);
		}

	};
}

