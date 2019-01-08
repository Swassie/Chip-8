#include "Display.h"
#include <cstdint>
#include <map>

#define STACK_LEVELS 16

class VM
{
public:
	VM();
	~VM();
	void loadProgram(uint8_t *src, uint32_t dest, uint32_t size);
	void run(uint16_t startAddr);

private:
	std::map<uint8_t, uint8_t> m_vRegs;
	uint16_t m_I;
	uint8_t m_delayTmr;
	uint8_t m_soundTmr;
	uint16_t m_PC;
	uint8_t m_SP;
	uint16_t m_stack[STACK_LEVELS];

	uint8_t *m_RAM;
	uint8_t *m_frameBuffer;
	Display m_display;
};
