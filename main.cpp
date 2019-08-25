#include <cstdlib>
#include <stack>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <CHIP-8/Chip8Emulator.hpp>

const unsigned short RenderScale = 20;

void HandleInput(CHIP8::byte* keys, CHIP8::byte keyIndex, sf::Keyboard::Key key)
{
	if (sf::Keyboard::isKeyPressed(key))
	{
		keys[keyIndex] = 1;
	}
	else
	{
		keys[keyIndex] = 0;
	}
}

void HandleInputs(CHIP8::byte* keys)
{
	for (int i = 0; i < 16; ++i)
	{
		keys[i] = 0;
	}

	HandleInput(keys, 0x0, sf::Keyboard::X);
	HandleInput(keys, 0x1, sf::Keyboard::Num1);
	HandleInput(keys, 0x2, sf::Keyboard::Num2);
	HandleInput(keys, 0x3, sf::Keyboard::Num3);
	HandleInput(keys, 0xC, sf::Keyboard::Num4);
	HandleInput(keys, 0x4, sf::Keyboard::Q);
	HandleInput(keys, 0x5, sf::Keyboard::W);
	HandleInput(keys, 0x6, sf::Keyboard::E);
	HandleInput(keys, 0xD, sf::Keyboard::R);
	HandleInput(keys, 0x7, sf::Keyboard::A);
	HandleInput(keys, 0x8, sf::Keyboard::S);
	HandleInput(keys, 0x9, sf::Keyboard::D);
	HandleInput(keys, 0xE, sf::Keyboard::F);
	HandleInput(keys, 0xA, sf::Keyboard::Z);
	HandleInput(keys, 0x0, sf::Keyboard::X);
	HandleInput(keys, 0xB, sf::Keyboard::C);
	HandleInput(keys, 0xF, sf::Keyboard::V);
}

void Draw(sf::RenderWindow& window, sf::Sprite& sprite, sf::Texture& texture, const CHIP8::byte* screenData)
{
	window.clear();

	unsigned short pixelCount = CHIP8::Chip8Emulator::ScreenWidth * CHIP8::Chip8Emulator::ScreenHeight;
	CHIP8::byte pixels[pixelCount * 4];
	for (unsigned int i = 0; i < CHIP8::Chip8Emulator::ScreenWidth; ++i)
    {
		for (unsigned int j = 0; j < CHIP8::Chip8Emulator::ScreenHeight; ++j)
	    {
	    	CHIP8::word chip8PixelIndex = (i * CHIP8::Chip8Emulator::ScreenHeight) + j;
	    	CHIP8::byte color = screenData[chip8PixelIndex] == 0 ? 0 : 0xff;
	    	unsigned int index = (j * CHIP8::Chip8Emulator::ScreenWidth + i) * 4;
	        pixels[index] = color;
	        pixels[index + 1] = color;
	        pixels[index + 2] = color;
	        pixels[index + 3] = 0xff;
	    }
	}

	texture.update(pixels);

	window.draw(sprite);

	window.display();
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "No ROM provided. Please provide a path to a valid CHIP-8 ROM." << std::endl;
		return EXIT_FAILURE;
	}

	// Get ROM path.
	const std::string path = argv[1];

	// Create CHIP-8 emulator, initialize it and load the provided ROM.
	CHIP8::Chip8Emulator chip8;
	chip8.resetCPU();
	if (chip8.loadROM(path.c_str()) == false)
	{
		std::cerr << "Provided CHIP-8 ROM path isn't valid. Please provide a path to a valid CHIP-8 ROM." << std::endl;
		return EXIT_FAILURE;
	}

	sf::RenderWindow window(sf::VideoMode(CHIP8::Chip8Emulator::ScreenWidth * RenderScale, CHIP8::Chip8Emulator::ScreenHeight * RenderScale), "CHIP-8");

	sf::Texture texture;
    texture.create(CHIP8::Chip8Emulator::ScreenWidth, CHIP8::Chip8Emulator::ScreenHeight);

	sf::Sprite sprite;
    sprite.setTexture(texture, true);
    sprite.setScale(sf::Vector2f(RenderScale, RenderScale));

    sf::SoundBuffer beepBuffer;
    beepBuffer.loadFromFile("../resources/sounds/beep.wav");
    sf::Sound beep;
    beep.setBuffer(beepBuffer);

    sf::Clock clock;
	const unsigned int fps = 60;
	const float interval = 1000 * 1000.0f / fps;

	// Main loop.
    while (window.isOpen())
    {
    	// Process events.
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // Start the clock to track CHIP-8 process time.
        clock.restart();

        HandleInputs(chip8.getKeys());

		// Update CHIP-8.
        bool keepGoing = chip8.update();
        if (keepGoing == false)
        {
        	return EXIT_FAILURE;
        }	

    	if (chip8.isBeepPlayable() == true)
    	{
			// BEEP !
			beep.play();
    	}

		// draw the frame.
        if (chip8.isDrawable() == true)
        {
        	Draw(window, sprite, texture, chip8.getScreenData());
        }

        // Compute time consumed by the CHIP-8 update.
        sf::Time elapsed = clock.getElapsedTime();

        // Compute the time to sleep to stay at a fixed framerate.
        long long sleepingDuration = interval - elapsed.asMicroseconds();
        sleepingDuration = sleepingDuration > 0 ? sleepingDuration : 0;
        // The process doesn't take 1/60 second, so wait the remaining time.
		sf::sleep(sf::microseconds(sleepingDuration));
    }

	return EXIT_SUCCESS;
}