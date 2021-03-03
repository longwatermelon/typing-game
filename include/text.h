#pragma once
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

const constexpr int CHAR_WIDTH = 15;
const constexpr int CHAR_HEIGHT = 20;


class Text
{
public:
	Text(int x, int y, const std::string& contents)
		: contents(contents)
	{
		rect = { x, y, (int)(contents.size()) * CHAR_WIDTH, CHAR_HEIGHT };
		f = TTF_OpenFont("OpenSans-Regular.ttf", 100);
	}

	
	void move(int x, int y)
	{
		if (!this) return;

		rect.x += x;
		rect.y += y;
	}


	void render(SDL_Renderer* rend)
	{
		std::string s1 = contents.substr(0, index).c_str();
		std::string s2 = contents.substr(index, contents.size()).c_str();

		SDL_Surface* surf = TTF_RenderText_Solid(f, s1.c_str(), { 255, 0, 0 });
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);

		SDL_Rect temp = { rect.x, rect.y, (int)strlen(s1.c_str()) * CHAR_WIDTH, CHAR_HEIGHT };
		SDL_RenderCopy(rend, tex, 0, &temp);

		surf = TTF_RenderText_Solid(f, s2.c_str(), { 255, 255, 255 });
		tex = SDL_CreateTextureFromSurface(rend, surf);

		temp = { rect.x + (int)strlen(s1.c_str()) * CHAR_WIDTH, rect.y, (int)strlen(s2.c_str()) * CHAR_WIDTH, CHAR_HEIGHT };
		SDL_RenderCopy(rend, tex, 0, &temp);

		SDL_FreeSurface(surf);
		SDL_DestroyTexture(tex);
	}


	std::string str() { return contents; }

	int idx() { return index; }

	void increment() { index++; }

	SDL_Rect rct() { return rect; }

private:
	SDL_Rect rect;
	std::string contents;
	int index = 0;

	TTF_Font* f = nullptr;
};