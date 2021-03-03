#include "../include/text.h"
#include "../include/particle.h"
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <chrono>
#include <SDL.h>
#include <SDL_ttf.h>


void spawn_text(std::vector<std::shared_ptr<Text>>& vec, int w, int h, std::vector<std::string>& words, std::mutex& mtx, bool& running)
{
	std::random_device rd;
	std::mt19937 rng(rd());

	int delay = 1000;

	while (running)
	{
		std::uniform_int_distribution<int> uni(0, h - CHAR_HEIGHT * 4);

		int y = uni(rng);

		std::uniform_int_distribution<int> unif(0, static_cast<int>(words.size()) - 1);

		{
			std::lock_guard lock(mtx);
			vec.push_back(std::make_shared<Text>(0, y, words[unif(rng)]));
		}

		//SDL_Delay(delay);
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));

		if (delay >= 1) --delay;
	}
}


void explode(SDL_Renderer* rend, const SDL_Rect& rect, std::vector<Particle>& particles)
{
	std::random_device rd;
	std::mt19937 rng(rd());

	for (int i = rect.x; i < rect.x + rect.w; i++)
	{
		std::uniform_real_distribution<float> xuni(-5.0f, 5.0f);
		std::uniform_real_distribution<float> yuni(-10.0f, -3.0f);

		std::uniform_int_distribution<int> yluni(rect.y, rect.y + rect.h);
		particles.emplace_back(Particle(i, yluni(rng), { xuni(rng), yuni(rng) }));
	}
}


void text(SDL_Renderer* rend, TTF_Font* font, const std::string& text, int x, int y)
{
	SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), { 255, 255, 255 });
	SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);

	SDL_Rect temp = { x, y, text.size() * CHAR_WIDTH, CHAR_HEIGHT };
	SDL_RenderCopy(rend, tex, 0, &temp);

	SDL_FreeSurface(surf);
	SDL_DestroyTexture(tex);
}


int main(int argc, char* argv[])
{
	bool running = true;

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	int screenw = 1000, screenh = 800;
	SDL_Window* window = SDL_CreateWindow("sample text", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenw, screenh, SDL_WINDOW_SHOWN);
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);


	std::vector<std::shared_ptr<Text>> vtext;
	std::vector<Particle> particles;


	int completed = 0;
	TTF_Font* font = TTF_OpenFont("OpenSans-Regular.ttf", 100);


	std::mutex mtx;
	std::vector<std::string> words = { "boys", "loss", "thin", "idea", "befall", "force", "sister", "construe", "yielding", "day", "abide", "daffy", "abaft", "measure", "selfish" };
	std::thread thr_spawn(spawn_text, std::ref(vtext), screenw, screenh, std::ref(words), std::ref(mtx), std::ref(running));


	std::shared_ptr<Text> selected;

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
					for (int i = vtext.size() - 1; i > -1; i--)
					{
						auto& t = vtext[i];

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
			++completed;
			explode(rend, vtext[erase_index]->rct(), particles);

			vtext.erase(vtext.begin() + erase_index);
			selected = nullptr;
		}


		SDL_RenderClear(rend);

		for (int i = 0; i < vtext.size(); i++)
		{
			auto& t = vtext[i];

			t->move(1, 0);

			if (t->offscreen(screenw, screenh))
			{
				if (selected == t) selected = nullptr;

				vtext.erase(vtext.begin() + i);
			}
			else t->render(rend);
		}

		for (int i = 0; i < particles.size(); i++)
		{
			auto& p = particles[i];

			p.move();

			if (p.offscreen(screenw, screenh)) { particles.erase(particles.begin() + i); }
			else p.render(rend);
		}

		text(rend, font, "Words completed: " + std::to_string(completed), 0, screenh - CHAR_WIDTH - 5);

		SDL_RenderPresent(rend);

		SDL_Delay(10);
	}

	if (thr_spawn.joinable()) thr_spawn.join();

	SDL_Quit();

	return 0;
}