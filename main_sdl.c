/*
	Copyright (C) 2023 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License version 2 as published by the
	Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

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

struct sdl_context {
	SDL_Window *win;
	SDL_Renderer *renderer;
	int n_textures;
	const char *textures_paths[MAX_TEXTURES];
	SDL_Texture *textures[MAX_TEXTURES];
	int n_sounds;
	const char *sounds_paths[MAX_SOUNDS];
	Mix_Chunk *sounds[MAX_SOUNDS];
};

static void __sdl_context_destroy(struct sdl_context *ctx)
{
	SDL_DestroyRenderer(ctx->renderer);
	SDL_DestroyWindow(ctx->win);

	IMG_Quit();
	SDL_Quit();
}

static void __sdl_context_query_window_size(struct sdl_context *ctx,
		int *ww, int *wh)
{
	SDL_GetWindowSize(ctx->win, ww, wh);
}

static void __sdl_context_create(struct sdl_context *ctx)
{
	// Init video and audio subsystems.
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
		exit(1);

	// Initialize image loader to accept png images.
	if (IMG_Init(IMG_INIT_PNG) < 0)
		exit(1);

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
		exit(1);

	ctx->win = SDL_CreateWindow("xviborita", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, 640, 480, 0);

	ctx->renderer = SDL_CreateRenderer(ctx->win, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	ctx->n_sounds = ctx->n_textures = 0;
}

static void __sdl_context_begin_draw(struct sdl_context *ctx)
{
	SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);
	SDL_RenderClear(ctx->renderer);
}

static void __sdl_context_end_draw(struct sdl_context *ctx)
{
	SDL_RenderPresent(ctx->renderer);
}

static int __sdl_context_load_texture(struct sdl_context *ctx,
		const char *texture_path)
{
	for (size_t i = 0; i < ctx->n_textures; ++i)
		if (strcmp(ctx->textures_paths[i], texture_path) == 0)
			return i;

	if (ctx->n_textures == MAX_TEXTURES)
		exit(1);

	ctx->textures_paths[ctx->n_textures] = texture_path;
	ctx->textures[ctx->n_textures] = IMG_LoadTexture(ctx->renderer,
			texture_path);
	ctx->n_textures += 1;

	return ctx->n_textures - 1;
}

static int __sdl_context_load_sound(struct sdl_context *ctx,
		const char *sound_path)
{
	for (size_t i = 0; i < ctx->n_sounds; ++i)
		if (strcmp(ctx->sounds_paths[i], sound_path) == 0)
			return i;

	if (ctx->n_sounds == MAX_SOUNDS)
		exit(1);

	ctx->sounds_paths[ctx->n_sounds] = sound_path;
	ctx->sounds[ctx->n_sounds] = Mix_LoadWAV(sound_path);
	ctx->n_sounds += 1;

	return ctx->n_sounds - 1;
}

static void __sdl_context_play_sound(const struct sdl_context *ctx,
		int sound_id)
{
	if (sound_id >= ctx->n_sounds || sound_id < 0)
		exit(1);

	Mix_PlayChannel(-1, ctx->sounds[sound_id], 0);
}

static void __sdl_context_render_texture(const struct sdl_context *ctx,
		int texture_id, int x, int y, int w, int h)
{
	if (texture_id >= ctx->n_textures || texture_id < 0)
		exit(1);

	SDL_Rect rect = {
		.x = x,
		.y = y,
		.w = w,
		.h = h
	};

	SDL_RenderCopy(ctx->renderer, ctx->textures[texture_id], NULL, &rect);
}

static void __sdl_context_render_rect(const struct sdl_context *ctx,
		int x, int y, int w, int h, uint32_t color)
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

static void __sdl_context_delay(const struct sdl_context *ctx, int ms)
{
	(void) ctx;
	SDL_Delay(ms);
}

static void __render_map(struct sdl_context *ctx, struct map *map, int cz)
{
	int food_texture = __sdl_context_load_texture(ctx, "./gfx/apple.png");

	int vt_down_left = __sdl_context_load_texture(ctx, "./gfx/body_bottomleft.png");
	int vt_down_right = __sdl_context_load_texture(ctx, "./gfx/body_bottomright.png");
	int vt_hor = __sdl_context_load_texture(ctx, "./gfx/body_horizontal.png");
	int vt_up_left = __sdl_context_load_texture(ctx, "./gfx/body_topleft.png");
	int vt_up_right = __sdl_context_load_texture(ctx, "./gfx/body_topright.png");
	int vt_ver = __sdl_context_load_texture(ctx, "./gfx/body_vertical.png");

	int tail_left = __sdl_context_load_texture(ctx, "./gfx/tail_left.png");
	int tail_right = __sdl_context_load_texture(ctx, "./gfx/tail_right.png");
	int tail_down = __sdl_context_load_texture(ctx, "./gfx/tail_down.png");
	int tail_up = __sdl_context_load_texture(ctx, "./gfx/tail_up.png");

	int head_left = __sdl_context_load_texture(ctx, "./gfx/head_left.png");
	int head_right = __sdl_context_load_texture(ctx, "./gfx/head_right.png");
	int head_down = __sdl_context_load_texture(ctx, "./gfx/head_down.png");
	int head_up = __sdl_context_load_texture(ctx, "./gfx/head_up.png");

	uint32_t bg_colors[2] = {0xa6d13c, 0xadd644};
	for (size_t x = 0; x < map->n_columns; ++x)
		for (size_t y = 0; y < map->n_rows; ++y)
			__sdl_context_render_rect(ctx, x*cz, y*cz, cz, cz, bg_colors[(x+y)%2]);

	size_t pr, pc;
	size_t r, c;
	size_t nr, nc;
	enum map_block_type prev, cur, next;
	int is_head = 0, is_tail;

	r = map->tail_row;
	c = map->tail_col;

	while (!is_head)
	{
		int text = -1;

		is_tail = c == map->tail_col && r == map->tail_row;
		is_head = c == map->head_col && r == map->head_row;

		map_find_viborita_next_block(map, r, c, &nr, &nc);

		cur = map->map[r][c];

		if (is_tail) switch (cur)
		{
			case MAP_BLOCK_VIBORITA_LEFT: text = tail_right; break;
			case MAP_BLOCK_VIBORITA_RIGHT: text = tail_left; break;
			case MAP_BLOCK_VIBORITA_UP: text = tail_down; break;
			case MAP_BLOCK_VIBORITA_DOWN: text = tail_up; break;
		}

		if (is_head) switch (cur)
		{
			case MAP_BLOCK_VIBORITA_LEFT: text = head_left; break;
			case MAP_BLOCK_VIBORITA_RIGHT: text = head_right; break;
			case MAP_BLOCK_VIBORITA_UP: text = head_up; break;
			case MAP_BLOCK_VIBORITA_DOWN: text = head_down; break;
		}

		if (!is_head && !is_tail)
		{
			next = map->map[nr][nc];
			prev = map->map[pr][pc];

			if (cur == prev)
			{
				switch (cur)
				{
					case MAP_BLOCK_VIBORITA_DOWN:
					case MAP_BLOCK_VIBORITA_UP:
						text = vt_ver;
						break;
					case MAP_BLOCK_VIBORITA_LEFT:
					case MAP_BLOCK_VIBORITA_RIGHT:
						text = vt_hor;
						break;
				}
			}
			else
			{
				if ((prev == MAP_BLOCK_VIBORITA_RIGHT && cur == MAP_BLOCK_VIBORITA_UP) ||
						(prev == MAP_BLOCK_VIBORITA_DOWN && cur == MAP_BLOCK_VIBORITA_LEFT))
					text = vt_up_left;
				else if ((prev == MAP_BLOCK_VIBORITA_RIGHT && cur == MAP_BLOCK_VIBORITA_DOWN) ||
						(prev == MAP_BLOCK_VIBORITA_UP && cur == MAP_BLOCK_VIBORITA_LEFT))
					text = vt_down_left;
				else if ((prev == MAP_BLOCK_VIBORITA_LEFT && cur == MAP_BLOCK_VIBORITA_UP) ||
						(prev == MAP_BLOCK_VIBORITA_DOWN && cur == MAP_BLOCK_VIBORITA_RIGHT))
					text = vt_up_right;
				else if ((prev == MAP_BLOCK_VIBORITA_LEFT && cur == MAP_BLOCK_VIBORITA_DOWN) ||
						(prev == MAP_BLOCK_VIBORITA_UP && cur == MAP_BLOCK_VIBORITA_RIGHT))
					text = vt_down_right;
			}
		}

		if (text != -1)
			__sdl_context_render_texture(ctx, text, c * cz, r * cz, cz, cz);

		pr = r; pc = c;
		c = nc; r = nr;
	}


	MAP_FOR_EACH_BLOCK(map, row, col, block)
	{
		switch (block)
		{
			case MAP_BLOCK_FOOD: __sdl_context_render_texture(ctx, food_texture, col * cz, row * cz, cz, cz); break;
			case MAP_BLOCK_WALL: __sdl_context_render_rect(ctx, col*cz, row*cz, cz, cz, 0x349eeb); break;
		}
	}
}

int
main(int argc, char **argv)
{
	int score = 0;
	struct map map;
	struct sdl_context sdl_context;
	enum map_viborita_state state;
	int c;
	SDL_Event event;

	if (argc < 2 || map_parse_file(&map, argv[1]) < 0)
		return 1;

	__sdl_context_create(&sdl_context);

	while (1)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT: return 0;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_h: map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_LEFT);  break;
						case SDLK_j: map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_DOWN);  break;
						case SDLK_k: map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_UP);    break;
						case SDLK_l: map_set_viborita_direction(&map, MAP_BLOCK_VIBORITA_RIGHT); break;
					}
					break;
			}
		}

		map_advance(&map, &state);

		switch (state)
		{
			case MAP_VIBORITA_EATING:
				__sdl_context_play_sound(&sdl_context, __sdl_context_load_sound(&sdl_context, "./sfx/chomp.wav"));
				map_spawn_food(&map);
				score += 1;
				break;
			case MAP_VIBORITA_DEAD:
				__sdl_context_play_sound(&sdl_context, __sdl_context_load_sound(&sdl_context, "./sfx/death.wav"));
				score = 0;
				map_parse_file(&map, argv[1]);
				break;
		}

		/* __sdl_context_query_window_size(&sdl_context, &ww, &wh); */
		__sdl_context_begin_draw(&sdl_context);
		__render_map(&sdl_context, &map, 40);
		__sdl_context_end_draw(&sdl_context);
		__sdl_context_delay(&sdl_context, 75);
	}

	__sdl_context_destroy(&sdl_context);

	return 0;
}
