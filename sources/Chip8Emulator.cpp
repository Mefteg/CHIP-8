#include "CHIP-8/Chip8Emulator.hpp"

#include <iostream>

using namespace CHIP8;

const byte Chip8Emulator::Fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8Emulator::Chip8Emulator()
	: m_cyclesPerFrame(DefaultCyclesPerFrame)
{
	m_opCodeProcessorMap.insert({0x0, &Chip8Emulator::processOpCodeGroup0});
	m_opCodeProcessorMap.insert({0x1, &Chip8Emulator::processOpCode1NNN});
	m_opCodeProcessorMap.insert({0x2, &Chip8Emulator::processOpCode2NNN});
	m_opCodeProcessorMap.insert({0x3, &Chip8Emulator::processOpCode3XNN});
	m_opCodeProcessorMap.insert({0x4, &Chip8Emulator::processOpCode4XNN});
	m_opCodeProcessorMap.insert({0x5, &Chip8Emulator::processOpCode5XY0});
	m_opCodeProcessorMap.insert({0x6, &Chip8Emulator::processOpCode6XNN});
	m_opCodeProcessorMap.insert({0x7, &Chip8Emulator::processOpCode7XNN});
	m_opCodeProcessorMap.insert({0x8, &Chip8Emulator::processOpCodeGroup8});
	m_opCodeProcessorMap.insert({0x9, &Chip8Emulator::processOpCode9XY0});
	m_opCodeProcessorMap.insert({0xa, &Chip8Emulator::processOpCodeANNN});
	m_opCodeProcessorMap.insert({0xc, &Chip8Emulator::processOpCodeCXNN});
	m_opCodeProcessorMap.insert({0xd, &Chip8Emulator::processOpCodeDXYN});
	m_opCodeProcessorMap.insert({0xe, &Chip8Emulator::processOpCodeGroupE});
	m_opCodeProcessorMap.insert({0xf, &Chip8Emulator::processOpCodeGroupF});

	m_opCodeGroup0ProcessorMap.insert({0x0, &Chip8Emulator::processOpCode00E0});
	m_opCodeGroup0ProcessorMap.insert({0xe, &Chip8Emulator::processOpCode00EE});

	m_opCodeGroup8ProcessorMap.insert({0x0, &Chip8Emulator::processOpCode8XY0});
	m_opCodeGroup8ProcessorMap.insert({0x1, &Chip8Emulator::processOpCode8XY1});
	m_opCodeGroup8ProcessorMap.insert({0x2, &Chip8Emulator::processOpCode8XY2});
	m_opCodeGroup8ProcessorMap.insert({0x3, &Chip8Emulator::processOpCode8XY3});
	m_opCodeGroup8ProcessorMap.insert({0x4, &Chip8Emulator::processOpCode8XY4});
	m_opCodeGroup8ProcessorMap.insert({0x5, &Chip8Emulator::processOpCode8XY5});
	m_opCodeGroup8ProcessorMap.insert({0x6, &Chip8Emulator::processOpCode8XY6});
	m_opCodeGroup8ProcessorMap.insert({0xe, &Chip8Emulator::processOpCode8XYE});

	m_opCodeGroupEProcessorMap.insert({0x9e, &Chip8Emulator::processOpCodeEX9E});
	m_opCodeGroupEProcessorMap.insert({0xa1, &Chip8Emulator::processOpCodeEXA1});

	m_opCodeGroupFProcessorMap.insert({0x07, &Chip8Emulator::processOpCodeFX07});
	m_opCodeGroupFProcessorMap.insert({0x0A, &Chip8Emulator::processOpCodeFX0A});
	m_opCodeGroupFProcessorMap.insert({0x15, &Chip8Emulator::processOpCodeFX15});
	m_opCodeGroupFProcessorMap.insert({0x18, &Chip8Emulator::processOpCodeFX18});
	m_opCodeGroupFProcessorMap.insert({0x1e, &Chip8Emulator::processOpCodeFX1E});
	m_opCodeGroupFProcessorMap.insert({0x29, &Chip8Emulator::processOpCodeFX29});
	m_opCodeGroupFProcessorMap.insert({0x33, &Chip8Emulator::processOpCodeFX33});
	m_opCodeGroupFProcessorMap.insert({0x55, &Chip8Emulator::processOpCodeFX55});
	m_opCodeGroupFProcessorMap.insert({0x65, &Chip8Emulator::processOpCodeFX65});
}

bool Chip8Emulator::isBeepPlayable() const
{
	return m_beepFlag;
}

const byte* Chip8Emulator::getScreenData() const
{
	return m_screenData;
}

bool Chip8Emulator::isDrawable() const
{
	return m_drawableFlag;
}

byte* Chip8Emulator::getKeys()
{
	return m_keys;
}


