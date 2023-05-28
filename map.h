#pragma once

#include <stddef.h>

#define MAX_COLS 100
#define MAX_ROWS 100

enum map_block_type {
	MAP_SPACE,
	MAP_BLOCK,
	MAP_VIBORITA,
	MAP_FOOD
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
