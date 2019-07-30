#include <cstdlib>
#include <stack>
#include <string>
#include <iostream>
#include <chrono>

#include <SFML/Graphics.hpp>

typedef unsigned char byte;
typedef unsigned short word;

const word MemorySize = 0x0fff;
const byte RegisterCount = 0x10;

const byte ScreenWidth = 64;
const byte ScreenHeight = 32;

const byte KeyCount = 16;

const byte Fontset[80] = {
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

const unsigned short RenderScale = 20;

byte Memory[MemorySize];
byte DataRegisters[RegisterCount];
word AdressRegisterI;
word ProgramCounter;
std::stack<word> Stack;

byte DelayTimer;
byte SoundTimer;

byte ScreenData[ScreenWidth][ScreenHeight];

byte Keys[KeyCount];

sf::RenderWindow RenderWindow(sf::VideoMode(ScreenWidth * RenderScale, ScreenHeight * RenderScale), "CHIP-8");
//sf::RenderWindow RenderWindow(sf::VideoMode(600, 800), "CHIP-8");
sf::Texture RenderTexture;
sf::Sprite RenderSprite;

void ResetCPU()
{
	// Set all data registers to 0.
	memset(DataRegisters, 0, RegisterCount);
	// Set I to 0.
	AdressRegisterI = 0;
	// Program starts at adress 0x200 (memory under was used for CHIP-8 interpreter).
	ProgramCounter = 0x200;

	// Load Fontset in memory.
	for(byte i = 0; i < 80; ++i)
	{
		Memory[i] = Fontset[i];	
	}
}

void LoadROM(const std::string& path)
{
	// Load in the game
   	FILE* fileDesc;
   	fileDesc = fopen(path.c_str(), "rb");

   	// Skip CHIP-8 interpreter space in memory.
   	fread(&Memory[0x200], 0xfff, 1, fileDesc);

   	fclose(fileDesc);
}

word GetNextOpCode()
{
	byte opCode[2] = {
		Memory[ProgramCounter + 1],
		Memory[ProgramCounter]
	};

	return *((word*)opCode);
}

void ProcessOpCode00EE(word opCode)
{
	ProgramCounter = Stack.top();
	Stack.pop();
}

void ProcessOpCode1NNN(word opCode)
{
	ProgramCounter = opCode & 0x0fff;
}

void ProcessOpCode2NNN(word opCode)
{
	Stack.push(ProgramCounter);
	ProgramCounter = opCode & 0x0fff;
}

void ProcessOpCode3XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (DataRegisters[x] == (opCode & 0x00ff))
	{
		ProgramCounter += 2;
	}
}

void ProcessOpCode4XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (DataRegisters[x] != (opCode & 0x00ff))
	{
		ProgramCounter += 2;
	}
}

void ProcessOpCode6XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	DataRegisters[x] = opCode & 0x00ff;
}

void ProcessOpCode7XNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	DataRegisters[x] += opCode & 0x00ff;
}

void ProcessOpCode8XY0(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;
	DataRegisters[x] = DataRegisters[y];
}

void ProcessOpCode8XY2(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;
	DataRegisters[x] = DataRegisters[x] & DataRegisters[y];
}

void ProcessOpCode8XY4(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	word valueX = DataRegisters[x];
	word valueY = DataRegisters[y];

	word newValue = valueX + valueY;

	DataRegisters[0xf] = valueX + valueY > 0xff ? 1 : 0;

	DataRegisters[x] += DataRegisters[y];
}

void ProcessOpCode8XY5(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	short valueX = DataRegisters[x];
	short valueY = DataRegisters[y];

	short newValue = valueX + valueY;

	DataRegisters[0xf] = DataRegisters[x] < DataRegisters[y] ? 0 : 1;

	DataRegisters[x] -= DataRegisters[y];
}

void ProcessOpCode9XY0(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;

	if (DataRegisters[x] != DataRegisters[y])
	{
		ProgramCounter += 2;
	}
}

void ProcessOpCodeANNN(word opCode)
{
	AdressRegisterI = opCode & 0x0fff;
}

void ProcessOpCodeCXNN(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	DataRegisters[x] = rand() & (opCode & 0x00ff);
}

void ProcessOpCodeDXYN(word opCode)
{
	DataRegisters[0xf] = 0;

	byte x = (opCode & 0x0f00) >> 8;
	byte y = (opCode & 0x00f0) >> 4;
	byte n = opCode & 0x000f;

	byte coordX = DataRegisters[x];
	byte coordY = DataRegisters[y];

	for (byte i = 0; i < n; ++i)
	{
		byte spriteLine = Memory[AdressRegisterI + i];

		for (byte j = 0; j < 8; ++j)
		{
			byte currentBitMask = 1 << ((8 - 1) - j);
			if ((spriteLine & currentBitMask) > 0)
			{
				byte screenPixel = ScreenData[coordX + j][coordY + i];

				if (screenPixel == 1)
				{
					// Toogle off.
					screenPixel = 0;
					// Signal a collision.
					DataRegisters[0xf] = 1;
				}
				else
				{
					// Toogle on.
					screenPixel = 1;
				}

				ScreenData[coordX + j][coordY + i] = screenPixel;
			}
		}
	}
}

