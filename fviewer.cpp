#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 320;

uint8_t font_data[32];
int filesize = 0;
bool isEng = false;
int scale = 1;

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s font_filename [scale(int)]\n", argv[0]);
		return 0;
	}

	if (argc >= 3) {
		scale = atoi(argv[2]);
	}

	FILE *fp = fopen(argv[1], "r");
	if (!fp)
	{
		printf("File not found!\n");
		return 0;
	}

	SDL_Window *window = NULL;
	SDL_Surface *screenSurface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH * scale, WINDOW_HEIGHT * scale, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("SDL_CreateWindow error: %s\n", SDL_GetError());
		SDL_Quit();
		return 0;
	}

	screenSurface = SDL_GetWindowSurface(window);
	SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0xff));

	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	if (filesize <= 4096)
	{
		isEng = true;
	}

	int x = 0;
	int y = 0;
	while (!feof(fp))
	{
		if (isEng)
		{
			fread(font_data, 1, 16, fp);

			for (int i = 0; i < 16; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					if (font_data[i] & (0x80 >> j))
					{
						SDL_Rect rect = {x + (j * scale), y + (i * scale), scale, scale};
						SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
					}
				}
			}

			x += 8 * scale;
		}
		else
		{
			fread(font_data, 1, 32, fp);
			for (int i = 0; i < 16; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					if (font_data[i << 1] & (0x80 >> j))
					{
						SDL_Rect rect = {x + (j * scale), y + (i * scale), scale, scale};
						SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
					}
				}
				for (int j = 0; j < 8; j++)
				{
					if (font_data[(i << 1) + 1] & (0x80 >> j))
					{
						SDL_Rect rect = {x + ((j + 8) * scale), y + (i * scale), scale, scale};
						SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
					}
				}
			}
			x += 16 * scale;
		}

		if (x >= (WINDOW_WIDTH * scale) - 1)
		{
			x = 0;
			y += 16 * scale;
		}
	}

	SDL_UpdateWindowSurface(window);

	SDL_Delay(5000);

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
