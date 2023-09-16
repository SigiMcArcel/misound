#pragma once
#include <alsa/asoundlib.h>
#include <sched.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <mutex>

class cAlsaStream
{
	snd_pcm_t* _alsaHandle;
	unsigned int _rate;
	unsigned int _channels;
	unsigned long _framesPerBuffer;
	unsigned char*  _waveData;
	unsigned long	_waveDataSize;
	int _playThread_id;
	pthread_t _playThread;
	bool _playing;
	bool _loop;
	bool _ready;
	bool _play;


	
	bool open(unsigned int rate, unsigned int channels)
	{
		int pcm = 0;
		unsigned int tmp = 0;
		snd_pcm_hw_params_t *params;
		
		
		/* Open the PCM device in playback mode */
		pcm = snd_pcm_open(&_alsaHandle, "plug:dmix", SND_PCM_STREAM_PLAYBACK, 0);
		if(pcm < 0)
		{
			printf("ERROR: Can't open \"%s\" PCM device. %s\n","plug:dmixer", snd_strerror(pcm));
			return false;
		}
			
		


		/* Allocate parameters object and fill it with default values*/
		snd_pcm_hw_params_alloca(&params);

		snd_pcm_hw_params_any(_alsaHandle, params);

		/* Set parameters */
		pcm = snd_pcm_hw_params_set_access(_alsaHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
			
		if (pcm < 0)
		{
			printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));
			close();
			return false;
		}
			
		pcm = snd_pcm_hw_params_set_format(_alsaHandle, params, SND_PCM_FORMAT_S16_LE);
		if (pcm < 0)
		{
			printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));
			close();
			return false;
		}
			

		pcm = snd_pcm_hw_params_set_channels(_alsaHandle, params, channels);
		if (pcm < 0)
		{
			printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));
			close();
			return false;
		}
			

		pcm = snd_pcm_hw_params_set_rate_near(_alsaHandle, params, &rate, 0);
		if (pcm < 0)
		{
			printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));
			close();
			return false;
		}
		snd_pcm_uframes_t uf = 2048;
		snd_pcm_hw_params_set_buffer_size_near(_alsaHandle, params,
			&uf);
		/* Write parameters */
		pcm = snd_pcm_hw_params(_alsaHandle, params);
		if (pcm < 0)
		{
			printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));
			close();
			return false;
		}
			
		//snd_pcm_nonblock(_alsaHandle, 1);
			
		/* Resume information */
		printf("PCM name: '%s'\n", snd_pcm_name(_alsaHandle));

		printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(_alsaHandle)));

		snd_pcm_hw_params_get_channels(params, &tmp);
		printf("channels: %i ", tmp);

		if (tmp == 1)
			printf("(mono)\n");
		else if (tmp == 2)
			printf("(stereo)\n");
		

		snd_pcm_hw_params_get_rate(params, &tmp, 0);
		printf("rate: %d bps\n", tmp);



		/* Allocate buffer to hold single period */
		snd_pcm_hw_params_get_period_size(params, &_framesPerBuffer, 0);



		snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
		return true;
	}

	
	void close()
	{
		snd_pcm_drop(_alsaHandle);
		snd_pcm_close(_alsaHandle);
	}

	void  startThread()
	{
		_playThread_id = pthread_create(&_playThread, NULL, cAlsaStream::PlayProc, this);
		pthread_setname_np(_playThread, "stream");
		
	}

	void  stopThread()
	{
		pthread_join(_playThread, NULL);

	}

	
	
