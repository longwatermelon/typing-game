#pragma once
#include <SDL.h>


struct v2d
{
	float x, y;
};


class Particle
{
public:
	Particle(int x, int y, const v2d& vec)
		: x(x), y(y), vec(vec) {}

	void move()
	{
		x += (int)vec.x;
		y += (int)vec.y;

		vec.y += 0.3f;
	}


	void render(SDL_Renderer* rend)
	{
		SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
		SDL_RenderDrawPoint(rend, x, y);
		SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
	}


	bool offscreen(int sw, int sh)
	{
		return (x < 0 || x > sw || y < 0 || y > sh);
	}

private:
	int x, y;
	v2d vec;
};