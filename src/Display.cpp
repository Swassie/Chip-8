#include "Display.h"
#include <iostream>

Display::Display(std::string title, int width, int height)
{
	SDL_CreateWindowAndRenderer(width, height, 0, &m_window, &m_renderer);

	if(!m_window || !m_renderer)
	{
		std::cout << "Window or renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return;
	}

	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(m_renderer, width, height);
	SDL_RenderClear(m_renderer);
	SDL_RenderPresent(m_renderer);

	m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	if(!m_texture)
	{
		std::cout << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return;
	}
}

Display::~Display()
{
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
}

void Display::draw(uint8_t *frameBuffer)
{
	SDL_UpdateTexture(m_texture, NULL, frameBuffer, 64);
	SDL_RenderClear(m_renderer);
	SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
	SDL_RenderPresent(m_renderer);
}