public:
	cAlsaStream(unsigned int rate, unsigned int channels)
		:_rate(rate)
		, _channels(channels)
		, _framesPerBuffer(0)
		, _playThread_id(0)
		, _playThread()
		, _playing(false)
		, _loop(false)
		, _play(false)

	{

	}

	~cAlsaStream()
	{
		
		_play = false;
	}

	bool playWave(unsigned char* pWave, const unsigned long framesCnt,bool loop)
	{
		if (pWave == NULL)
		{
			return false;
		}
		_loop = loop;
		_playing = true;
		_play = true;
		_waveDataSize = framesCnt * _channels * SND_PCM_FORMAT_S16_LE;
		_waveData = pWave;
		
		if (!open(_rate, _channels))
		{
			return false;
		}
		startThread();
		return true;
	}

	void stopWave()
	{
		if (_playing)
		{
			_play = false;
			stopThread();
			
		}
		
	}

	bool playing()
	{
		return _playing;
	}

	

	static void* PlayProc(void *p)
	{
		if (p == NULL)
		{
			return 0;
		}
		cAlsaStream* stream = (cAlsaStream*)p;
		snd_pcm_sframes_t pcm = 0;
		unsigned long position = 0, nextPosition = 0;
		unsigned long framesToWrite = stream->_framesPerBuffer;
		
		/*struct sched_param params;
		pthread_t this_thread = pthread_self();
		params.__sched_priority = sched_get_priority_max(SCHED_FIFO);
		pthread_setschedparam(this_thread, SCHED_FIFO, &params);*/
		
		do
		{
			nextPosition += (stream->_framesPerBuffer * stream->_channels * 2);
			//Ist die Anzahl verbliebener Frames im wave kleiner als die Anzahl frames per buffer dann:
			if (nextPosition >= stream->_waveDataSize)
			{
				framesToWrite = (stream->_waveDataSize - position) / (stream->_channels * 2);
			}
			pcm = snd_pcm_writei(stream->_alsaHandle
				, &stream->_waveData[position]
				, 	static_cast<snd_pcm_uframes_t>(framesToWrite));
			if (pcm == -EPIPE) {
				if (snd_pcm_prepare(stream->_alsaHandle)<0)
				{
					printf("super error\n");
				}
			}
	
			position += (stream->_framesPerBuffer * stream->_channels * 2);

			
			if (stream->_loop)
			{
				if (position >= stream->_waveDataSize)
				{
					position = 0;
					nextPosition = 0;
					framesToWrite = stream->_framesPerBuffer;
				}
			}
				
		} while ((position <= stream->_waveDataSize) &&  stream->_play);

		stream->close();
		stream->_playing = false;
		return NULL;
	}
};

class cAlsaVolume
{
private:
	int _volumeThread_id;
	pthread_t _volumeThread;
	int _lastVolumeValue;
	int _volumeValue;
	long _volumeValueMin;
	long _volumeValueMax;
	int _intervall;
	snd_mixer_t *_mixerHandle;
	snd_mixer_elem_t* _AlsaElem;
	bool _stop;
	int _error = 0;

	void  startThread()
	{
		_stop = false;
		_volumeThread_id = pthread_create(&_volumeThread, NULL, cAlsaVolume::VolumeProc, this);
		pthread_setname_np(_volumeThread, "volume");
	}

	void  stopThread()
	{
		_stop = true;
		if (_volumeThread != 0)
		{
			pthread_join(_volumeThread, NULL);
		}
		
	}

	bool configVolume()
	{
		int result = 0;
		
		snd_mixer_selem_id_t *sid = NULL;
		const char *card = "hw:0";
		const char *selem_name = "Digital";

		if (_mixerHandle == NULL)
		{
			return false;
		}

		result = snd_mixer_attach(_mixerHandle, card);
		if (result < 0)
		{
			return false;
		}
		result = snd_mixer_selem_register(_mixerHandle, NULL, NULL);
		if (result < 0)
		{
			return false;
		}
		result = snd_mixer_load(_mixerHandle);
		if (result < 0)
		{
			return false;
		}

		snd_mixer_selem_id_alloca(&sid);
		snd_mixer_selem_id_set_index(sid, 0);
		snd_mixer_selem_id_set_name(sid, selem_name);
		_AlsaElem = snd_mixer_find_selem(_mixerHandle, sid);
		if (_AlsaElem == NULL)
		{
			return false;
		}

		if (snd_mixer_selem_get_playback_volume_range(_AlsaElem, &_volumeValueMin, &_volumeValueMax) < 0)
		{
			return false;
		}
		return true;
	}