void ProcessOpCodeEX9E(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (Keys[DataRegisters[x]] > 0)
	{
		ProgramCounter += 2;
	}
}

void ProcessOpCodeEXA1(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	if (Keys[DataRegisters[x]] == 0)
	{
		ProgramCounter += 2;
	}
}

void ProcessOpCodeFX07(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	DataRegisters[x] = DelayTimer;
}

void ProcessOpCodeFX15(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	DelayTimer = DataRegisters[x];
}

void ProcessOpCodeFX18(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	SoundTimer = DataRegisters[x];
}

void ProcessOpCodeFX1E(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	AdressRegisterI += DataRegisters[x];
}

void ProcessOpCodeFX29(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	AdressRegisterI = DataRegisters[x] * 5;
}

void ProcessOpCodeFX33(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;
	byte value = DataRegisters[x];

	Memory[AdressRegisterI] = value / 100; // Hundreds.
	Memory[AdressRegisterI + 1] = (value / 10) % 10; // Tens.
	Memory[AdressRegisterI + 2] = value % 10; // Units.
}

void ProcessOpCodeFX65(word opCode)
{
	byte x = (opCode & 0x0f00) >> 8;

	for (byte i = 0; i <= x; ++i)
	{
		DataRegisters[i] = Memory[AdressRegisterI + i];
	}

	AdressRegisterI += x + 1;
}

bool ProcessNextOpCode()
{
	word opCode = GetNextOpCode();
	ProgramCounter += 2;

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

void HandleInput(byte keyIndex, sf::Keyboard::Key key)
{
	if (sf::Keyboard::isKeyPressed(key))
	{
		Keys[keyIndex] = 1;
	}
	else
	{
		Keys[keyIndex] = 0;
	}
}

void HandleInputs()
{
	HandleInput(4, sf::Keyboard::A);
	HandleInput(5, sf::Keyboard::Left);
	HandleInput(6, sf::Keyboard::Right);
	HandleInput(7, sf::Keyboard::Up);
	HandleInput(8, sf::Keyboard::Down);
}

void Draw()
{
	/*std::string border(ScreenWidth + 2, '-');
	border[0] = ' ';
	border[ScreenWidth + 1] = ' ';
	std::cout << border << std::endl;

	for (byte i = 0; i < ScreenHeight; ++i)
	{
		std::string line(ScreenWidth + 2, ' ');
		line[0] = '|';
		line[ScreenWidth + 1] = '|';

		for (byte j = 0; j < ScreenWidth; ++j)
		{
			if (ScreenData[j][i] == 0)
			{
				line[j + 1] = ' ';
			}
			else
			{
				line[j + 1] = '#';
			}
		}

		std::cout << line << std::endl;
	}

	std::cout << border << std::endl;*/

	RenderWindow.clear();

	unsigned short pixelCount = ScreenWidth * ScreenHeight;
	byte pixels[pixelCount * 4];
	for (unsigned int i = 0; i < ScreenHeight; ++i)
    {
		for (unsigned int j = 0; j < ScreenWidth; ++j)
	    {
	    	byte color = ScreenData[j][i] == 0 ? 0 : 0xff;
	    	unsigned int index = ((i * ScreenWidth) + j) * 4;
	        pixels[index] = color;
	        pixels[index + 1] = color;
	        pixels[index + 2] = color;
	        pixels[index + 3] = 0xff;
	    }
	}

	RenderTexture.update(pixels);

	RenderWindow.draw(RenderSprite);

	RenderWindow.display();
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "No ROM provided. Please provide a path to a valid CHIP-8 ROM.";
		return EXIT_FAILURE;
	}

	// Get ROM path.
	const std::string path = argv[1];

	ResetCPU();
	LoadROM(path);

	unsigned int fps = 60;
	float interval = 1000.0f / fps;
	int opCodeCountPerSecond = 100 / fps;
	auto timeSinceLastDraw = std::chrono::high_resolution_clock::now();

    RenderTexture.create(ScreenWidth, ScreenHeight);
    RenderSprite.setTexture(RenderTexture, true);
    RenderSprite.setScale(sf::Vector2f(RenderScale, RenderScale));

    while (RenderWindow.isOpen())
    {
        sf::Event event;
        while (RenderWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                RenderWindow.close();
            }
        }

        auto now = std::chrono::high_resolution_clock::now();
		auto elapsed = now - timeSinceLastDraw;
		long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		if (milliseconds >= interval)
		{
			if (DelayTimer > 0)
			{
				--DelayTimer;
			}

			if (SoundTimer > 0)
			{
				--SoundTimer;
			}

			if (SoundTimer > 0)
			{
				// BEEP !
			}

			HandleInputs();

			// Process OpCodes for the frame.
			for (int i = 0; i < opCodeCountPerSecond; ++i)
			{
				bool keepGoing = ProcessNextOpCode();
				if (keepGoing == false)
				{
					return EXIT_FAILURE;
				}	
			}

			// Draw the frame.
			Draw();

			timeSinceLastDraw = now;
		}
    }

	return EXIT_SUCCESS;
}