#include "VM.h"
#include "Keyboard.h"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <iostream>
#include <thread>

#define RAM_SIZE 4096
#define EMU_SPEED_HZ 500
#define EMU_SPEED_US 1000000 / EMU_SPEED_HZ
#define TIMER_HZ 60
#define TIMER_US 1000000 / TIMER_HZ

uint8_t hexChars[80] = {0xF0, 0x90, 0x90, 0x90, 0xF0,
						0x20, 0x60, 0x20, 0x20, 0x70,
						0xF0, 0x10, 0xF0, 0x80, 0xF0,
						0xF0, 0x10, 0xF0, 0x10, 0xF0,
						0x90, 0x90, 0xF0, 0x10, 0x10,
						0xF0, 0x80, 0xF0, 0x10, 0xF0,
						0xF0, 0x80, 0xF0, 0x90, 0xF0,
						0xF0, 0x10, 0x20, 0x40, 0x40,
						0xF0, 0x90, 0xF0, 0x90, 0xF0,
						0xF0, 0x90, 0xF0, 0x10, 0xF0,
						0xF0, 0x90, 0xF0, 0x90, 0x90,
						0xE0, 0x90, 0xE0, 0x90, 0xE0,
						0xF0, 0x80, 0x80, 0x80, 0xF0,
						0xE0, 0x90, 0x90, 0x90, 0xE0,
						0xF0, 0x80, 0xF0, 0x80, 0xF0,
						0xF0, 0x80, 0xF0, 0x80, 0x80};

VM::VM() :
	m_I(0),
	m_delayTmr(0),
	m_soundTmr(0),
	m_PC(0),
	m_SP(0),
	m_initIO(),
	m_display("CHIP-8", 640, 320),
	m_sound(1000)
{
	m_vRegs[0x0] = 0;
	m_vRegs[0x1] = 0;
	m_vRegs[0x2] = 0;
	m_vRegs[0x3] = 0;
	m_vRegs[0x4] = 0;
	m_vRegs[0x5] = 0;
	m_vRegs[0x6] = 0;
	m_vRegs[0x7] = 0;
	m_vRegs[0x8] = 0;
	m_vRegs[0x9] = 0;
	m_vRegs[0xA] = 0;
	m_vRegs[0xB] = 0;
	m_vRegs[0xC] = 0;
	m_vRegs[0xD] = 0;
	m_vRegs[0xE] = 0;
	m_vRegs[0xF] = 0;

	m_RAM = new uint8_t[RAM_SIZE];
	std::memcpy(m_RAM, hexChars, sizeof(hexChars));

	m_frameBuffer = new uint8_t[64 * 32];
	std::memset(m_frameBuffer, 0, 64 * 32);
}

VM::~VM()
{
	delete[] m_frameBuffer;
	delete[] m_RAM;
}

void VM::loadProgram(uint8_t *src, uint32_t dest, uint32_t size)
{
	if(dest + size > RAM_SIZE)
	{
		throw std::invalid_argument("Program doesn't fit in RAM. Dest: " + std::to_string(dest) + "  Size: " + std::to_string(size));
	}

	std::memcpy(m_RAM + dest, src, size);
}

