#pragma once
#include <alsa/asoundlib.h>
#include <sched.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <mutex>
#include <math.h>
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

		bool open(unsigned int rate, unsigned int channels)
		{
			int pcm = 0;
			unsigned int tmp = 0;
			snd_pcm_hw_params_t* params;
			_snd_pcm_format usedFormat = SND_PCM_FORMAT_UNKNOWN;

			printf("Alsa stream Card %s :open channels = %d rate = %d\n", _Soundcard.c_str(), _Channels, rate);
			/* Open the PCM device in playback mode */

			pcm = snd_pcm_open(&_AlsaHandle, _Soundcard.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
			if (pcm < 0)
			{
				printf("ERROR: Can't open \"%s\" PCM device. %s\n", _Soundcard.c_str(), snd_strerror(pcm));
				return false;
			}

			/* Allocate parameters object and fill it with default values*/
			snd_pcm_hw_params_alloca(&params);

			snd_pcm_hw_params_any(_AlsaHandle, params);

			/* Set parameters */
			pcm = snd_pcm_hw_params_set_access(_AlsaHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

			if (pcm < 0)
			{
				printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));
				close();
				return false;
			}
			if (_SoundFormat == SoundFormat::SoundFormat_S16_LE)
			{
				usedFormat = SND_PCM_FORMAT_S16_LE;
			}
			else if (_SoundFormat == SoundFormat::SoundFormat_S24_LE)
			{
				usedFormat = SND_PCM_FORMAT_S24_3LE;
			}
			else
			{
				printf("ERROR: SoundFormat not supported\n");
				close();
				return false;
			}

			pcm = snd_pcm_hw_params_set_format(_AlsaHandle, params, usedFormat);
			if (pcm < 0)
			{
				printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));
				close();
				return false;
			}

			pcm = snd_pcm_hw_params_set_channels(_AlsaHandle, params, channels);
			if (pcm < 0)
			{
				printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));
				close();
				return false;
			}

			pcm = snd_pcm_hw_params_set_rate_near(_AlsaHandle, params, &rate, 0);
			if (pcm < 0)
			{
				printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));
				close();
				return false;
			}

			snd_pcm_uframes_t uf = 2048;
			snd_pcm_hw_params_set_buffer_size_near(_AlsaHandle, params,
				&uf);

			/* Write parameters */
			pcm = snd_pcm_hw_params(_AlsaHandle, params);
			if (pcm < 0)
			{
				printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));
				close();
				return false;
			}

			//snd_pcm_nonblock(_alsaHandle, 1);

			/* Resume information */
			printf("PCM name: '%s'\n", snd_pcm_name(_AlsaHandle));

			printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(_AlsaHandle)));

			snd_pcm_hw_params_get_channels(params, &tmp);
			printf("channels: %i ", tmp);

			if (tmp == 1)
				printf("(mono)\n");
			else if (tmp == 2)
				printf("(stereo)\n");

			snd_pcm_hw_params_get_rate(params, &tmp, 0);
			printf("rate: %d bps\n", tmp);

			/* Allocate buffer to hold single period */
			snd_pcm_hw_params_get_period_size(params, &_FramesPerBuffer, 0);

			snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
			return true;
		}

		void close()
		{
			snd_pcm_drop(_AlsaHandle);
			snd_pcm_close(_AlsaHandle);
		}

		void  startThread()
		{
			struct sched_param params;
			_PlayThread_id = pthread_create(&_PlayThread, NULL, AlsaStream::PlayProc, this);
			pthread_setname_np(_PlayThread, "stream");
			params.__sched_priority = 50;
			pthread_setschedparam(_PlayThread, SCHED_FIFO, &params);

		}

		void  stopThread()
		{
			pthread_join(_PlayThread, NULL);

		}

		bool playWaveIntern()
		{
			if (!open(_Rate, _Channels))
			{
				return false;
			}
			startThread();
			//_Timer.Start(10);
			return true;
		}

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
			printf("Alsatream Card %sconstructor channels %d rate = %d\n", _Soundcard.c_str(), _Channels, _Rate);
		}

		virtual ~AlsaStream()
		{
			_Play = false;
		}

		virtual void changeSoundcard(const std::string& device)
		{
			setSoundcard(device);
		}

		void setup(unsigned int rate, unsigned int channels, SoundFormat format, const std::string& soundCard)
		{
			_Rate = rate;
			_Channels = channels;
			_SoundFormat = format;
			_Soundcard = soundCard;
		}

		bool playWave(misound::WaveData_t wave, const unsigned long framesCnt, bool loop)
		{
			_Loop = loop;
			_Playing = true;
			_Play = true;
			_WaveDataSize = framesCnt * _Channels * static_cast<unsigned long>(_SoundFormat);
			_WaveData = wave;

			return playWaveIntern();
		}

		void stopWave()
		{
			if (_Playing)
			{
				_Play = false;
				//stopThread();

			}

		}

		bool setSoundcard(const std::string soundcard)
		{
			int timeout = 100;
			_Soundcard = soundcard;
			if (_Playing)
			{
				_Play = false;
				while (_Playing)
				{
					usleep(10000);
					timeout--;
					if (timeout <= 0)
					{
						printf("stop wave timeOut\n");
						return false;
					}
				}
				_Play = true;
				_Playing = true;
				return playWaveIntern();
			}
			return true;
		}

		bool playing()
		{
			return _Playing;
		}

		static void* PlayProc(void* p)
		{
			if (p == NULL)
			{
				return 0;
			}
			AlsaStream* stream = (AlsaStream*)p;
			snd_pcm_sframes_t pcm = 0;
			unsigned long position = 0, nextPosition = 0;
			unsigned long framesToWrite = stream->_FramesPerBuffer;

			/*struct sched_param params;
			pthread_t this_thread = pthread_self();
			params.__sched_priority = sched_get_priority_max(SCHED_FIFO);
			pthread_setschedparam(this_thread, SCHED_FIFO, &params);*/

			do
			{
				nextPosition += (stream->_FramesPerBuffer * stream->_Channels * static_cast<unsigned long>(stream->_SoundFormat));
				//Ist die Anzahl verbliebener Frames im wave kleiner als die Anzahl frames per buffer dann:
				if (nextPosition >= stream->_WaveDataSize)
				{
					framesToWrite = (stream->_WaveDataSize - position) / (stream->_Channels * static_cast<unsigned long>(stream->_SoundFormat));
				}
				pcm = snd_pcm_writei(stream->_AlsaHandle
					, &stream->_WaveData[position]
					, static_cast<snd_pcm_uframes_t>(framesToWrite));
				if (pcm == -EPIPE) {
					if (snd_pcm_prepare(stream->_AlsaHandle) < 0)
					{
						printf("super error\n");
					}
				}

				position += (stream->_FramesPerBuffer * stream->_Channels * static_cast<unsigned long>(stream->_SoundFormat));

				if (stream->_Loop)
				{
					if (position >= stream->_WaveDataSize)
					{
						position = 0;
						nextPosition = 0;
						framesToWrite = stream->_FramesPerBuffer;
					}
				}

			} while ((position <= stream->_WaveDataSize) && stream->_Play);

			stream->close();
			stream->_Playing = false;
			return NULL;
		}
	};



	class AlsaVolume
	{
	private:
		int _VolumeThread_id;
		pthread_t _VolumeThread;
		double _VolumeValue;
		int _Intervall;
		snd_mixer_t* _MixerHandle;
		snd_mixer_elem_t* _AlsaElem;
		std::string _SoundCard;
		std::string _Hw;
		bool _Stop;
		int _Error = 0;

		/* from alsa-utils/alsamixer/volume_mapping.c
*
* The mapping is designed so that the position in the interval is proportional
* to the volume as a human ear would perceive it (i.e., the position is the
* cubic root of the linear sample multiplication factor).  For controls with
* a small range (24 dB or less), the mapping is linear in the dB values so
* that each step has the same size visually.  Only for controls without dB
* information, a linear mapping of the hardware volume register values is used
* (this is the same algorithm as used in the old alsamixer).
*
* When setting the volume, 'dir' is the rounding direction:
* -1/0/1 = down/nearest/up.
*/
		static bool use_linear_dB_scale(long dBmin, long dBmax)
		{
			return dBmax - dBmin <= MAX_LINEAR_DB_SCALE * 100;
		}

		static long lrint_dir(double x, int dir)
		{
			if (dir > 0)
				return lrint(ceil(x));
			else if (dir < 0)
				return lrint(floor(x));
			else
				return lrint(x);
		}

		// from alsamixer/volume-mapping.c, sets volume in line with human perception
		static int volume_normalized_set(snd_mixer_elem_t* elem, double volume, int dir)
		{
			long min, max, value;
			double min_norm;
			int err;

			err = snd_mixer_selem_get_playback_dB_range(elem, &min, &max);
			if (err < 0 || min >= max)
			{
				err = snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
				if (err < 0)
					return err;

				value = lrint_dir(volume * static_cast<double>(max - min), dir) + min;
				return snd_mixer_selem_set_playback_volume_all(elem, value);
			}



			if (use_linear_dB_scale(min, max))
			{
				value = lrint_dir(volume * static_cast<double>(max - min), dir) + min;
				return snd_mixer_selem_set_playback_dB_all(elem, value, dir);
			}

			if (min != SND_CTL_TLV_DB_GAIN_MUTE)
			{
				min_norm = pow(10, static_cast<double>(min - max) / 6000.0);
				volume = volume * (1 - min_norm) + min_norm;
			}

			value = lrint_dir(6000.0 * log10(volume), dir) + max;
			return snd_mixer_selem_set_playback_dB_all(elem, value, dir);
		}

		bool getUnderlyingHardware() {
			
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

		void  startThread()
		{
			_Stop = false;
			_VolumeThread_id = pthread_create(&_VolumeThread, NULL, AlsaVolume::VolumeProc, this);
			pthread_setname_np(_VolumeThread, "volume");
			printf("startThread id = %d\n", _VolumeThread_id);
		}

		void  stopThread()
		{
			_Stop = true;
			if (_VolumeThread != 0)
			{
				pthread_join(_VolumeThread, NULL);
			}
		}

		bool configVolume()
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

		bool openVolume(int interval)
		{
			int result = snd_mixer_open(&_MixerHandle, 0);
			if (result < 0)
			{
				_MixerHandle = NULL;
				return false;
			}
			if (configVolume())
			{
				startThread();
				return true;
			}
			closeVolume();
			return false;
		}
		bool closeVolume()
		{
			stopThread();
			if (_MixerHandle)
			{
				snd_mixer_close(_MixerHandle);
				_MixerHandle = NULL;
				return true;
			}
			return false;
		}

	public:
		AlsaVolume(unsigned int intervall,const std::string& soundCard)
			: _VolumeThread_id(0)
			, _VolumeThread()
			, _VolumeValue(0)
			, _Intervall(intervall)
			, _MixerHandle(nullptr)
			, _AlsaElem(nullptr)
			, _SoundCard(soundCard)
			, _Hw("")
			, _Stop(false)
			, _Error(0)

		{
			_Error = 0;
			if (!getUnderlyingHardware())
			{
				_Error = -1;
				return;
			}
			if (!openVolume(_Intervall))
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
			volume_normalized_set(_AlsaElem, 0, 0);
		}

		~AlsaVolume()
		{
			closeVolume();
		}

		bool setSoundcard(const std::string& soundCard)
		{
			printf("Alsa.h Volume::setSoundcard %s\n", soundCard.c_str());
			_SoundCard = soundCard;
			if (_MixerHandle != nullptr)
			{
				closeVolume();
				if (!getUnderlyingHardware())
				{
					return false;
				}
				openVolume(_Intervall);
			}
			
			return true;
		}

		bool setVolume(double volume)
		{
			if (_Error < 0)
			{
				printf("setVolume error \n");
				return false;
			}
			_VolumeValue = volume;
			return true;
		}

		static void* VolumeProc(void* p)
		{
#if 0
			int last = 0;
#endif
			int current = 0;

			if (p == NULL)
			{
				return NULL;
			}
			AlsaVolume* volume = (AlsaVolume*)p;
			if (volume->_AlsaElem == nullptr)
			{
				return nullptr;
			}
			while (!volume->_Stop)
			{
				current = static_cast<int>(volume->_VolumeValue);
				volume_normalized_set(volume->_AlsaElem, static_cast<double>(current) / 100.0, 0);
#if 0
				while (current > last)
				{
					volume_normalized_set(volume->_AlsaElem, static_cast<double>(last) / 100.0, 0);
					last++;
					usleep(volume->_Intervall * 1000);

				}
				while (current < last)
				{
					volume_normalized_set(volume->_AlsaElem, static_cast<double>(last) / 100.0, 0);
					usleep(volume->_Intervall * 1000);
					last--;
				}
#endif
				usleep(volume->_Intervall * 1000);
			}

			return nullptr;
		}
	};

	class AlsaMidi
	{

	public:
		enum class MidiCommand_e
		{
			NoteOff = 0x08,
			NoteOn = 0x09,
			PolyPressure = 0x0A,
			Controller = 0x0B,
			ProgramChange = 0x0C,
			ChannelPressure = 0x0D,
			PitchWheel = 0x0E
		};

		typedef struct midiMessage_t
		{
			union midiMessage_u
			{
				unsigned char MessageRaw[20];
				struct Message_t
				{
					struct StatusByte_t
					{
						MidiCommand_e Command : 4;
						unsigned char Channel : 4;
					}StatusByte;
					unsigned char Key;
					unsigned char Velocity;
					unsigned char Reserve[17];
				}Message;
			}U;
			size_t Len;
		} MidiMessage;

	private:
		typedef char* MidiDataRaw_p;
		typedef snd_rawmidi_t MidiHandle_t;
		std::string _Device;
		MidiHandle_t* _MidiHandle;
		int _MidiInputThread_id;
		pthread_t _MidiInputThread;
		int _MidiOutputThread_id;
		pthread_t _MidiOutputThread;
		bool _Stop;
		std::vector<midiMessage_t> _MidiInputBuffer;
		std::vector<midiMessage_t> _MidiOutputBuffer;


		void  startThread()
		{
			_Stop = false;
			_MidiInputThread_id = pthread_create(&_MidiInputThread, NULL, AlsaMidi::MidiInputProc, this);
			_MidiOutputThread_id = pthread_create(&_MidiOutputThread, NULL, AlsaMidi::MidiOutputProc, this);
			pthread_setname_np(_MidiInputThread, "midiinproc");
			pthread_setname_np(_MidiOutputThread, "midioutproc");
		}

		void  stopThread()
		{
			_Stop = true;
			if (_MidiInputThread != 0)
			{
				pthread_join(_MidiInputThread, NULL);
			}
			if (_MidiOutputThread != 0)
			{
				pthread_join(_MidiOutputThread, NULL);
			}

		}

	public:

		AlsaMidi(const std::string device)
			:_Device(device)
			, _MidiHandle(nullptr)
		{

		}

		int putMessage(const MidiMessage& message)
		{
			_MidiOutputBuffer.push_back(message);
			return static_cast<int>(_MidiOutputBuffer.size());
		}

		int getMessage(MidiMessage& message)
		{
			if (_MidiInputBuffer.empty())
			{
				return -1;
			}

			message = _MidiInputBuffer.back();
			_MidiInputBuffer.pop_back();
			return static_cast<int>(_MidiInputBuffer.size());
		}

		bool Open()
		{
			int status;
			int mode = SND_RAWMIDI_SYNC | SND_RAWMIDI_NONBLOCK;
			if ((status = snd_rawmidi_open(NULL, &_MidiHandle, _Device.c_str(), mode)) < 0) {
				printf("Problem opening MIDI output: %s", snd_strerror(status));
				return  false;
			}
			snd_rawmidi_nonblock(_MidiHandle, 0);
			return true;
		}

		void Close()
		{
			snd_rawmidi_close(_MidiHandle);
		}

		bool Read(midiMessage_t* data)
		{
			ssize_t status;
			if ((status = snd_rawmidi_read(_MidiHandle, data->U.MessageRaw, data->Len)) < 0) {
				printf("Problem writing to MIDI output: %s", snd_strerror(static_cast<int>(status)));
				return false;
			}
			return true;
		}

		bool Write(midiMessage_t* data)
		{
			ssize_t status;
			if ((status = snd_rawmidi_write(_MidiHandle, data->U.MessageRaw, 3)) < 0) {
				printf("Problem writing to MIDI output: %s", snd_strerror(static_cast<int>(status)));
				return false;
			}
			return true;
		}


		static void* MidiInputProc(void* p)
		{
			std::mutex messageMutex;
			midiMessage_t data;

			if (p == NULL)
			{
				return NULL;
			}
			AlsaMidi* alsaMidi = (AlsaMidi*)p;
			while (!alsaMidi->_Stop)
			{
				std::lock_guard<std::mutex> guard(messageMutex);
				if (alsaMidi->Read(&data))
				{
					alsaMidi->_MidiInputBuffer.push_back(data);
				}
			}
		}

		static void* MidiOutputProc(void* p)
		{
			std::mutex messageMutex;
			midiMessage_t data;

			if (p == NULL)
			{
				return NULL;
			}
			AlsaMidi* alsaMidi = (AlsaMidi*)p;
			while (!alsaMidi->_Stop)
			{
				std::lock_guard<std::mutex> guard(messageMutex);
				while (!alsaMidi->_MidiOutputBuffer.empty())
				{
					data = alsaMidi->_MidiOutputBuffer.back();
					alsaMidi->Write(&data);
					alsaMidi->_MidiOutputBuffer.pop_back();
				}
			}

		}

	};
}