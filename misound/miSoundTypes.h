#pragma once
#include <memory>
namespace misound
{
	typedef enum WaveType_e
	{
		file,
		sine,
		square,
		triangle,
		sawTooth
	}WaveType;

	typedef enum class SoundFormat_e
	{
		SoundFormat_NONE = 0,
		SoundFormat_S16_LE = 2,
		SoundFormat_S24_LE = 3
	}SoundFormat;

	typedef std::shared_ptr<unsigned char[]> WaveData_t;
}
