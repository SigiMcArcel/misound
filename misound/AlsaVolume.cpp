#include "AlsaVolume.h"
#include <algorithm>

namespace misound
{

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

	bool AlsaVolume::setVolumeIntern(double volAsPercent)
	{
		//long alsaVolume = linearToAlsaVolumeLog(volAsPercent);
		long min, max;
		double dMin, dMax;
		snd_mixer_selem_get_playback_volume_range(_AlsaElem, & min, & max);

		dMax = static_cast<double>(max);
		dMin = static_cast<double>(min);
		// Berechne den skalierter Wert basierend auf min und max
		double offsetAlsa = dMin + (_VolumeOffset * (dMax - dMin) / 100.0);
		
		//double scaledVolume = (dMin + offsetAlsa) + (volAsPercent * (dMax - (dMin + offsetAlsa)) / 100.0);
		double scaledVolume =log10(1 + 9 * volAsPercent / 100.0)* (dMax - (dMin + offsetAlsa)) + (dMin + offsetAlsa);
		//double offsetPercent = std::max(0.0, volAsPercent - _VolumeOffset); // Beispiel: Startet bei 34% Potentiometerwert
		//double scaledVolume = offsetPercent / (100.0 - offsetPercent) * (dMax - dMin) + dMin; // Skaliert von 34% bis 100% auf Lautstärkebereich
		
		
		
		printf("percent %f scaled %f dmax = %f dmin = %f scaled int %d offsetAlsa %f\n", 
			volAsPercent, scaledVolume, dMin, dMax, static_cast<long>(scaledVolume), offsetAlsa);
		if (snd_mixer_selem_set_playback_volume_all(_AlsaElem, static_cast<long>(scaledVolume)) < 0) 
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
			if (!getUnderlyingHardware())
			{
				return false;
			}
			openVolume();
		}

		return true;
	}

	bool misound::AlsaVolume::setVolume(double volumePercent)
	{
		if (_Error < 0)
		{
			printf("setVolume error \n");
			return false;
		}
		setVolumeIntern(volumePercent);
		return true;
	}
}