word Chip8Emulator::getCyclesPerFrame() const
{
	return m_cyclesPerFrame;
}

void Chip8Emulator::setCyclesPerFrame(word cyclesPerFrame)
{
	m_cyclesPerFrame = cyclesPerFrame;
}

void Chip8Emulator::Chip8Emulator::resetCPU()
{
	// Set all data registers to 0.
	memset(m_dataRegisters, 0, RegisterCount);
	// Set I to 0.
	m_addressRegisterI = 0;
	// Program starts at adress 0x200 (m_memory under was used for CHIP-8 interpreter).
	m_programCounter = 0x200;

	// Load Fontset in m_memory.
	for(byte i = 0; i < 80; ++i)
	{
		m_memory[i] = Fontset[i];	
	}

	memset(m_screenData, 0, ScreenWidth * ScreenHeight);
	memset(m_keys, 0, KeyCount);
}

bool Chip8Emulator::Chip8Emulator::loadROM(const char* path)
{
	// Load in the game
   	FILE* fileDesc;
   	fileDesc = fopen(path, "rb");
   	if (fileDesc == nullptr)
   	{
   		return false;
   	}

   	// Skip CHIP-8 interpreter space in m_memory.
   	fread(&m_memory[0x200], 0xfff, 1, fileDesc);

   	fclose(fileDesc);

   	return true;
}

bool Chip8Emulator::update()
{
	m_drawableFlag = false;
	m_beepFlag = false;

	for (word i = 0; i < m_cyclesPerFrame; ++i)
	{
		bool keepGoing = processNextOpCode();
		if (keepGoing == false)
		{
			return false;
		}	
	}

	processTimers();	

	return true;
}

word Chip8Emulator::getNextOpCode()
{
	byte opCode[2] = {
		m_memory[m_programCounter + 1],
		m_memory[m_programCounter]
	};

	return *((word*)opCode);
}

bool Chip8Emulator::processOpCodeInMap(const std::unordered_map<word, OpCodeProcessor>& processMap, byte key, word opCode)
{
	auto processor = processMap.find(key);
	if (processor == processMap.end())
	{
		unknownOpCode(opCode);
		return false;
	}

	// Call OpCode/OpCodeGroup processor.
	std::invoke(processor->second, *this, opCode);	

	return true;
}

bool Chip8Emulator::processNextOpCode()
{
	word opCode = getNextOpCode();

	return processOpCodeInMap(m_opCodeProcessorMap, (opCode & 0xf000) >> 12, opCode);
}

void Chip8Emulator::processOpCodeGroup0(word opCode)
{
	processOpCodeInMap(m_opCodeGroup0ProcessorMap, opCode & 0x000f, opCode);
}

