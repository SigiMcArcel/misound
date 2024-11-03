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

	long misound::AlsaVolume::linearToAlsaVolume(int linearVolume)
	{
		if (linearVolume < 0) linearVolume = 0;
		if (linearVolume > 100) linearVolume = 100;

		// Logarithmische Skalierung: 0-100% -> -60 dB bis 0 dB
		// Hier kannst du die minimale dB anpassen, je nach Hardware
		double minDb = -60.0;
		double maxDb = 0.0;

		// Berechne den dB-Wert basierend auf der linearen Eingabe
		double db = minDb + (linearVolume / 100.0) * (maxDb - minDb);

		// Konvertiere dB in eine für ALSA geeignete Lautstärke (0 - 100%)
		// Dies hängt vom Mixer-Element ab, hier als Beispiel 0-100
		// In der Praxis musst du die maximale Lautstärke des Mixers abrufen
		// und entsprechend skalieren.
		return static_cast<long>((db / 100.0) * 100); // Einfaches Beispiel
	}

	// Funktion zur Berechnung des logaritmischen Lautstärkewerts
	long misound::AlsaVolume::linearToAlsaVolumeLog(int linearVolume)
	{
		if (linearVolume <= 0)
			return 0;

		// Logarithmische Skalierung: 0-100% -> 0 - maxVolume
		// Verwende eine exponentielle Funktion für die Skalierung
		// Beispiel: y = a * (exp(b * x) - 1)
		// Dabei ist y der skalierte Wert und x die lineare Eingabe (0-100)

		// Parameter für die Skalierung (kann angepasst werden)
		double a = 1.0;
		double b = 0.035; // Anpassbar je nach gewünschter Krümmung

		double scaled = a * (exp(b * linearVolume) - 1);

		// Normiere den Wert auf 0 - 100
		double normalized = (scaled / (exp(b * 100) - 1)) * 100.0;

		// Runde auf den nächsten ganzen Wert
		return static_cast<long>(normalized + 0.5);
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

	bool AlsaVolume::setVolumeIntern(double volAsPercent, double volumeMax, double volumeMin, misound::VolumeScaleMode scaleMode)
	{
		long min, max;
		double dMin, dMax;
		double dScaledVolume = 0.0;

		if ((volAsPercent < 0.0) || (volAsPercent > 100.0))
		{
			return false;
		}
		if ((volumeMax < 0.0) || (volumeMax > 100.0))
		{
			return false;
		}
		if ((volumeMin < 0.0) || (volumeMin > 100.0))
		{
			return false;
		}

		snd_mixer_selem_get_playback_volume_range(_AlsaElem, & min, & max);

		dMax = static_cast<double>(max);
		dMin = static_cast<double>(min);
		// Berechne den Offset für ALSA basierend auf VolumeMin und VolumeMax
		double offsetAlsa = dMin + (volumeMin * (dMax - dMin) / 100.0);  // _VolumeMin und _VolumeMax als Prozentsatz
		double alsaMaxScaled = dMin + (volumeMax * (dMax - dMin) / 100.0);

		// Überprüfe, ob volAsPercent innerhalb des gültigen Bereichs liegt
		if (volAsPercent < 0.1) volAsPercent = 0.0;
		if (volAsPercent > 100.0) volAsPercent = 100.0;

		// Berechnung des skalierten ALSA-Werts basierend auf logarithmischer oder linearer Skalierung
		if (scaleMode == misound::VolumeScaleMode::log)
		{
			dScaledVolume = log10(1 + 9 * volAsPercent / 100.0) * (alsaMaxScaled - offsetAlsa) + offsetAlsa;
		}
		else  // lineare Skalierung
		{
			dScaledVolume = offsetAlsa + ((volAsPercent / 100.0) * (alsaMaxScaled - offsetAlsa));
		}
		
		if (snd_mixer_selem_set_playback_volume_all(_AlsaElem, static_cast<long>(dScaledVolume)) < 0)
		{
			printf("Alsa.h Volume::setVolumeIntern failed\n");
			return false;
		}
		return true;
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
}
