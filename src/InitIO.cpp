#include "InitIO.h"
#include <iostream>
#include <SDL2/SDL.h>

InitIO::InitIO()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
	}
}

InitIO::~InitIO()
{
	SDL_Quit();
}
