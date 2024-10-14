#pragma once
#include <alsa/asoundlib.h>
#include <sched.h>
#include <pthread.h>
#include <string>
#include "miSoundTypes.h"

namespace misound
{
#define MAX_LINEAR_DB_SCALE 24


	class AlsaSoundcardInterface
	{
	public:
		virtual void changeSoundcard(const std::string& device) = 0;
	};

	class AlsaStream
		:public AlsaSoundcardInterface
	{
		snd_pcm_t* _AlsaHandle;
		unsigned int _Rate;
		unsigned int _Channels;
		unsigned long _FramesPerBuffer;
		misound::WaveData_t _WaveData;
		unsigned long	_WaveDataSize;
		int _PlayThread_id;
		pthread_t _PlayThread;
		bool _Playing;
		bool _Loop;
		bool _Ready;
		bool _Play;
		misound::SoundFormat _SoundFormat;
		std::string _Soundcard;
		bool _Valid;
		unsigned long _Position;
		unsigned long _NextPosition;
		unsigned long _PositionStep;

		bool open(unsigned int rate, unsigned int channels);
		void close();
		void  startThread();
		void  stopThread();
		bool playWaveIntern();

	public:
		AlsaStream()
			:_Rate(0)
			, _Channels(0)
			, _FramesPerBuffer(0)
			, _PlayThread_id(0)
			, _PlayThread()
			, _Playing(false)
			, _Loop(false)
			, _Play(false)
			, _SoundFormat(SoundFormat::SoundFormat_NONE)
			, _Soundcard("")
			, _Valid(false)
			, _Position(0)
			, _NextPosition(0)
			, _PositionStep(0)
		{

		}
		AlsaStream(const AlsaStream& other) = default;
		AlsaStream(unsigned int rate, unsigned int channels, SoundFormat format, const std::string& soundCard)
			:_Rate(rate)
			, _Channels(channels)
			, _FramesPerBuffer(0)
			, _PlayThread_id(0)
			, _PlayThread()
			, _Playing(false)
			, _Loop(false)
			, _Play(false)
			, _SoundFormat(format)
			, _Soundcard(soundCard) //"plug:dmix"
			, _Valid(true)
			, _Position(0)
			, _NextPosition(0)
			, _PositionStep(0)

		{
			if ((_Rate == 0)
				|| (_Channels == 0)
				|| (_SoundFormat != SoundFormat::SoundFormat_S16_LE)
				|| (_SoundFormat != SoundFormat::SoundFormat_S24_LE)
				|| (soundCard == "")
				)
			{
				printf("Alsa.h Alsa : Could not create stream");
				_Valid = false;
			}
			//printf("Alsatream Card %sconstructor channels %d rate = %d\n", _Soundcard.c_str(), _Channels, _Rate);
		}

		virtual ~AlsaStream()
		{
			_Play = false;
		}

		virtual void changeSoundcard(const std::string& device);

		void setup(unsigned int rate, unsigned int channels, SoundFormat format, const std::string& soundCard);

		bool playWave(misound::WaveData_t wave, const unsigned long framesCnt, bool loop);

		void stopWave();
		bool setSoundcard(const std::string soundcard);
		bool playing();

		static void* PlayProc(void* p);
	};
}