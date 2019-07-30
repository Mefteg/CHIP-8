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
{

}

byte Chip8Emulator::getDelayTimer() const
{
	return m_delayTimer;
}

void Chip8Emulator::setDelayTimer(byte timer)
{
	m_delayTimer = timer;
}

byte Chip8Emulator::getSoundTimer() const
{
	return m_soundTimer;
}

void Chip8Emulator::setSoundTimer(byte timer)
{
	m_soundTimer = timer;
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

void Chip8Emulator::Chip8Emulator::loadROM(const char* path)
{
	// Load in the game
   	FILE* fileDesc;
   	fileDesc = fopen(path, "rb");

   	// Skip CHIP-8 interpreter space in m_memory.
   	fread(&m_memory[0x200], 0xfff, 1, fileDesc);

   	fclose(fileDesc);
}


bool Chip8Emulator::processNextOpCode()
{
	word opCode = GetNextOpCode();
	m_programCounter += 2;

	m_drawableFlag = false;

	switch (opCode & 0xf000)
	{
		case 0x0000:
		{
			switch (opCode & 0x000f)
			{
				case 0x000e:
				{
					ProcessOpCode00EE(opCode);
					break;
				}

				default:
				{
					std::cout << "Unknown OpCode: " << std::hex << opCode << std::endl;
					return false;
				}
			}

			break;
		}

		case 0x1000:
		{
			ProcessOpCode1NNN(opCode);
			break;
		}

		case 0x2000:
		{
			ProcessOpCode2NNN(opCode);
			break;
		}

		case 0x3000:
		{
			ProcessOpCode3XNN(opCode);
			break;
		}

		case 0x4000:
		{
			ProcessOpCode4XNN(opCode);
			break;
		}

		case 0x6000:
		{
			ProcessOpCode6XNN(opCode);
			break;
		}

		case 0x7000:
		{
			ProcessOpCode7XNN(opCode);
			break;
		}

		case 0x8000:
		{
			switch (opCode & 0x000f)
			{
				case 0x0000:
				{
					ProcessOpCode8XY0(opCode);
					break;
				}

				case 0x0002:
				{
					ProcessOpCode8XY2(opCode);
					break;
				}

				case 0x0004:
				{
					ProcessOpCode8XY4(opCode);
					break;
				}

				case 0x0005:
				{
					ProcessOpCode8XY5(opCode);
					break;
				}

				default:
				{
					std::cout << "Unknown OpCode: " << std::hex << opCode << std::endl;
					return false;
				}
			}

			break;
		}

		case 0x9000:
		{
			ProcessOpCode9XY0(opCode);
			break;
		}

		case 0xa000:
		{
			ProcessOpCodeANNN(opCode);
			break;
		}

		case 0xc000:
		{
			ProcessOpCodeCXNN(opCode);
			break;
		}

		case 0xd000:
		{
			ProcessOpCodeDXYN(opCode);
			break;
		}

		case 0xe000:
		{
			switch (opCode & 0x00ff)
			{
				case 0x009E:
				{
					ProcessOpCodeEX9E(opCode);
					break;
				}

				case 0x00A1:
				{
					ProcessOpCodeEXA1(opCode);
					break;
				}

				default:
				{
					std::cout << "Unknown OpCode: " << std::hex << opCode << std::endl;
					return false;
				}
			}

			break;	
		}

		case 0xf000:
		{
			switch (opCode & 0x00ff)
			{
				case 0x0007:
				{
					ProcessOpCodeFX07(opCode);
					break;
				}

				case 0x0015:
				{
					ProcessOpCodeFX15(opCode);
					break;
				}

				case 0x0018:
				{
					ProcessOpCodeFX18(opCode);
					break;
				}

				case 0x001e:
				{
					ProcessOpCodeFX1E(opCode);
					break;
				}

				case 0x0029:
				{
					ProcessOpCodeFX29(opCode);
					break;
				}

				case 0x0033:
				{
					ProcessOpCodeFX33(opCode);
					break;
				}

				case 0x0065:
				{
					ProcessOpCodeFX65(opCode);
					break;
				}

				default:
				{
					std::cout << "Unknown OpCode: " << std::hex << opCode << std::endl;
					return false;
				}
			}

			break;
		}

		default:
		{
			std::cout << "Unknown OpCode: " << std::hex << opCode << std::endl;
			return false;
		}
	}

	return true;
}

word Chip8Emulator::GetNextOpCode()
{
	byte opCode[2] = {
		m_memory[m_programCounter + 1],
		m_memory[m_programCounter]
	};

	return *((word*)opCode);
}

void Chip8Emulator::ProcessOpCode00EE(word opCode)
{
	m_programCounter = m_stack.top();
	m_stack.pop();
}

void Chip8Emulator::ProcessOpCode1NNN(word opCode)
{
	m_programCounter = opCode & 0x0fff;
}

void Chip8Emulator::ProcessOpCode2NNN(word opCode)
{
	m_stack.push(m_programCounter);
	m_programCounter = opCode & 0x0fff;
}

void Chip8Emulator::ProcessOpCode3XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_dataRegisters[x] == (opCode & 0x00ff))
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::ProcessOpCode4XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_dataRegisters[x] != (opCode & 0x00ff))
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::ProcessOpCode6XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_dataRegisters[x] = opCode & 0x00ff;
}

