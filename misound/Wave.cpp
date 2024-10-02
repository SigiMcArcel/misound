#include "Wave.h"

bool misound::Wave::setup()
{
	SNDFILE* file = sf_open(_Path.c_str(), SFM_READ, &_Info);
	if (sf_error(file) != SF_ERR_NO_ERROR)
	{
		printf("%s\n", sf_strerror(file));
		printf("File: %s\n", _Path.c_str());
		_Error = 2;
		return false;
	}

	if ((_Info.format & 0x0f) == SF_FORMAT_PCM_16)
	{
		_Format = misound::SoundFormat::SoundFormat_S16_LE;
	}
	else if ((_Info.format & 0x0f) == SF_FORMAT_PCM_24)
	{
		_Format = misound::SoundFormat::SoundFormat_S24_LE;
	}
	else
	{
		printf("Wave.c Format not supported\n");
		return false;
	}
	_WaveSize = (unsigned long)_Info.frames * _Info.channels * static_cast<unsigned long>(_Format);
	_WaveData = std::shared_ptr<unsigned char[]>(new unsigned char[_WaveSize]);
	sf_read_raw(file, _WaveData.get(), _WaveSize);
	sf_close(file);

	
	_Stream.setup((unsigned int)_Info.samplerate, _Info.channels, _Format, _SoundCard);
	return true;
}

misound::Wave::Wave(const string& path,const string& name, const std::string& soundCard, bool loop)
	:_WaveFile(NULL)
	,_WaveData(NULL)
	,_Info()
	,_Loop(loop)
	,_SamplePosition(0)
	,_WaveSize(0)
	,_Playing(false)
	,_Name(name)
	,_Path(path)
	,_Stream()
	,_SoundCard(soundCard)
	, _Error(false)
{
	setup();
}

misound::Wave:: Wave(const Wave& other)
	: _WaveFile(other._WaveFile)
	, _WaveData(other._WaveData)
	, _Info(other._Info)
	, _Loop(other._Loop)
	, _SamplePosition(other._SamplePosition)
	, _WaveSize(other._WaveSize)
	, _Playing(other._Playing)
	, _Name(other._Name)
	, _Stream(other._Stream)
	, _SoundCard(other._SoundCard)
	, _Error(other._Error)
{
	
};

misound::Wave::Wave()
	: _WaveFile(NULL)
	, _WaveData(NULL)
	, _Info()
	, _Loop(false)
	, _SamplePosition(0)
	, _WaveSize(0)
	, _Playing(false)
	, _Name("")
	, _Stream()
	, _SoundCard(_DefaultSoundCard)
	, _Error(false)
{
	
};

unsigned long misound::Wave::samplePosition() const
{
	return _SamplePosition;
}
unsigned long misound::Wave::size() const
{
	return _WaveSize;
}
int misound::Wave::error()
{
	return _Error;
}

const SF_INFO& misound::Wave::info()
{
	return _Info;
}

misound::Wave::~Wave()
{
	
}

void misound::Wave::play()
{
	if(!_Stream.playing())
	{
		_Stream.playWave(_WaveData, static_cast<unsigned long>(_Info.frames), _Loop);
	}	
}

void misound::Wave::stop()
{
	_Stream.stopWave();
}


