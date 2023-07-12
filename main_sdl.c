/*
	Copyright (C) 2023 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License version 2 as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "map.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

#define MAX_TEXTURES 32
#define MAX_SOUNDS 8

struct sdl_context
{
	SDL_Window *win;
	SDL_Renderer *renderer;
	int n_textures;
	const char *textures_paths[MAX_TEXTURES];
	SDL_Texture *textures[MAX_TEXTURES];
	int n_sounds;
	const char *sounds_paths[MAX_SOUNDS];
	Mix_Chunk *sounds[MAX_SOUNDS];
};

void fail(const char *msg)
{
	fputs(msg, stderr);
	exit(1);
}

// Setup SDL subsystems and create a window & a renderer.
void init_context(struct sdl_context *ctx)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
		fail("couldn't init sdl_video & sdl_audio");

	if (IMG_Init(IMG_INIT_PNG) < 0)
		fail("couldn't init sdl_image");

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
		fail("couldn't open audio device");

	ctx->n_sounds = 0;
	ctx->n_textures = 0;

	ctx->win = SDL_CreateWindow(
		"viborita",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640, 480,
		0
	);

	ctx->renderer = SDL_CreateRenderer(
		ctx->win,
		-1,
		SDL_RENDERER_ACCELERATED |
			SDL_RENDERER_PRESENTVSYNC
	);
}

// Release previously allocated sdl resources.
void fini_context(struct sdl_context *ctx)
{
	for (size_t i = 0; i < ctx->n_textures; ++i)
		SDL_DestroyTexture(ctx->textures[i]);

	for (size_t i = 0; i < ctx->n_sounds; ++i)
		Mix_FreeChunk(ctx->sounds[i]);

	SDL_DestroyRenderer(ctx->renderer);
	SDL_DestroyWindow(ctx->win);

	IMG_Quit();
	SDL_Quit();
}

// Load a texture and return an id that identifies that texture.
int load_texture(struct sdl_context *ctx, const char *texture_path)
{
	for (size_t i = 0; i < ctx->n_textures; ++i)
		if (strcmp(ctx->textures_paths[i], texture_path) == 0)
			return i;

	if (ctx->n_textures == MAX_TEXTURES)
		fail("max textures limit reached");

	ctx->textures_paths[ctx->n_textures] = texture_path;
	ctx->textures[ctx->n_textures] = IMG_LoadTexture(ctx->renderer,
			texture_path);
	ctx->n_textures += 1;

	return ctx->n_textures - 1;
}

// Load a sound and return an id that identifies that sound.
int load_sound(struct sdl_context *ctx, const char *sound_path)
{
	for (size_t i = 0; i < ctx->n_sounds; ++i)
		if (strcmp(ctx->sounds_paths[i], sound_path) == 0)
			return i;

	if (ctx->n_sounds == MAX_SOUNDS)
		fail("max sounds limit reached");

	ctx->sounds_paths[ctx->n_sounds] = sound_path;
	ctx->sounds[ctx->n_sounds] = Mix_LoadWAV(sound_path);
	ctx->n_sounds += 1;

	return ctx->n_sounds - 1;
}

void play_sound(const struct sdl_context *ctx, int id)
{
	if (id >= ctx->n_sounds || id < 0)
		fail("unknown sound id");

	Mix_PlayChannel(-1, ctx->sounds[id], 0);
}

void render_texture(const struct sdl_context *ctx, int id, int x, int y,
		int w, int h)
{
	if (id >= ctx->n_textures || id < 0)
		fail("unknown texture id");

	SDL_Rect rect = {
		.x = x,
		.y = y,
		.w = w,
		.h = h
	};

	SDL_RenderCopy(
		ctx->renderer,
		ctx->textures[id],
		NULL,
		&rect
	);
}

void render_rect(const struct sdl_context *ctx, int x, int y, int w, int h,
		uint32_t color)
{
	int r = (color >> 16) & 0xff,
		g = (color >>  8) & 0xff,
		b = (color >>  0) & 0xff;

	SDL_Rect rect = {
		.x = x,
		.y = y,
		.w = w,
		.h = h
	};

	SDL_SetRenderDrawColor(ctx->renderer, r, g, b, 0xff);
	SDL_RenderFillRect(ctx->renderer, &rect);
}

void begin_draw(struct sdl_context *ctx)
{
	SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
	SDL_RenderClear(ctx->renderer);
}

void end_draw(struct sdl_context *ctx)
{
	SDL_RenderPresent(ctx->renderer);
}

void get_window_size(struct sdl_context *ctx, int *ww, int *wh)
{
	SDL_GetWindowSize(ctx->win, ww, wh);
}

void delay(const struct sdl_context *ctx, int ms)
{
	(void) ctx;
	SDL_Delay(ms);
}

void render_map(struct sdl_context *ctx, struct map *map, int cz)
{
	int food_texture            = load_texture(ctx, "./gfx/apple.png");

	int vib_texture_down_left   = load_texture(ctx, "./gfx/body_down_left.png");
	int vib_texture_down_right  = load_texture(ctx, "./gfx/body_down_right.png");
	int vib_texture_horizontal  = load_texture(ctx, "./gfx/body_horizontal.png");
	int vib_texture_up_left     = load_texture(ctx, "./gfx/body_up_left.png");
	int vib_texture_up_right    = load_texture(ctx, "./gfx/body_up_right.png");
	int vib_texture_vertical    = load_texture(ctx, "./gfx/body_vertical.png");

	int vib_texture_tail_left   = load_texture(ctx, "./gfx/tail_left.png");
	int vib_texture_tail_right  = load_texture(ctx, "./gfx/tail_right.png");
	int vib_texture_tail_down   = load_texture(ctx, "./gfx/tail_down.png");
	int vib_texture_tail_up     = load_texture(ctx, "./gfx/tail_up.png");

	int vib_texture_head_left   = load_texture(ctx, "./gfx/head_left.png");
	int vib_texture_head_right  = load_texture(ctx, "./gfx/head_right.png");
	int vib_texture_head_down   = load_texture(ctx, "./gfx/head_down.png");
	int vib_texture_head_up     = load_texture(ctx, "./gfx/head_up.png");

	int ww, wh;
	get_window_size(ctx, &ww, &wh);
	int cam_x = map->head_col * cz - (ww - cz) / 2;
	int cam_y = map->head_row * cz - (wh - cz) / 2;

	// Render background (space).
	for (size_t x = 0; x < map->n_cols; ++x)
	{
		for (size_t y = 0; y < map->n_rows; ++y)
		{
			render_rect(
				ctx,
				x * cz - cam_x,
				y * cz - cam_y,
				cz,
				cz,
				0x090909 * ((x + y) % 2 == 0)
			);
		}
	}

	// Render snake.
	size_t pr, pc, r, c, nr, nc;
	enum map_block_type prev, cur, next;
	int is_head = 0, is_tail;

	r = map->tail_row;
	c = map->tail_col;

	while (!is_head)
	{
		int text = -1;

		is_tail = c == map->tail_col && r == map->tail_row;
		is_head = c == map->head_col && r == map->head_row;

		map_find_snake_next_block(map, r, c, &nr, &nc);

		cur = map->map[r][c];

		if (is_tail) switch (cur)
		{
			case MAP_BLOCK_SNAKE_LEFT:
				text = vib_texture_tail_left;
				break;
			case MAP_BLOCK_SNAKE_RIGHT:
				text = vib_texture_tail_right;
				break;
			case MAP_BLOCK_SNAKE_UP:
				text = vib_texture_tail_up;
				break;
			case MAP_BLOCK_SNAKE_DOWN:
				text = vib_texture_tail_down;
				break;
		}

		if (is_head) switch (cur)
		{
			case MAP_BLOCK_SNAKE_LEFT:
				text = vib_texture_head_left;
				break;
			case MAP_BLOCK_SNAKE_RIGHT:
				text = vib_texture_head_right;
				break;
			case MAP_BLOCK_SNAKE_UP:
				text = vib_texture_head_up;
				break;
			case MAP_BLOCK_SNAKE_DOWN:
				text = vib_texture_head_down;
				break;
		}

		if (!is_head && !is_tail)
		{
			next = map->map[nr][nc];
			prev = map->map[pr][pc];

			if (cur == prev)
			{
				switch (cur)
				{
					case MAP_BLOCK_SNAKE_DOWN:
					case MAP_BLOCK_SNAKE_UP:
						text = vib_texture_vertical;
						break;
					case MAP_BLOCK_SNAKE_LEFT:
					case MAP_BLOCK_SNAKE_RIGHT:
						text = vib_texture_horizontal;
						break;
				}
			}
			else
			{
				if ((prev == MAP_BLOCK_SNAKE_DOWN
						&& cur == MAP_BLOCK_SNAKE_LEFT)
						|| (prev == MAP_BLOCK_SNAKE_RIGHT
							&& cur == MAP_BLOCK_SNAKE_UP))
				{
					text = vib_texture_down_left;
				}
				else if ((prev == MAP_BLOCK_SNAKE_UP
							&& cur == MAP_BLOCK_SNAKE_LEFT)
						|| (prev == MAP_BLOCK_SNAKE_RIGHT
							&& cur == MAP_BLOCK_SNAKE_DOWN))
				{
					text = vib_texture_up_left;
				}
				else if ((prev == MAP_BLOCK_SNAKE_DOWN
							&& cur == MAP_BLOCK_SNAKE_RIGHT)
						|| (prev == MAP_BLOCK_SNAKE_LEFT
							&& cur == MAP_BLOCK_SNAKE_UP))
				{
					text = vib_texture_down_right;
				}
				else if ((prev == MAP_BLOCK_SNAKE_UP
							&& cur == MAP_BLOCK_SNAKE_RIGHT)
						|| (prev == MAP_BLOCK_SNAKE_LEFT
							&& cur == MAP_BLOCK_SNAKE_DOWN))
				{
					text = vib_texture_up_right;
				}
			}
		}

		render_texture(ctx, text, c * cz - cam_x, r * cz - cam_y, cz, cz);

		pr = r;
		pc = c;
		c = nc;
		r = nr;
	}

	// Render walls and food.
	MAP_FOR_EACH_BLOCK(map, row, col, block) switch (block)
	{
		case MAP_BLOCK_FOOD:
			render_texture(
				ctx,
				food_texture,
				col * cz - cam_x,
				row * cz - cam_y,
				cz,
				cz
			);
			break;
		case MAP_BLOCK_WALL:
			render_rect(ctx, col*cz - cam_x, row*cz-cam_y, cz, cz, 0x349eeb);
			break;
	}
}

int
main(int argc, char **argv)
{
	int score = 0;
	struct map map;
	struct sdl_context sdl_context;
	enum map_block_type dir;
	enum map_snake_state state;
	int c;
	SDL_Event event;
	bool paused = false;

	if (argc < 2 || map_parse_file(&map, argv[1]) < 0)
		return 1;

	init_context(&sdl_context);

	while (1)
	{
		dir = MAP_BLOCK_INVALID;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT: return 0;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_h: dir = MAP_BLOCK_SNAKE_LEFT;  break;
						case SDLK_j: dir = MAP_BLOCK_SNAKE_DOWN;  break;
						case SDLK_k: dir = MAP_BLOCK_SNAKE_UP;    break;
						case SDLK_l: dir = MAP_BLOCK_SNAKE_RIGHT; break;
						case SDLK_SPACE: paused = !paused; break;
					}
					break;
			}
		}

		if (dir != MAP_BLOCK_INVALID)
		{
			map_set_snake_direction(&map, dir);
			paused = false;
		}

		if (!paused)
		{
			map_advance(&map, &state);

			switch (state)
			{
				case MAP_SNAKE_EATING:
					play_sound(
						&sdl_context,
						load_sound(
							&sdl_context,
							"./sfx/chomp.wav"
						)
					);
					map_spawn_food(&map);
					score += 1;
					break;
				case MAP_SNAKE_DEAD:
					play_sound(
						&sdl_context,
						load_sound(
							&sdl_context,
							"./sfx/death.wav"
						)
					);
					score = 0;
					map_parse_file(&map, argv[1]);
					break;
			}
		}

		begin_draw(&sdl_context);
		render_map(&sdl_context, &map, 40);
		end_draw(&sdl_context);
		delay(&sdl_context, 50);
	}

	fini_context(&sdl_context);

	return 0;
}
