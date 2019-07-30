#pragma once

#include <stack>

#include "Chip8Types.hpp"

namespace CHIP8
{
	class Chip8Emulator
	{
	public:
		static const byte ScreenWidth = 64;
		static const byte ScreenHeight = 32;

	private:
		static const word MemorySize = 4096;
		static const byte RegisterCount = 16;

		static const byte KeyCount = 16;

		static const byte Fontset[80];

		byte m_memory[MemorySize];
		byte m_dataRegisters[RegisterCount];
		word m_addressRegisterI;
		word m_programCounter;
		std::stack<word> m_stack;

		byte m_delayTimer;
		byte m_soundTimer;

		byte m_screenData[ScreenWidth * ScreenHeight];

		byte m_keys[KeyCount];

	public:
		Chip8Emulator();

		byte getDelayTimer() const;
		void setDelayTimer(byte timer);

		byte getSoundTimer() const;
		void setSoundTimer(byte timer);

		const byte* getScreenData() const;

		byte* getKeys();

		void resetCPU();
		void loadROM(const char* path);

		bool processNextOpCode();

	private:
		word GetNextOpCode();

		void ProcessOpCode00EE(word opCode);

		void ProcessOpCode1NNN(word opCode);

		void ProcessOpCode2NNN(word opCode);

		void ProcessOpCode3XNN(word opCode);

		void ProcessOpCode4XNN(word opCode);

		void ProcessOpCode6XNN(word opCode);

		void ProcessOpCode7XNN(word opCode);

		void ProcessOpCode8XY0(word opCode);
		void ProcessOpCode8XY2(word opCode);
		void ProcessOpCode8XY4(word opCode);
		void ProcessOpCode8XY5(word opCode);

		void ProcessOpCode9XY0(word opCode);

		void ProcessOpCodeANNN(word opCode);

		void ProcessOpCodeCXNN(word opCode);

		void ProcessOpCodeDXYN(word opCode);

		void ProcessOpCodeEX9E(word opCode);
		void ProcessOpCodeEXA1(word opCode);

		void ProcessOpCodeFX07(word opCode);
		void ProcessOpCodeFX15(word opCode);
		void ProcessOpCodeFX18(word opCode);
		void ProcessOpCodeFX1E(word opCode);
		void ProcessOpCodeFX29(word opCode);
		void ProcessOpCodeFX33(word opCode);
		void ProcessOpCodeFX65(word opCode);
	};	
}
