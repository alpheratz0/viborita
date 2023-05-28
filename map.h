#pragma once

#include <stddef.h>

#define MAX_COLS 100
#define MAX_ROWS 100

#define MAP_BLOCK_TYPE_IS_VIBORA(bt) \
	(bt == MAP_BLOCK_VIBORITA_DOWN || \
		bt == MAP_BLOCK_VIBORITA_LEFT || \
		bt == MAP_BLOCK_VIBORITA_RIGHT || \
		bt == MAP_BLOCK_VIBORITA_UP)

#define MAP_BLOCK_TYPE_FROM_CHAR(ch) \
	(ch == ' ' ? MAP_BLOCK_SPACE : \
		ch == '=' ? MAP_BLOCK_WALL : \
		ch == '*' ? MAP_BLOCK_FOOD : \
		ch == '^' ? MAP_BLOCK_VIBORITA_UP : \
		ch == '<' ? MAP_BLOCK_VIBORITA_LEFT : \
		ch == 'v' ? MAP_BLOCK_VIBORITA_DOWN : \
		ch == '>' ? MAP_BLOCK_VIBORITA_RIGHT : \
		            MAP_BLOCK_INVALID)

#define MAP_BLOCK_TYPE_TO_CHAR(bt) \
	(bt == MAP_BLOCK_SPACE             ? ' ' : \
		bt == MAP_BLOCK_WALL           ? '=' : \
		bt == MAP_BLOCK_FOOD           ? '*' : \
		bt == MAP_BLOCK_VIBORITA_UP    ? '^' : \
		bt == MAP_BLOCK_VIBORITA_LEFT  ? '<' : \
		bt == MAP_BLOCK_VIBORITA_DOWN  ? 'v' : \
		bt == MAP_BLOCK_VIBORITA_RIGHT ? '>' : \
		                                 '?')

#define MAP_FOR_EACH_BLOCK(m, row, col, block) \
	for (size_t row = 0, col = 0; row < (m)->n_rows; ++row, col = 0) \
		for (enum map_block_type block; col < (m)->n_columns && (block = (m)->map[row][col]) == block; ++col) \

enum map_block_type {
	MAP_BLOCK_SPACE,
	MAP_BLOCK_WALL,
	MAP_BLOCK_FOOD,
	MAP_BLOCK_VIBORITA_UP,
	MAP_BLOCK_VIBORITA_LEFT,
	MAP_BLOCK_VIBORITA_DOWN,
	MAP_BLOCK_VIBORITA_RIGHT,
	MAP_BLOCK_INVALID
};

enum map_viborita_state {
	MAP_VIBORITA_DEAD,
	MAP_VIBORITA_IDLE,
	MAP_VIBORITA_EATING
};

struct map {
	size_t n_columns;
	size_t n_rows;
	enum map_block_type map[MAX_COLS][MAX_ROWS];
};

int map_init(struct map *map, size_t n_cols, size_t n_rows);
int map_copy(const struct map *from, struct map *to);
int map_parse(struct map *map, const char *map_str);
int map_parse_file(struct map *map, const char *path);
int map_stringify(const struct map *map, size_t max_size, char *str);
int map_get_ref(struct map *map, size_t col, size_t row, enum map_block_type **bt);
int map_get(const struct map *map, size_t col, size_t row, enum map_block_type *bt);
int map_set(struct map *map, size_t col, size_t row, enum map_block_type bt);
int map_get_vibora_prev_block(struct map *map, size_t row, size_t col, size_t *prev_row, size_t *prev_col);
int map_get_vibora_next_block(struct map *map, size_t row, size_t col, size_t *next_row, size_t *next_col);
int map_is_head(struct map *map, size_t row, size_t col);
int map_is_tail(struct map *map, size_t row, size_t col);
int map_find(struct map *map, int (*pred)(struct map *, size_t, size_t), size_t *row, size_t *col);
int map_find_viborita_head(struct map *map, size_t *row, size_t *col);
int map_find_viborita_tail(struct map *map, size_t *row, size_t *col);
int map_advance(struct map *map, enum map_viborita_state *viborita_state);
