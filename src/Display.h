#include <cstdint>
#include <string>
#include <SDL2/SDL.h>

class Display
{
public:
	Display(std::string title, int width, int height);
	~Display();
	void draw(uint8_t *frameBuffer);

private:
	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
	SDL_Texture *m_texture;
};
