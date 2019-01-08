#include "VM.h"
#include <cstdint>
#include <fstream>
#include <vector>

int main(int argc, char* args[])
{
	std::ifstream file("PONG", std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	if(file.read(reinterpret_cast<char*>(buffer.data()), size))
	{
		VM vm;
		vm.loadProgram(buffer.data(), 0x200, size);
		vm.run(0x200);
	}

	return 0;
}
