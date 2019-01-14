#include "VM.h"
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cout << "Usage: CHIP-8 ROM-path" << std::endl;
		return 0;
	}
	std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	if(file.read(reinterpret_cast<char*>(buffer.data()), size))
	{
		try
		{
			VM vm;
			vm.loadProgram(buffer.data(), 0x200, size);
			vm.run(0x200);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return -1;
		}
	}

	return 0;
}
