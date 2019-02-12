#pragma once
#include <cstdint>

class Keyboard
{
public:
	static bool isKeyPressed(uint8_t key);
	static uint8_t waitForKeyPress();
};
