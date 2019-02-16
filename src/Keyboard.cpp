#include "Keyboard.h"
#include <SDL2/SDL.h>
#include <thread>

bool Keyboard::isKeyPressed(uint8_t key)
{
	SDL_PumpEvents();
	const uint8_t *state = SDL_GetKeyboardState(NULL);

	switch(key)
	{
	case 0:
		return state[SDL_SCANCODE_X];
	case 1:
		return state[SDL_SCANCODE_1];
	case 2:
		return state[SDL_SCANCODE_2];
	case 3:
		return state[SDL_SCANCODE_3];
	case 4:
		return state[SDL_SCANCODE_Q];
	case 5:
		return state[SDL_SCANCODE_W];
	case 6:
		return state[SDL_SCANCODE_E];
	case 7:
		return state[SDL_SCANCODE_A];
	case 8:
		return state[SDL_SCANCODE_S];
	case 9:
		return state[SDL_SCANCODE_D];
	case 10:
		return state[SDL_SCANCODE_Z];
	case 11:
		return state[SDL_SCANCODE_C];
	case 12:
		return state[SDL_SCANCODE_4];
	case 13:
		return state[SDL_SCANCODE_R];
	case 14:
		return state[SDL_SCANCODE_F];
	case 15:
		return state[SDL_SCANCODE_V];
	default:
		return false;
	}
}

uint8_t Keyboard::waitForKeyPress()
{
	while(true)
	{
		SDL_PumpEvents();
		const uint8_t *state = SDL_GetKeyboardState(NULL);

		if(state[SDL_SCANCODE_X])
		{
			return 0;
		}
		else if(state[SDL_SCANCODE_1])
		{
			return 1;
		}
		else if(state[SDL_SCANCODE_2])
		{
			return 2;
		}
		else if(state[SDL_SCANCODE_3])
		{
			return 3;
		}
		else if(state[SDL_SCANCODE_Q])
		{
			return 4;
		}
		else if(state[SDL_SCANCODE_W])
		{
			return 5;
		}
		else if(state[SDL_SCANCODE_E])
		{
			return 6;
		}
		else if(state[SDL_SCANCODE_A])
		{
			return 7;
		}
		else if(state[SDL_SCANCODE_S])
		{
			return 8;
		}
		else if(state[SDL_SCANCODE_D])
		{
			return 9;
		}
		else if(state[SDL_SCANCODE_Z])
		{
			return 10;
		}
		else if(state[SDL_SCANCODE_C])
		{
			return 11;
		}
		else if(state[SDL_SCANCODE_4])
		{
			return 12;
		}
		else if(state[SDL_SCANCODE_R])
		{
			return 13;
		}
		else if(state[SDL_SCANCODE_F])
		{
			return 14;
		}
		else if(state[SDL_SCANCODE_V])
		{
			return 15;
		}

		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
}