void Chip8Emulator::processOpCode00E0(word opCode)
{
	memset(m_screenData, 0, ScreenWidth * ScreenHeight);
	m_drawableFlag = true;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode00EE(word opCode)
{
	m_programCounter = m_stack.top() + 2;
	m_stack.pop();
}

void Chip8Emulator::processOpCode1NNN(word opCode)
{
	m_programCounter = opCode & 0x0fff;
}

void Chip8Emulator::processOpCode2NNN(word opCode)
{
	m_stack.push(m_programCounter);
	m_programCounter = opCode & 0x0fff;
}

void Chip8Emulator::processOpCode3XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_dataRegisters[x] == (opCode & 0x00ff))
	{
		m_programCounter += 4;
	}
	else
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::processOpCode4XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_dataRegisters[x] != (opCode & 0x00ff))
	{
		m_programCounter += 4;
	}
	else
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::processOpCode5XY0(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	if (m_dataRegisters[x] == m_dataRegisters[y])
	{
		m_programCounter += 4;
	}
	else
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::processOpCode6XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;

	m_dataRegisters[x] = opCode & 0x00ff;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode7XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;

	m_dataRegisters[x] += opCode & 0x00ff;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeGroup8(word opCode)
{
	processOpCodeInMap(m_opCodeGroup8ProcessorMap, opCode & 0x000f, opCode);
}

void Chip8Emulator::processOpCode8XY0(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	m_dataRegisters[x] = m_dataRegisters[y];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode8XY1(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	m_dataRegisters[x] |= m_dataRegisters[y];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode8XY2(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	m_dataRegisters[x] &= m_dataRegisters[y];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode8XY3(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	m_dataRegisters[x] ^= m_dataRegisters[y];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode8XY4(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	word valueX = m_dataRegisters[x];
	word valueY = m_dataRegisters[y];

	word newValue = valueX + valueY;

	m_dataRegisters[0xf] = valueX + valueY > 0xff ? 1 : 0;

	m_dataRegisters[x] += m_dataRegisters[y];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode8XY5(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	short valueX = m_dataRegisters[x];
	short valueY = m_dataRegisters[y];

	short newValue = valueX + valueY;

	m_dataRegisters[0xf] = m_dataRegisters[x] < m_dataRegisters[y] ? 0 : 1;

	m_dataRegisters[x] -= m_dataRegisters[y];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode8XY6(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	short valueX = m_dataRegisters[x];

	m_dataRegisters[0xf] = valueX & 0x1;
	m_dataRegisters[x] >>= 1;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode8XYE(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	short valueX = m_dataRegisters[x];

	m_dataRegisters[0xf] = valueX >> 7;
	m_dataRegisters[x] <<= 1;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCode9XY0(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	if (m_dataRegisters[x] != m_dataRegisters[y])
	{
		m_programCounter += 4;
	}
	else
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::processOpCodeANNN(word opCode)
{
	m_addressRegisterI = opCode & 0x0fff;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeCXNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_dataRegisters[x] = rand() & (opCode & 0x00ff);

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeDXYN(word opCode)
{
	m_dataRegisters[0xf] = 0;

	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;
	byte n = opCode & 0x000f;

	byte coordX = m_dataRegisters[x];
	byte coordY = m_dataRegisters[y];

	for (byte i = 0; i < n; ++i)
	{
		byte spriteLine = m_memory[m_addressRegisterI + i];

		for (byte j = 0; j < 8; ++j)
		{
			byte currentBitMask = 1 << ((8 - 1) - j);
			if ((spriteLine & currentBitMask) > 0)
			{
				word pixelIndex = (coordX + j) * ScreenHeight + (coordY + i);
				byte screenPixel = m_screenData[pixelIndex];

				if (screenPixel == 1)
				{
					// Toogle off.
					screenPixel = 0;
					// Signal a collision.
					m_dataRegisters[0xf] = 1;
				}
				else
				{
					// Toogle on.
					screenPixel = 1;
				}

				m_screenData[pixelIndex] = screenPixel;
			}
		}
	}

	m_drawableFlag = true;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeGroupE(word opCode)
{
	processOpCodeInMap(m_opCodeGroupEProcessorMap, opCode & 0x00ff, opCode);
}

void Chip8Emulator::processOpCodeEX9E(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_keys[m_dataRegisters[x]] != 0)
	{
		m_programCounter += 4;
	}
	else
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::processOpCodeEXA1(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_keys[m_dataRegisters[x]] == 0)
	{
		m_programCounter += 4;
	}
	else
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::processOpCodeGroupF(word opCode)
{
	processOpCodeInMap(m_opCodeGroupFProcessorMap, opCode & 0x00ff, opCode);
}

void Chip8Emulator::processOpCodeFX07(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_dataRegisters[x] = m_delayTimer;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeFX0A(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;

	for (byte i = 0; i < KeyCount; ++i)
	{
		if (m_keys[i] != 0)
		{
			m_dataRegisters[x] = i;
			m_programCounter += 2;

			return;
		}
	}
}

void Chip8Emulator::processOpCodeFX15(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_delayTimer = m_dataRegisters[x];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeFX18(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_soundTimer = m_dataRegisters[x];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeFX1E(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if(m_addressRegisterI + m_dataRegisters[x] > 0xfff)
	{
		m_dataRegisters[0xf] = 1;
	}
	else
	{
		m_dataRegisters[0xf] = 0;
	}
	m_addressRegisterI += m_dataRegisters[x];

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeFX29(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_addressRegisterI = m_dataRegisters[x] * 5;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeFX33(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte value = m_dataRegisters[x];

	m_memory[m_addressRegisterI] = value / 100; // Hundreds.
	m_memory[m_addressRegisterI + 1] = (value / 10) % 10; // Tens.
	m_memory[m_addressRegisterI + 2] = value % 10; // Units.

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeFX55(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	
	for (byte i = 0; i <= x; ++i)
	{
		m_memory[m_addressRegisterI + i] = m_dataRegisters[i];
	}

	m_addressRegisterI += x + 1;

	m_programCounter += 2;
}

void Chip8Emulator::processOpCodeFX65(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;

	for (byte i = 0; i <= x; ++i)
	{
		m_dataRegisters[i] = m_memory[m_addressRegisterI + i];
	}

	m_addressRegisterI += x + 1;

	m_programCounter += 2;
}

void Chip8Emulator::unknownOpCode(word opCode)
{
	std::cout << "Unknown OpCode: " << std::hex << opCode << std::endl;	
}

void Chip8Emulator::processTimers()
{
	if (m_delayTimer > 0)
	{
		--m_delayTimer;
	}

	if (m_soundTimer > 0)
	{
		if (m_soundTimer == 1)
		{
			m_beepFlag = true;
		}

		--m_soundTimer;
	}
}