void Chip8Emulator::ProcessOpCode7XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_dataRegisters[x] += opCode & 0x00ff;
}

void Chip8Emulator::ProcessOpCode8XY0(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;
	m_dataRegisters[x] = m_dataRegisters[y];
}

void Chip8Emulator::ProcessOpCode8XY2(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;
	m_dataRegisters[x] = m_dataRegisters[x] & m_dataRegisters[y];
}

void Chip8Emulator::ProcessOpCode8XY4(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	word valueX = m_dataRegisters[x];
	word valueY = m_dataRegisters[y];

	word newValue = valueX + valueY;

	m_dataRegisters[0xf] = valueX + valueY > 0xff ? 1 : 0;

	m_dataRegisters[x] += m_dataRegisters[y];
}

void Chip8Emulator::ProcessOpCode8XY5(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	short valueX = m_dataRegisters[x];
	short valueY = m_dataRegisters[y];

	short newValue = valueX + valueY;

	m_dataRegisters[0xf] = m_dataRegisters[x] < m_dataRegisters[y] ? 0 : 1;

	m_dataRegisters[x] -= m_dataRegisters[y];
}

void Chip8Emulator::ProcessOpCode9XY0(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	if (m_dataRegisters[x] != m_dataRegisters[y])
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::ProcessOpCodeANNN(word opCode)
{
	m_addressRegisterI = opCode & 0x0fff;
}

void Chip8Emulator::ProcessOpCodeCXNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_dataRegisters[x] = rand() & (opCode & 0x00ff);
}

void Chip8Emulator::ProcessOpCodeDXYN(word opCode)
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
				int pixelIndex = (coordX + j) * ScreenHeight + (coordY + i);
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
}

void Chip8Emulator::ProcessOpCodeEX9E(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_keys[m_dataRegisters[x]] > 0)
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::ProcessOpCodeEXA1(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (m_keys[m_dataRegisters[x]] == 0)
	{
		m_programCounter += 2;
	}
}

void Chip8Emulator::ProcessOpCodeFX07(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_dataRegisters[x] = m_delayTimer;
}

void Chip8Emulator::ProcessOpCodeFX15(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_delayTimer = m_dataRegisters[x];
}

void Chip8Emulator::ProcessOpCodeFX18(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_soundTimer = m_dataRegisters[x];
}

void Chip8Emulator::ProcessOpCodeFX1E(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_addressRegisterI += m_dataRegisters[x];
}

void Chip8Emulator::ProcessOpCodeFX29(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	m_addressRegisterI = m_dataRegisters[x] * 5;
}

void Chip8Emulator::ProcessOpCodeFX33(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte value = m_dataRegisters[x];

	m_memory[m_addressRegisterI] = value / 100; // Hundreds.
	m_memory[m_addressRegisterI + 1] = (value / 10) % 10; // Tens.
	m_memory[m_addressRegisterI + 2] = value % 10; // Units.
}

void Chip8Emulator::ProcessOpCodeFX65(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;

	for (byte i = 0; i <= x; ++i)
	{
		m_dataRegisters[i] = m_memory[m_addressRegisterI + i];
	}

	m_addressRegisterI += x + 1;
}
