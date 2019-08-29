#include <cstdlib>
#include <stack>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <sstream>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <CHIP-8/Chip8Emulator.hpp>

#include <SceneGraph/GroupNode.hpp>
#include <SceneGraph/TextNode.hpp>

const unsigned short RenderScale = 20;

void HandleChip8Input(CHIP8::byte* keys, CHIP8::byte keyIndex, sf::Keyboard::Key key)
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

void HandleChip8Inputs(CHIP8::byte* keys)
{
	for (int i = 0; i < 16; ++i)
	{
		keys[i] = 0;
	}

	HandleChip8Input(keys, 0x0, sf::Keyboard::X);
	HandleChip8Input(keys, 0x1, sf::Keyboard::Num1);
	HandleChip8Input(keys, 0x2, sf::Keyboard::Num2);
	HandleChip8Input(keys, 0x3, sf::Keyboard::Num3);
	HandleChip8Input(keys, 0xC, sf::Keyboard::Num4);
	HandleChip8Input(keys, 0x4, sf::Keyboard::Q);
	HandleChip8Input(keys, 0x5, sf::Keyboard::W);
	HandleChip8Input(keys, 0x6, sf::Keyboard::E);
	HandleChip8Input(keys, 0xD, sf::Keyboard::R);
	HandleChip8Input(keys, 0x7, sf::Keyboard::A);
	HandleChip8Input(keys, 0x8, sf::Keyboard::S);
	HandleChip8Input(keys, 0x9, sf::Keyboard::D);
	HandleChip8Input(keys, 0xE, sf::Keyboard::F);
	HandleChip8Input(keys, 0xA, sf::Keyboard::Z);
	HandleChip8Input(keys, 0x0, sf::Keyboard::X);
	HandleChip8Input(keys, 0xB, sf::Keyboard::C);
	HandleChip8Input(keys, 0xF, sf::Keyboard::V);
}

void DrawChip8(sf::RenderWindow& window, sf::Sprite& sprite, sf::Texture& texture, const CHIP8::byte* screenData)
{
	unsigned short pixelCount = CHIP8::Chip8Emulator::ScreenWidth * CHIP8::Chip8Emulator::ScreenHeight;
	CHIP8::byte pixels[pixelCount * 4];
	for (unsigned int i = 0; i < CHIP8::Chip8Emulator::ScreenWidth; ++i)
    {
		for (unsigned int j = 0; j < CHIP8::Chip8Emulator::ScreenHeight; ++j)
	    {
	    	CHIP8::word chip8PixelIndex = (i * CHIP8::Chip8Emulator::ScreenHeight) + j;
	    	CHIP8::byte color = screenData[chip8PixelIndex] == 0 ? 0 : 0xFF;
	    	unsigned int index = (j * CHIP8::Chip8Emulator::ScreenWidth + i) * 4;
	        pixels[index] = color;
	        pixels[index + 1] = color;
	        pixels[index + 2] = color;
	        pixels[index + 3] = 0xFF;
	    }
	}

	texture.update(pixels);

	sprite.setPosition(0, 0);

	window.draw(sprite);
}

std::string IntToString(int i)
{
	std::stringstream stream;
	stream << i;

	return stream.str();
}

std::string IntAsHexToString(int i)
{
	std::stringstream stream;
	stream << std::hex << i;

	return stream.str();
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

	sf::Vector2u defaultWindowSize(CHIP8::Chip8Emulator::ScreenWidth * RenderScale, CHIP8::Chip8Emulator::ScreenHeight * RenderScale);
	sf::Vector2u debugWindowSize(defaultWindowSize.x + 200, defaultWindowSize.y + 16 * (24 + 2));
	sf::RenderWindow window(sf::VideoMode(defaultWindowSize.x, defaultWindowSize.y), "CHIP-8");
	const sf::View& defaultWindowView = window.getDefaultView();
	const sf::View debugWindowView(sf::FloatRect(0, 0, debugWindowSize.x, debugWindowSize.y));

	sf::Texture texture;
    texture.create(CHIP8::Chip8Emulator::ScreenWidth, CHIP8::Chip8Emulator::ScreenHeight);

	sf::Sprite sprite;
    sprite.setTexture(texture, true);
    sprite.setScale(sf::Vector2f(RenderScale, RenderScale));

    sf::SoundBuffer beepBuffer;
    if (beepBuffer.loadFromFile("../resources/sounds/beep.wav") == false)
    {
    	std::cerr << "Error loading sound." << std::endl;
    	return EXIT_FAILURE;
    }

    sf::Sound beep;
    beep.setBuffer(beepBuffer);

    sf::Font font;
    if (font.loadFromFile("../resources/fonts/8bit16.ttf") == false)
    {
    	std::cerr << "Error loading font." << std::endl;
    	return EXIT_FAILURE;
    }

    SceneGraph::GroupNode root;
    sf::Transform rootTransform;
    rootTransform.translate(0, CHIP8::Chip8Emulator::ScreenHeight * RenderScale + 10);
    root.setTransform(rootTransform);

    SceneGraph::TextNode registerTextNodes[16];
    for (int i = 0; i < 16; ++i)
    {
        sf::Text& text = registerTextNodes[i].getText();
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        text.setPosition(0, i * (24 + 1));	

        root.addChild(&registerTextNodes[i]);
    }

    bool debugMode = false;
    bool runEmulator = true;
    bool updateOneTime = false;
    CHIP8::word initialCyclesPerFrame = chip8.getCyclesPerFrame();
    CHIP8::word cyclesPerFrame = 1;

    sf::Clock clock;
	const unsigned int fps = 60;
	const float interval = 1000 * 1000.0f / fps;

	// Main loop.
    while (window.isOpen())
    {
    	window.clear();

    	// Process events.
        sf::Event event;
        while (window.pollEvent(event))
        {
        	// Close the window.
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

	        // Toggle emulator running.
            if (event.type == sf::Event::KeyReleased)
            {
            	if (event.key.code == sf::Keyboard::Space)
            	{
            		runEmulator = !runEmulator;
            		chip8.setCyclesPerFrame(runEmulator == true ? initialCyclesPerFrame : cyclesPerFrame);
            	}

            	if (event.key.code == sf::Keyboard::Right && runEmulator == false)
            	{
            		updateOneTime = true;
            	}

            	if (event.key.code == sf::Keyboard::Up)
            	{
            		++cyclesPerFrame;
            	}

            	if (event.key.code == sf::Keyboard::Down && cyclesPerFrame > 0)
            	{
            		--cyclesPerFrame;
            	}

            	if (event.key.code == sf::Keyboard::B)
            	{
            		if (debugMode == true)
            		{
            			window.setSize(defaultWindowSize);
            			window.setView(defaultWindowView);
            			debugMode = false;
            		}
            		else
            		{
            			window.setSize(debugWindowSize);
            			window.setView(debugWindowView);
            			debugMode = true;
            		}
            	}
            }
        }

        // Start the clock to track CHIP-8 process time.
        clock.restart();

        if (runEmulator == true || updateOneTime == true)
        {
        	HandleChip8Inputs(chip8.getKeys());

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

        	updateOneTime = false;
        }

        DrawChip8(window, sprite, texture, chip8.getScreenData());
        if (debugMode == true)
        {
        	for (int i = 0; i < 16; ++i)
        	{
        		sf::Text& text = registerTextNodes[i].getText();
        		text.setString(sf::String("V[") + sf::String(IntAsHexToString(i)) + sf::String("] = ") + sf::String(IntAsHexToString(chip8.getDataRegister(i))));
        	}
        	root.draw(window, sf::Transform::Identity);
        }

        window.display();

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