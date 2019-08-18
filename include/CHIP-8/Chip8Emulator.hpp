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
		bool m_drawableFlag;

		byte m_keys[KeyCount];

	public:
		Chip8Emulator();

		byte getDelayTimer() const;
		void setDelayTimer(byte timer);

		byte getSoundTimer() const;
		void setSoundTimer(byte timer);

		const byte* getScreenData() const;

		bool isDrawable() const;

		byte* getKeys();

		void resetCPU();
		void loadROM(const char* path);

		bool processNextOpCode();

	private:
		word getNextOpCode();

		void processOpCode00EE(word opCode);

		void processOpCode1NNN(word opCode);

		void processOpCode2NNN(word opCode);

		void processOpCode3XNN(word opCode);

		void processOpCode4XNN(word opCode);

		void processOpCode6XNN(word opCode);

		void processOpCode7XNN(word opCode);

		void processOpCode8XY0(word opCode);
		void processOpCode8XY2(word opCode);
		void processOpCode8XY4(word opCode);
		void processOpCode8XY5(word opCode);

		void processOpCode9XY0(word opCode);

		void processOpCodeANNN(word opCode);

		void processOpCodeCXNN(word opCode);

		void processOpCodeDXYN(word opCode);

		void processOpCodeEX9E(word opCode);
		void processOpCodeEXA1(word opCode);

		void processOpCodeFX07(word opCode);
		void processOpCodeFX15(word opCode);
		void processOpCodeFX18(word opCode);
		void processOpCodeFX1E(word opCode);
		void processOpCodeFX29(word opCode);
		void processOpCodeFX33(word opCode);
		void processOpCodeFX65(word opCode);

		void unknownOpCode(word opCode);
	};	
}