void VM::run(uint16_t startAddr)
{
	if(startAddr >= RAM_SIZE)
	{
		throw std::invalid_argument("Invalid start address: " + std::to_string(startAddr));
	}

	m_PC = startAddr;

	auto loopTimestamp = std::chrono::high_resolution_clock::now();
	auto timerTimestamp = std::chrono::high_resolution_clock::now();

	std::srand(std::time(nullptr));
	
	// Main loop
	while(true)
	{
		// Control emulator speed
		while(true)
		{
			auto elapsed = std::chrono::high_resolution_clock::now() - loopTimestamp;
			size_t elapsedUS = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
			
			if(elapsedUS < EMU_SPEED_US)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(EMU_SPEED_US - elapsedUS));
			}
			else
			{
				break;
			}
		}

		loopTimestamp = std::chrono::high_resolution_clock::now();
		
		// Check if timers should decrement
		auto elapsed = std::chrono::high_resolution_clock::now() - timerTimestamp;

		if(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() >= TIMER_US)
		{
			if(m_delayTmr != 0)
			{
				--m_delayTmr;
			}

			if(m_soundTmr != 0)
			{
				if(--m_soundTmr == 0)
				{
					m_sound.stop();
				}
			}

			timerTimestamp = std::chrono::high_resolution_clock::now();
		}
		
		// Fetch and decode instruction
		if(m_PC >= RAM_SIZE - 1)
		{
			throw std::runtime_error("Program counter out of range: " + std::to_string(m_PC));
		}
		
		uint16_t instr = m_RAM[m_PC];
		instr <<= 8;
		instr |= m_RAM[m_PC + 1];

		// Execute instruction
		if(instr == 0x00E0) // CLS
		{
			std::memset(m_frameBuffer, 0, 64 * 32);
			m_display.draw(m_frameBuffer);
			m_PC += 2;
		}
		else if(instr == 0x00EE) // RET
		{
			m_PC = m_stack[--m_SP];
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0x0000) // SYS addr
		{
			// Ignore
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0x1000) // JP addr
		{
			m_PC = instr & 0x0FFF;
		}
		else if((instr & 0xF000) == 0x2000) // CALL addr
		{
			m_stack[m_SP++] = m_PC;
			m_PC = instr & 0x0FFF;
		}
		else if((instr & 0xF000) == 0x3000) // SE Vx, value
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t value = instr & 0x00FF;

			if(m_vRegs[x] == value)
			{
				m_PC += 2;
			}
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0x4000) // SNE Vx, value
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t value = instr & 0x00FF;

			if(m_vRegs[x] != value)
			{
				m_PC += 2;
			}
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x5000) // SE Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;

			if(m_vRegs[x] == m_vRegs[y])
			{
				m_PC += 2;
			}
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0x6000) // LD Vx, value
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t value = instr & 0x00FF;
			m_vRegs[x] = value;
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0x7000) // ADD Vx, value
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t value = instr & 0x00FF;
			m_vRegs[x] += value;
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8000) // LD Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			m_vRegs[x] = m_vRegs[y];
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8001) // OR Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			m_vRegs[x] |= m_vRegs[y];
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8002) // AND Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			m_vRegs[x] &= m_vRegs[y];
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8003) // XOR Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			m_vRegs[x] ^= m_vRegs[y];
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8004) // ADD Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			uint16_t res = m_vRegs[x] + m_vRegs[y];
			m_vRegs[0xF] = res > 255 ? 1 : 0;
			m_vRegs[x] = res;
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8005) // SUB Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			uint16_t res = m_vRegs[x] - m_vRegs[y];
			m_vRegs[0xF] = res <= 255 ? 1 : 0;
			m_vRegs[x] = res;
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8006) // SHR Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_vRegs[0xF] = m_vRegs[x] & 0x1 ? 1 : 0;
			m_vRegs[x] >>= 1;
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x8007) // SUBN Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			uint16_t res = m_vRegs[y] - m_vRegs[x];
			m_vRegs[0xF] = res <= 255 ? 1 : 0;
			m_vRegs[x] = res;
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x800E) // SHL Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_vRegs[0xF] = m_vRegs[x] & 0x8 ? 1 : 0;
			m_vRegs[x] <<= 1;
			m_PC += 2;
		}
		else if((instr & 0xF00F) == 0x9000) // SNE Vx, Vy
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;

			if(m_vRegs[x] != m_vRegs[y])
			{
				m_PC += 2;
			}
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0xA000) // LD I, addr
		{
			m_I = instr & 0x0FFF;
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0xB000) // JP V0, addr
		{
			m_PC = (instr & 0x0FFF) + m_vRegs[0x0];
		}
		else if((instr & 0xF000) == 0xC000) // RND Vx, value
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t value = instr & 0x00FF;
			m_vRegs[x] = std::rand() % 256 & value;
			m_PC += 2;
		}
		else if((instr & 0xF000) == 0xD000) // DRW Vx, Vy, size
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			uint8_t y = (instr & 0x00F0) >> 4;
			uint8_t size = instr & 0x000F;
			m_vRegs[0xF] = 0;

			for(int pixel = 0; pixel < size * 8; ++pixel)
			{
				uint8_t pixelX = pixel % 8;
				uint8_t pixelY = pixel / 8;

				// Don't do anything if pixel in sprite isn't set.
				if(!(m_RAM[m_I + pixelY] & (0x80 >> pixelX)))
				{
					continue;
				}
				
				uint8_t pixelDestX = (m_vRegs[x] + pixelX) % 64;
				uint8_t pixelDestY = (m_vRegs[y] + pixelY) % 32;

				if(m_frameBuffer[pixelDestX + pixelDestY * 64])
				{
					m_frameBuffer[pixelDestX + pixelDestY * 64] = 0;
					m_vRegs[0xF] = 1;
				}
				else
				{
					m_frameBuffer[pixelDestX + pixelDestY * 64] = 0xFF;
				}
			}
			m_display.draw(m_frameBuffer);
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xE09E) // SKP Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;

			if(Keyboard::isKeyPressed(m_vRegs[x]))
			{
				m_PC += 2;
			}
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xE0A1) // SKNP Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;

			if(!Keyboard::isKeyPressed(m_vRegs[x]))
			{
				m_PC += 2;
			}
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF007) // LD Vx, DT
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_vRegs[x] = m_delayTmr;
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF00A) // LD Vx, key
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_vRegs[x] = Keyboard::waitForKeyPress();
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF015) // LD DT, Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_delayTmr = m_vRegs[x];
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF018) // LD ST, Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_soundTmr = m_vRegs[x];

			if(m_soundTmr != 0)
			{
				m_sound.play();
			}
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF01E) // ADD I, Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_I += m_vRegs[x];
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF029) // LD F, Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_I = m_vRegs[x] * 5;
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF033) // LD B, Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;
			m_RAM[m_I] = m_vRegs[x] / 100;
			m_RAM[m_I + 1] = (m_vRegs[x] / 10) % 10;
			m_RAM[m_I + 2] = m_vRegs[x] % 100 % 10;
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF055) // LD [I], Vx
		{
			uint8_t x = (instr & 0x0F00) >> 8;

			for(uint8_t i = 0; i <= x; ++i)
			{
				m_RAM[m_I + i] = m_vRegs[i];
			}
			m_PC += 2;
		}
		else if((instr & 0xF0FF) == 0xF065) // LD Vx, [I]
		{
			uint8_t x = (instr & 0x0F00) >> 8;

			for(uint8_t i = 0; i <= x; ++i)
			{
				m_vRegs[i] = m_RAM[m_I + i];
			}
			m_PC += 2;
		}
		else
		{
			throw std::runtime_error("Illegal instruction: " + std::to_string(instr));
		}
	}
}
