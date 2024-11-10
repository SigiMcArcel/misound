#include "AlsaVolume.h"
#include <algorithm>

namespace misound
{
	void AlsaVolume::init()
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

	bool misound::AlsaVolume::getUnderlyingHardware() {

		int err;
		const char* device_name = _SoundCard.c_str();
		snd_pcm_t* pcm;

		err = snd_pcm_open(&pcm, device_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
		if (err < 0) {
			fprintf(stderr, "Failed to open audio device '%s': %s\n", device_name, snd_strerror(err));
			return false;
		}

		snd_pcm_info_t* info;
		err = snd_pcm_info_malloc(&info);
		if (err < 0) {
			fprintf(stderr, "Failed to allocate PCM info: %s\n", snd_strerror(err));
			snd_pcm_close(pcm);
			return false;
		}
		err = snd_pcm_info(pcm, info);
		if (err < 0) {
			fprintf(stderr, "Failed to get PCM device info: %s\n", snd_strerror(err));
			snd_pcm_info_free(info);
			snd_pcm_close(pcm);
			return false;
		}

		int card_no = snd_pcm_info_get_card(info);
		if (card_no < 0) {
			fprintf(stderr, "Failed to get PCM card number: %s\n", snd_strerror(card_no));
			snd_pcm_info_free(info);
			snd_pcm_close(pcm);
			return false;
		}

		unsigned dev_no = snd_pcm_info_get_device(info);

		printf("The ALSA path is: /dev/snd/pcmC%dD%up\n", card_no, dev_no);
		snd_pcm_info_free(info);
		snd_pcm_close(pcm);

		_Hw = "";
		_Hw.append("hw:");
		_Hw.append(std::to_string(card_no));

		printf("Underlying hardware device: %s \n", _Hw.c_str());
		return true;
	}

	bool misound::AlsaVolume::configVolume()
	{
		int result = 0;
		snd_mixer_selem_id_t* sid = NULL;
		snd_mixer_elem_t* elem;
		const char* card = _Hw.c_str();
		const char* selem_name = "Digital";

		if (_MixerHandle == NULL)
		{
			return false;
		}

		result = snd_mixer_attach(_MixerHandle, card);

		if (result < 0)
		{
			return false;
		}

		result = snd_mixer_selem_register(_MixerHandle, NULL, NULL);

		if (result < 0)
		{
			return false;
		}

		result = snd_mixer_load(_MixerHandle);

		if (result < 0)
		{
			return false;
		}

		snd_mixer_selem_id_alloca(&sid);
		for (elem = snd_mixer_first_elem(_MixerHandle); elem; elem = snd_mixer_elem_next(elem)) {
			snd_mixer_selem_get_id(elem, sid);
			if (!snd_mixer_selem_is_active(elem))
				continue;

			_AlsaElem = snd_mixer_find_selem(_MixerHandle, sid);
			if (_AlsaElem == NULL)
			{
				printf("Als.h Volume mixer %s not found\n", selem_name);
				return false;
			}

			if (snd_mixer_selem_has_playback_volume(elem) && snd_mixer_selem_has_playback_switch(elem) && !snd_mixer_selem_has_capture_volume(elem)) {
				printf("Simple mixer control found '%s',%i\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
				snd_mixer_selem_id_set_index(sid, 0);
				snd_mixer_selem_id_set_name(sid, snd_mixer_selem_id_get_name(sid));
				return true;
			}
		}

		return true;
	}

	bool misound::AlsaVolume::openVolume()
	{
		int result = snd_mixer_open(&_MixerHandle, 0);
		if (result < 0)
		{
			_MixerHandle = NULL;
			return false;
		}
		if (configVolume())
		{
			return true;
		}
		closeVolume();
		return false;
	}

	bool misound::AlsaVolume::closeVolume()
	{
		if (_MixerHandle)
		{
			snd_mixer_close(_MixerHandle);
			_MixerHandle = NULL;
			return true;
		}
		return false;
	}

	/*
	* This function converts a exp curve (from a log pot thru adc -> going to inverse log) to a mostly linear curve 
	* Is needed for alsa, which accept linear values
	*/
	double AlsaVolume::transformLogToLinear(double volume, double volumeMax,double volumeMin)
	{
		double min = volumeMin;
		if (min == 0)
		{
			min = 1;
		}

		double tmp = std::log(volume) / std::log(volumeMax) * volumeMax;
		if (tmp < volumeMin)
		{
			tmp = volumeMin;
		}
		if (tmp > volumeMax)
		{
			tmp = volumeMax;
		}
		return tmp;
	}

	double AlsaVolume::scale(double volume, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode)
	{
		double result =  0.0;
		double alsaMinScaled = _VolumeCardMin + (volumeMin * (_VolumeCardMax - _VolumeCardMin) / 100.0);
		double alsaMaxScaled = _VolumeCardMin + (volumeMax * (_VolumeCardMax - _VolumeCardMin) / 100.0);

		switch (scaleMode)
		{
		case misound::VolumeScaleMode::percentToAlsa:
		{
			result = alsaMinScaled + ((volume / 100.0) * (alsaMaxScaled - alsaMinScaled));
			break;
		}
		case misound::VolumeScaleMode::percentLogToLinearAlsa:
		{
			result = transformLogToLinear(volume, volumeMax,volumeMin);
			result = alsaMinScaled + ((result / 100.0) * (alsaMaxScaled - alsaMinScaled));
			break;
		}
		case misound::VolumeScaleMode::none:
		{
			result = volume;
			break;
		}

		default:
			break;
		}

		if ((result < _VolumeCardMin) || (result > _VolumeCardMax))
		{
			fprintf(stderr,"AlsaVolume : volume is out of range\n");
		}
		if (result < _VolumeCardMin)
		{
			result = _VolumeCardMin;
		}
		if (result > _VolumeCardMax)
		{
			result = _VolumeCardMax;
		}
		
		return result;
	}

	bool AlsaVolume::setVolumeIntern(double volume, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode)
	{
		if (_VolumeCardMax == 0)
		{
			fprintf(stderr, "AlsaVolume : Error. card maximum is null, card max = %f\n", _VolumeCardMax);
			return false;
		}
		
		_ScaledVolume = scale(volume, volumeMax, volumeMin, scaleMode);

		if (snd_mixer_selem_set_playback_volume_all(_AlsaElem, static_cast<long>(_ScaledVolume)) < 0)
		{
			printf("Alsa.h Volume::setVolumeIntern failed\n");
			return false;
		}
		return true;
	}

	void AlsaVolume::getCardInfo()
	{
		long min, max;
		snd_mixer_selem_get_playback_volume_range(_AlsaElem, &min, &max);
		_VolumeCardMax = static_cast<double>(max);
		_VolumeCardMin = static_cast<double>(min);
	}


	bool misound::AlsaVolume::setSoundcard(const std::string& soundCard)
	{
		_SoundCard = soundCard;
		if (_MixerHandle != nullptr)
		{
			closeVolume();
		}
		if (!getUnderlyingHardware())
		{
			return false;
		}
		openVolume();
		getCardInfo();
		return true;
	}

	bool AlsaVolume::setSoundcard(const std::string& soundCard, double volumeMin, double volumeMax, misound::VolumeScaleMode scaleMode)
	{
		_VolumeMax = volumeMax;
		_VolumeMin = volumeMin;
		_ScaleMode = scaleMode;
		setSoundcard(soundCard);
		setVolumeIntern(_VolumeAsPercent, _VolumeMax, _VolumeMin, _ScaleMode);
		return false;
	}

	bool misound::AlsaVolume::setVolume(double volumePercent)
	{
		if (_Error < 0)
		{
			printf("setVolume error \n");
			return false;
		}
		_VolumeAsPercent = volumePercent;
		setVolumeIntern(_VolumeAsPercent, _VolumeMax, _VolumeMin, _ScaleMode);
		return true;
	}

	bool AlsaVolume::setVolume(double volume, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode)
	{
		return setVolumeIntern(volume, volumeMax, volumeMin, scaleMode);
	}

	VolumeRange AlsaVolume::getVolumeRange()
	{
		VolumeRange range;
		range.Max = _VolumeCardMax;
		range.Max = _VolumeCardMin;

		return range;
	}
	
}