	bool openVolume(int interval)
	{
		int result = snd_mixer_open(&_mixerHandle, 0);
		if (result < 0)
		{
			_mixerHandle = NULL;
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
		if (_mixerHandle)
		{
			snd_mixer_close(_mixerHandle);
			_mixerHandle = NULL;
			return true;
		}
		return false;
	}

public:
	cAlsaVolume(unsigned int intervall)
		: _volumeThread_id(0)
		, _volumeThread()
		, _lastVolumeValue(-1)
		, _volumeValue(0)
		, _volumeValueMin(0)
		, _volumeValueMax(0)
		, _intervall(intervall)
		, _mixerHandle(NULL)
		, _AlsaElem(NULL)
		, _stop(false)
		, _error(0)

	{
		_error = 0;
		if (!openVolume(_intervall))
		{
			_error = -1;
		}
	}

	~cAlsaVolume()
	{
		closeVolume();
	}

	bool setVolume(int volume)
	{
		if (_error < 0)
		{
			return false;
		}
		_volumeValue = volume;
		return true;
	}

	static void* VolumeProc(void *p)
	{
		int result = 0;
		if (p == NULL)
		{
			return NULL;
		}
		cAlsaVolume* volume = (cAlsaVolume*)p;
		while (!volume->_stop)
		{
			if (volume->_volumeValue != volume->_lastVolumeValue)
			{
				result = snd_mixer_selem_set_playback_volume_all(volume->_AlsaElem, volume->_volumeValue * volume->_volumeValueMax / 100);
				if (result < 0)
				{
					volume->_error = result;
				}
			}
			volume->_lastVolumeValue = volume->_volumeValue;
			usleep(volume->_intervall * 1000);
		}
		
		//printf("vol = %d,max = %d vol = %d\r\n", volume, max, volume * max / 100);
		return NULL;
	}
};

class cAlsaMidi
{
	typedef char* MidiDataRaw_p;
	typedef snd_rawmidi_t MidiHandle_t;

	typedef struct midiMessage_t
	{
		char MessageRaw[20];
		size_t Len;
	} MidiMessage;



private:
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
		_MidiInputThread_id = pthread_create(&_MidiInputThread, NULL, cAlsaMidi::MidiInputProc, this);
		_MidiOutputThread_id = pthread_create(&_MidiOutputThread, NULL, cAlsaMidi::MidiOutputProc, this);
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
	cAlsaMidi(const std::string device)
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
		int mode = SND_RAWMIDI_SYNC;
		if ((status = snd_rawmidi_open(NULL, &_MidiHandle, _Device.c_str(), mode)) < 0) {
			printf("Problem opening MIDI output: %s", snd_strerror(status));
			return  false;
		}
		return true;
	}

	void Close()
	{
		snd_rawmidi_close(_MidiHandle);
	}

	bool Read(midiMessage_t* data)
	{
		ssize_t status;
		if ((status = snd_rawmidi_read(_MidiHandle, data->MessageRaw, data->Len)) < 0) {
			printf("Problem writing to MIDI output: %s", snd_strerror(static_cast<int>(status)));
			return false;
		}
		return true;
	}

	bool Write(midiMessage_t* data)
	{
		ssize_t status;
		if ((status = snd_rawmidi_write(_MidiHandle, data, 3)) < 0) {
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
		cAlsaMidi* alsaMidi = (cAlsaMidi*)p;
		while (!alsaMidi->_Stop)
		{
			std::lock_guard<std::mutex> guard(messageMutex);
			if(alsaMidi->Read(&data))
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
		cAlsaMidi* alsaMidi = (cAlsaMidi*)p;
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