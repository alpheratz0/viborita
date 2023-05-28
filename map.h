#pragma once

#include <stddef.h>

#define MAX_COLS 100
#define MAX_ROWS 100

#define MAP_BLOCK_TYPE_IS_VIBORA(bt) \
	(bt == MAP_BLOCK_VIBORITA_DOWN || \
		bt == MAP_BLOCK_VIBORITA_LEFT || \
		bt == MAP_BLOCK_VIBORITA_RIGHT || \
		bt == MAP_BLOCK_VIBORITA_UP)

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
int map_get(const struct map *map, size_t col, size_t row, enum map_block_type *bt);
int map_set(struct map *map, size_t col, size_t row, enum map_block_type bt);
int map_find_viborita_head(struct map *map, size_t *row, size_t *col);
int map_find_viborita_tail(struct map *map, size_t *row, size_t *col);
