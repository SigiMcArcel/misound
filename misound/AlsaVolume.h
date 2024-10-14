#pragma once
#include <alsa/asoundlib.h>
#include <string>
#include <math.h>

namespace misound
{
	#define MAX_LINEAR_DB_SCALE 24
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
		double _VolumeOffset;

		long linearToAlsaVolume(int linearVolume);
		long linearToAlsaVolumeLog(int linearVolume);
		bool getUnderlyingHardware();
		bool configVolume();
		bool openVolume();
		bool closeVolume();
		bool setVolumeIntern(double volAsPercent);

	public:
		AlsaVolume(const std::string& soundCard,double volumeOffset)
			: _VolumeValue(0)
			, _MixerHandle(nullptr)
			, _AlsaElem(nullptr)
			, _SoundCard(soundCard)
			, _Hw("")
			, _Stop(false)
			, _Error(0)
			, _VolumeOffset(volumeOffset)

		{
			_Error = 0;
			if (!getUnderlyingHardware())
			{
				_Error = -1;
				return;
			}
			if (!openVolume())
			{
				_Error = -1;
				return;
			}
			if (_MixerHandle == nullptr)
			{
				_Error = -1;
				return;
			}
			if (_AlsaElem == nullptr)
			{
				_Error = -1;
				return;
			}
			snd_mixer_selem_set_playback_volume_all(_AlsaElem, 0);
		}

		~AlsaVolume()
		{
			closeVolume();
		}

		bool setSoundcard(const std::string& soundCard);
		bool setVolume(double volumePercent);
	};
}

