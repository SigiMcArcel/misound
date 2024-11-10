#pragma once
#include <alsa/asoundlib.h>
#include <string>
#include <math.h>
#include "miSoundDebug.h"

namespace misound
{
	typedef enum class VolumeScaleMode_t
	{
		none,
		percentLogToLinearAlsa,
		percentToAlsa
	}VolumeScaleMode;

	struct VolumeRange
	{
		double Max;
		double Min;
	};

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
		double _VolumeAsPercent;
		double _ScaledVolume;
		double _VolumeCardMax;
		double _VolumeCardMin;

		void init();
		bool getUnderlyingHardware();
		bool configVolume();
		bool openVolume();
		bool closeVolume();
		bool setVolumeIntern(double volume, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode);
		void getCardInfo();
		double transformLogToLinear(double volume, double volumeMax,double volumeMin);
		double scale(double volume, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode);
		
		

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
			, _VolumeAsPercent(0.0)
			, _ScaledVolume(0.0)
			, _VolumeCardMax(0.0)
			, _VolumeCardMin(0.0)
		{
			init();
		}

		AlsaVolume(const std::string& soundCard)
			: _VolumeValue(0)
			, _MixerHandle(nullptr)
			, _AlsaElem(nullptr)
			, _SoundCard(soundCard)
			, _Hw("")
			, _Stop(false)
			, _Error(0)
			, _VolumeMin(0.0)
			, _VolumeMax(100.0)
			, _ScaleMode(VolumeScaleMode::none)
			, _VolumeAsPercent(0.0)
			, _ScaledVolume(0.0)
			, _VolumeCardMax(0.0)
			, _VolumeCardMin(0.0)
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
		bool setVolume(double volume, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode);
		double scaledVolume()
		{
			return _ScaledVolume;
		}
		VolumeRange getVolumeRange();

	};
}

