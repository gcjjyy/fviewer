#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define JOONG_INDEX (160)
#define JONG_INDEX (160 + 88)

int WINDOW_WIDTH = 320;
int WINDOW_HEIGHT = 320 + (16 * 4);
const int MAX_FILESIZE = 65536;

const char g_choseongType[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0};
const char g_choseongTypeJongseongExist[] = {0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5};
const char g_jongseongType[] = {0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1};

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

FILE *fp;
uint8_t font_data[MAX_FILESIZE];
int filesize = 0;
bool isEng = false;
int scale = 1;

void put_glyph_eng(int x, int y, uint8_t *font_data)
{
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			if (font_data[i] & (0x80 >> j)) {
				SDL_Rect rect = {x + (j * scale), y + (i * scale), scale, scale};
				SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
			}
		}
	}
}

void put_glyph_kor(int x, int y, uint8_t *font_data)
{
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			if (font_data[i << 1] & (0x80 >> j)) {
				SDL_Rect rect = {x + (j * scale), y + (i * scale), scale, scale};
				SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
			}
		}
		for (int j = 0; j < 8; j++) {
			if (font_data[(i << 1) + 1] & (0x80 >> j)) {
				SDL_Rect rect = {x + ((j + 8) * scale), y + (i * scale), scale, scale};
				SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));
			}
		}
	}
}

void redraw()
{
	const char *samples[4] = {
		"가나다라마바사 아자차카타파하",
		"다람쥐 헌 쳇바퀴에 타고파",
		"동해물과 백두산이 마르고 닳도록",
		"대한사람 대한으로 길이 보전하세"
	};

	screenSurface = SDL_GetWindowSurface(window);
	SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0xff));

	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	if (filesize <= 4096) {
		isEng = true;
	}

	int x = 0;
	int y = 0;
	int korIndex = 0;
	int dataOffset = 0;
	while (!feof(fp)) {
		if (isEng) {
			fread(font_data + dataOffset, 1, 16, fp);
			put_glyph_eng(x, y, font_data + dataOffset);
			x += 8 * scale;
			dataOffset += 16;
		} else {
			fread(font_data + dataOffset, 1, 32, fp);
			put_glyph_kor(x, y, font_data + dataOffset);

			if (korIndex == 4) {
				put_glyph_kor(x, y, font_data + dataOffset);
			}

			x += 16 * scale;
			dataOffset += 32;
			korIndex++;
		}

		if (x >= (WINDOW_WIDTH * scale) - 1) {
			x = 0;
			y += 16 * scale;
		}
	}

	if (!isEng) {
		y = (WINDOW_HEIGHT - (16 * 4)) * scale;

		for (int i = 0; i < 4; i++) {
			x = 0;
			for (int j = 0; ;) {
				if (samples[i][j] == 0) break;

				if (((samples[i][j    ] & 0xe0) == 0xe0) && ((samples[i][j + 1] & 0x80) == 0x80) &&
					((samples[i][j + 2] & 0x80) == 0x80)) {
					uint8_t hibyte = 0;
					uint8_t lobyte = 0;
					hibyte |= (samples[i][j    ] & 0x0f) << 4;
					hibyte |= (samples[i][j + 1] & 0x3c) >> 2;
					lobyte |= (samples[i][j + 1] & 0x03) << 6;
					lobyte |= (samples[i][j + 2] & 0x3f);

					uint16_t index = (uint16_t)(hibyte << 8) + lobyte;
					index -= 0xac00;

					uint16_t choseong = ((index / 28) / 21) + 1;
					uint16_t joongseong = ((index / 28) % 21) + 1;
					uint16_t jongseong = index % 28;

					int32_t cho_type = (jongseong) ? g_choseongTypeJongseongExist[joongseong] : g_choseongType[joongseong];
					int32_t joong_type = ((choseong == 1 || choseong == 16) ? 0 : 1) + (jongseong ? 2 : 0);
					int32_t jong_type = g_jongseongType[joongseong];

					put_glyph_kor(x, y, &font_data[(cho_type * 20 + choseong) * 32]);
					put_glyph_kor(x, y, &font_data[(JOONG_INDEX + (joong_type * 22 + joongseong)) * 32]);

					if (jongseong) {
						put_glyph_kor(x, y, &font_data[(JONG_INDEX + (jong_type * 28 + jongseong)) * 32]);
					}

					j += 3;
					x += 16 * scale;
				}
				else {
					x += 8 * scale;
					j++;
				}
			}
			printf("\n");
			y += 16 * scale;
		}
	}

	SDL_UpdateWindowSurface(window);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s font_filename [scale(int)] <width> <height>\n", argv[0]);
		return 0;
	}

	if (argc >= 3) {
		scale = atoi(argv[2]);
	}

	if (argc >= 4) {
		WINDOW_WIDTH = atoi(argv[3]);
	}

	if (argc >= 5) {
		WINDOW_HEIGHT = atoi(argv[4]);
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		printf("File not found!\n");
		return 0;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH * scale,
							  WINDOW_HEIGHT * scale, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("SDL_CreateWindow error: %s\n", SDL_GetError());
		SDL_Quit();
		exit(0);
	}

	redraw();

	while (SDL_WaitEvent(&event) >= 0) {
		switch (event.type) {
		case SDL_KEYDOWN: {
			switch (event.key.keysym.sym) {
			case SDLK_PLUS:
			case SDLK_EQUALS:
				if (scale < 5)
					scale++;
				SDL_SetWindowSize(window, WINDOW_WIDTH * scale, WINDOW_HEIGHT * scale);
				redraw();
				break;

			case SDLK_MINUS:
				if (scale > 1)
					scale--;
				SDL_SetWindowSize(window, WINDOW_WIDTH * scale, WINDOW_HEIGHT * scale);
				redraw();
				break;

			case SDLK_ESCAPE:
				fclose(fp);
				SDL_DestroyWindow(window);
				SDL_Quit();
				return 0;
				break;
			}
		} break;

		case SDL_QUIT: {
			fclose(fp);
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 0;
		} break;
		}
	}

	printf("Unknown error exit\n");
	fclose(fp);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
