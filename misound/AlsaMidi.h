#pragma once
#include <vector>
#include <alsa/asoundlib.h>
#include <sched.h>
#include <pthread.h>
#include <string>
#include <mutex>

namespace misound
{
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

		void  startThread();
		void  stopThread();

	public:

		AlsaMidi(const std::string device)
			:_Device(device)
			, _MidiHandle(nullptr)
		{

		}

		int putMessage(const MidiMessage& message);
		int getMessage(MidiMessage& message);
		bool open();
		void close();
		bool read(midiMessage_t* data);
		bool write(midiMessage_t* data);
		static void* midiInputProc(void* p);
		static void* midiOutputProc(void* p);
	};
}

