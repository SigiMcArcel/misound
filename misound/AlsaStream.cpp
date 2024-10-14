#include "AlsaStream.h"

bool misound::AlsaStream::open(unsigned int rate, unsigned int channels)
{
	int pcm = 0;
	unsigned int tmp = 0;
	snd_pcm_hw_params_t* params;
	_snd_pcm_format usedFormat = SND_PCM_FORMAT_UNKNOWN;

	//printf("Alsa stream Card %s :open channels = %d rate = %d\n", _Soundcard.c_str(), _Channels, rate);
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

	snd_pcm_hw_params_get_channels(params, &tmp);
	
	snd_pcm_hw_params_get_rate(params, &tmp, 0);

	/* Allocate buffer to hold single period */
	snd_pcm_hw_params_get_period_size(params, &_FramesPerBuffer, 0);

	snd_pcm_hw_params_get_period_time(params, &tmp, NULL);

	_PositionStep = _FramesPerBuffer * _Channels * static_cast<unsigned long>(_SoundFormat);
	return true;
}

void misound::AlsaStream::close()
{
	snd_pcm_drop(_AlsaHandle);
	snd_pcm_close(_AlsaHandle);
}

void misound::AlsaStream::startThread()
{
	struct sched_param params;
	_PlayThread_id = pthread_create(&_PlayThread, NULL, AlsaStream::PlayProc, this);
	pthread_setname_np(_PlayThread, "stream");
	params.__sched_priority = 50;
	pthread_setschedparam(_PlayThread, SCHED_FIFO, &params);
}

void misound::AlsaStream::stopThread()
{
	pthread_join(_PlayThread, NULL);
}

bool misound::AlsaStream::playWaveIntern()
{
	if (!open(_Rate, _Channels))
	{
		return false;
	}
	startThread();

	return true;
}

void misound::AlsaStream::changeSoundcard(const std::string& device)
{
	setSoundcard(device);
}

void misound::AlsaStream::setup(unsigned int rate, unsigned int channels, SoundFormat format, const std::string& soundCard)
{
	_Rate = rate;
	_Channels = channels;
	_SoundFormat = format;
	_Soundcard = soundCard;
}

bool misound::AlsaStream::playWave(misound::WaveData_t wave, const unsigned long framesCnt, bool loop)
{
	_Loop = loop;
	_Playing = true;
	_Play = true;
	_WaveDataSize = framesCnt * _Channels * static_cast<unsigned long>(_SoundFormat);
	_WaveData = wave;
	_Position = 0;
	_NextPosition = 0;
	return playWaveIntern();
}

void misound::AlsaStream::stopWave()
{
	if (_Playing)
	{
		_Play = false;
	}
}

bool misound::AlsaStream::setSoundcard(const std::string soundcard)
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

bool misound::AlsaStream::playing()
{
	return _Playing;
}

void* misound::AlsaStream::PlayProc(void* p)
{
	if (p == NULL)
	{
		return 0;
	}
	AlsaStream* stream = (AlsaStream*)p;
	snd_pcm_sframes_t pcm = 0;
	unsigned long framesToWrite = stream->_FramesPerBuffer;

	/*struct sched_param params;
	pthread_t this_thread = pthread_self();
	params.__sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(this_thread, SCHED_FIFO, &params);*/

	do
	{
		stream->_NextPosition += stream->_PositionStep;
		//Ist die Anzahl verbliebener Frames im wave kleiner als die Anzahl frames per buffer dann:
		if (stream->_NextPosition >= stream->_WaveDataSize)
		{
			framesToWrite = (stream->_WaveDataSize - stream->_Position) / (stream->_Channels * static_cast<unsigned long>(stream->_SoundFormat));
		}
		pcm = snd_pcm_writei(stream->_AlsaHandle
			, &stream->_WaveData[stream->_Position]
			, static_cast<snd_pcm_uframes_t>(framesToWrite));
		if (pcm == -EPIPE) {
			if (snd_pcm_prepare(stream->_AlsaHandle) < 0)
			{
				printf("super error\n");
			}
		}

		stream->_Position += stream->_PositionStep;

		if (stream->_Loop)
		{
			if (stream->_Position >= stream->_WaveDataSize)
			{
				stream->_Position = 0;
				stream->_NextPosition = 0;
				framesToWrite = stream->_FramesPerBuffer;
			}
		}

	} while ((stream->_Position <= stream->_WaveDataSize) && stream->_Play);

	stream->close();
	stream->_Playing = false;
	return NULL;
}


