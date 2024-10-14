#include "AlsaMidi.h"

void misound::AlsaMidi::startThread()
{
	_Stop = false;
	_MidiInputThread_id = pthread_create(&_MidiInputThread, NULL, AlsaMidi::midiInputProc, this);
	_MidiOutputThread_id = pthread_create(&_MidiOutputThread, NULL, AlsaMidi::midiOutputProc, this);
	pthread_setname_np(_MidiInputThread, "midiinproc");
	pthread_setname_np(_MidiOutputThread, "midioutproc");
}

void misound::AlsaMidi::stopThread()
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

int misound::AlsaMidi::putMessage(const MidiMessage& message)
{
	_MidiOutputBuffer.push_back(message);
	return static_cast<int>(_MidiOutputBuffer.size());
}

int misound::AlsaMidi::getMessage(MidiMessage& message)
{
	if (_MidiInputBuffer.empty())
	{
		return -1;
	}

	message = _MidiInputBuffer.back();
	_MidiInputBuffer.pop_back();
	return static_cast<int>(_MidiInputBuffer.size());
}

bool misound::AlsaMidi::open()
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

void misound::AlsaMidi::close()
{
	snd_rawmidi_close(_MidiHandle);
}

bool misound::AlsaMidi::read(midiMessage_t* data)
{
	ssize_t status;
	if ((status = snd_rawmidi_read(_MidiHandle, data->U.MessageRaw, data->Len)) < 0) {
		printf("Problem writing to MIDI output: %s", snd_strerror(static_cast<int>(status)));
		return false;
	}
	return true;
}

bool misound::AlsaMidi::write(midiMessage_t* data)
{
	ssize_t status;
	if ((status = snd_rawmidi_write(_MidiHandle, data->U.MessageRaw, 3)) < 0) {
		printf("Problem writing to MIDI output: %s", snd_strerror(static_cast<int>(status)));
		return false;
	}
	return true;
}

void* misound::AlsaMidi::midiInputProc(void* p)
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
		if (alsaMidi->read(&data))
		{
			alsaMidi->_MidiInputBuffer.push_back(data);
		}
	}
	return nullptr;
}

void* misound::AlsaMidi::midiOutputProc(void* p)
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
			alsaMidi->write(&data);
			alsaMidi->_MidiOutputBuffer.pop_back();
		}
	}
	return nullptr;
}
