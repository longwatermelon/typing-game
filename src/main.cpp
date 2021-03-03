#include "../include/text.h"
#include "../include/particle.h"
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <SDL.h>
#include <SDL_ttf.h>


#include <iostream>

void spawn_text(std::vector<std::shared_ptr<Text>>& vec, int w, int h, std::vector<std::string>& words, std::mutex& mtx, bool& running)
{
	std::random_device rd;
	std::mt19937 rng(rd());

	while (running)
	{

		std::uniform_int_distribution<int> uni(0, h);

		int y = uni(rng);

		std::uniform_int_distribution<int> unif(0, (int)words.size() - 1);

		{
			std::lock_guard lock(mtx);
			vec.push_back(std::make_shared<Text>(0, y, words[unif(rng)]));
		}

		SDL_Delay(1000);
	}
}


void explode(SDL_Renderer* rend, const SDL_Rect& rect, std::vector<Particle>& particles)
{
	std::random_device rd;
	std::mt19937 rng(rd());

	for (int i = rect.x; i < rect.x + rect.w; i++)
	{
		std::uniform_int_distribution<int> xuni(-5, 5);
		std::uniform_int_distribution<int> yuni(-7, -3);

		std::uniform_int_distribution<int> yluni(rect.y, rect.y + rect.h);
		particles.emplace_back(Particle(i, yluni(rng), { (float)xuni(rng), (float)yuni(rng) }));
	}
}


int main(int argc, char* argv[])
{
	bool running = true;

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	int screenw = 700, screenh = 500;
	SDL_Window* window = SDL_CreateWindow("sample text", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenw, screenh, SDL_WINDOW_SHOWN);
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	std::vector<std::shared_ptr<Text>> vtext;
	std::vector<Particle> particles;

	std::vector<std::string> words = { "boys", "loss", "thin", "idea", "befall" };

	std::mutex mtx;
	std::thread thr_spawn(spawn_text, std::ref(vtext), screenw, screenh, std::ref(words), std::ref(mtx), std::ref(running));

	std::shared_ptr<Text> selected;
	auto index = 0;

	SDL_Event evt;

	while (running)
	{
		while (SDL_PollEvent(&evt))
		{
			switch (evt.type)
			{
			case SDL_QUIT: running = false; break;
			case SDL_TEXTINPUT: {
				if (!selected)
				{
					for (auto& t : vtext)
					{
						if (t->str()[0] == evt.text.text[0])
						{
							selected = t;
						}
					}
				}

				if (selected)
				{
					if (selected->str()[selected->idx()] == evt.text.text[0])
					{
						selected->increment();
					}
				}
			} break;
			}
		}

		int erase_index = -1;
		for (int i = 0; i < vtext.size(); i++)
		{
			if (vtext[i]->idx() >= (int)vtext[i]->str().size())
			{
				erase_index = i;
				break;
			}
		}

		if (erase_index != -1)
		{
			explode(rend, vtext[erase_index]->rct(), particles);

			vtext.erase(vtext.begin() + erase_index);
			selected = nullptr;
		}


		SDL_RenderClear(rend);

		for (auto& t : vtext)
		{
			t->move(1, 0);
			t->render(rend);
		}

		std::vector<int> removed;
		for (int i = 0; i < particles.size(); i++)
		{
			auto& p = particles[i];

			p.move();

			if (p.offscreen(screenw, screenh)) { particles.erase(particles.begin() + i); }
			else p.render(rend);
		}
		

		SDL_RenderPresent(rend);
	}

	if (thr_spawn.joinable()) thr_spawn.join();

	SDL_Quit();
	TTF_Quit();

	return 0;
}