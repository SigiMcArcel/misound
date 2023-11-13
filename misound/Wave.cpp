#include "Wave.h"


misound::Wave::Wave(const string& path,const string& name, bool loop)
	:_waveFile(NULL)
	,_error(false)
	,_waveData(NULL)
	,_info()
	,_loop(loop)
	,_samplePosition(0)
	,_waveSize(0)
	,_playing(false)
	,_name(name)
	,_path(path)
	,_stream(NULL)
{
	
	
	SNDFILE* file = sf_open(path.c_str(), SFM_READ, &_info);
	if (sf_error(file) != SF_ERR_NO_ERROR)
	{
		printf( "%s\n", sf_strerror(_waveFile));
		printf( "File: %s\n", _path.c_str());
		_error = 2;
		return;
	}
	
	int test = (_info.format & SF_FORMAT_PCM_16);
	if ((_info.format & 0x0f) == SF_FORMAT_PCM_16)
	{
		_format = misound::SoundFormat::SoundFormat_S16_LE;
	}
	else if((_info.format & 0x0f) == SF_FORMAT_PCM_24)
	{
		_format = misound::SoundFormat::SoundFormat_S24_LE;
	}
	else
	{
		printf("Wave.c Format not supported\n");
		return;
	}
	_waveSize = (unsigned long)_info.frames * _info.channels * static_cast<unsigned long>(_format);
	_waveData = (unsigned char*)malloc(static_cast<size_t>(_waveSize));
	sf_read_raw(file, _waveData, _waveSize);
	sf_close(file);

	if (_waveData == NULL)
	{
		printf("Memory: %s\n", path.c_str());
		_error = 1;
		sf_close(file);
		return;

	}
	_stream = new AlsaStream((unsigned int)_info.samplerate,_info.channels,_format);
}

misound::Wave:: Wave(const Wave& other)
	:_waveFile(other._waveFile)
	, _error(other._error)
	, _waveData(other._waveData)
	, _info(other._info)
	, _loop(other._loop)
	, _samplePosition(other._samplePosition)
	, _waveSize(other._waveSize)
	, _playing(other._playing)
	, _name(other._name)
	, _stream(other._stream)
{
	
};

misound::Wave::Wave()
	:_waveFile(NULL)
	, _error(false)
	, _waveData(NULL)
	, _info()
	, _loop(false)
	, _samplePosition(0)
	, _waveSize(0)
	, _playing(false)
	, _name("")
	, _stream(NULL)
{
	
};

unsigned long misound::Wave::samplePosition() const
{
	return _samplePosition;
}
unsigned long misound::Wave::size() const
{
	return _waveSize;
}
int misound::Wave::error()
{
	return _error;
}

const SF_INFO& misound::Wave::info()
{
	return _info;
}

misound::Wave::~Wave()
{
	if (_waveData != NULL)
	{
		free(_waveData);
	}
	_waveData = NULL;
	delete _stream;
}

void misound::Wave::play()
{
	if(!_stream->playing())
	{
		_stream->playWave(_waveData, static_cast<unsigned long>(_info.frames), _loop);
	}	
}

void misound::Wave::stop()
{
	_stream->stopWave();
}

