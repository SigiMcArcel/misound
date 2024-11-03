#pragma once
#include <alsa/asoundlib.h>
#include <string>
#include <math.h>
#include "miSoundDebug.h"

namespace misound
{
	#define MAX_LINEAR_DB_SCALE 24

	#define VOLDEBUG(y,...) miSoundDebug::miDebug("mivolume",y, __VA_ARGS__)

	typedef enum class VolumeScaleMode_t
	{
		linear,
		log,
	}VolumeScaleMode;

	class AlsaVolume
	{
	private:
		double _VolumeValue;
		snd_mixer_t* _MixerHandle;
		snd_mixer_elem_t* _AlsaElem;
		std::string _SoundCard;
		std::string _Hw;
		bool _Stop;
		int _Error = 0;
		double _VolumeMin;
		double _VolumeMax;
		VolumeScaleMode _ScaleMode;

		void init();
		long linearToAlsaVolume(int linearVolume);
		long linearToAlsaVolumeLog(int linearVolume);
		bool getUnderlyingHardware();
		bool configVolume();
		bool openVolume();
		bool closeVolume();
		bool setVolumeIntern(double volAsPercent, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode);

	public:
		AlsaVolume(const std::string& soundCard,double volumeMin,double volumeMax, misound::VolumeScaleMode scaleMode)
			: _VolumeValue(0)
			, _MixerHandle(nullptr)
			, _AlsaElem(nullptr)
			, _SoundCard(soundCard)
			, _Hw("")
			, _Stop(false)
			, _Error(0)
			, _VolumeMin(volumeMin)
			, _VolumeMax(volumeMax)
			, _ScaleMode(scaleMode)
		{
			init();
		}

		AlsaVolume(const std::string& soundCard, double volumeMin)
			: _VolumeValue(0)
			, _MixerHandle(nullptr)
			, _AlsaElem(nullptr)
			, _SoundCard(soundCard)
			, _Hw("")
			, _Stop(false)
			, _Error(0)
			, _VolumeMin(volumeMin)
			, _ScaleMode(VolumeScaleMode::log)
		{
			init();
		}

		~AlsaVolume()
		{
			closeVolume();
		}

		bool setSoundcard(const std::string& soundCard);
		bool setSoundcard(const std::string& soundCard, double volumeMin, double volumeMax, misound::VolumeScaleMode scaleMode);
		bool setVolume(double volumePercent);
	};
}

