/*
	Copyright (C) 2023-2025 <alpheratz99@protonmail.com>

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

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include "map.h"

#define VIBORITA_WM_NAME "viborita"
#define VIBORITA_WM_CLASS "viborita\0viborita\0"

static struct map map;
static xcb_connection_t *conn;
static xcb_screen_t *screen;
static xcb_window_t window;
static xcb_gcontext_t gc_space[2];
static xcb_gcontext_t gc_food;
static xcb_gcontext_t gc_wall;
static xcb_gcontext_t gc_snake;
static xcb_key_symbols_t *ksyms;
static uint32_t width, height;
static int zoom;
static bool should_close, paused;

static void
die(const char *fmt, ...)
{
	va_list args;

	fputs("viborita: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	exit(1);
}

static xcb_atom_t
get_atom(const char *name)
{
	xcb_atom_t atom;
	xcb_generic_error_t *error;
	xcb_intern_atom_cookie_t cookie;
	xcb_intern_atom_reply_t *reply;

	cookie = xcb_intern_atom(conn, 0, strlen(name), name);
	reply = xcb_intern_atom_reply(conn, cookie, &error);

	if (NULL != error)
		die("xcb_intern_atom failed with error code: %d",
				(int)(error->error_code));

	atom = reply->atom;
	free(reply);

	return atom;
}

static xcb_gcontext_t
xcolor(uint32_t fg)
{
	xcb_gcontext_t gc;
	gc = xcb_generate_id(conn);
	xcb_create_gc(conn, gc, window, XCB_GC_FOREGROUND, &fg);
	return gc;
}

static void
create_window(void)
{
	if (xcb_connection_has_error(conn = xcb_connect(NULL, NULL)))
		die("can't open display");

	if (NULL == (screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data))
		die("can't get default screen");

	width = 640, height = 480;
	ksyms = xcb_key_symbols_alloc(conn);
	window = xcb_generate_id(conn);

	xcb_create_window_aux(
		conn, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0,
		width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		screen->root_visual, XCB_CW_EVENT_MASK | XCB_CW_BACK_PIXEL,
		(const xcb_create_window_value_list_t []) {{
			.background_pixel = 0x0,
			.event_mask = XCB_EVENT_MASK_EXPOSURE |
			              XCB_EVENT_MASK_KEY_PRESS |
			              XCB_EVENT_MASK_BUTTON_PRESS |
			              XCB_EVENT_MASK_STRUCTURE_NOTIFY
		}}
	);

	gc_food = xcolor(0xc10b26);
	gc_wall = xcolor(0xffffff);
	gc_space[0] = xcolor(0x000000);
	gc_space[1] = xcolor(0x090909);
	gc_snake = xcolor(0xc4f669);

	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, window, get_atom("_NET_WM_NAME"),
		get_atom("UTF8_STRING"), 8, sizeof(VIBORITA_WM_NAME) - 1,
		VIBORITA_WM_NAME
	);

	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_CLASS,
		XCB_ATOM_STRING, 8, sizeof(VIBORITA_WM_CLASS) - 1,
		VIBORITA_WM_CLASS
	);

	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, window,
		get_atom("WM_PROTOCOLS"), XCB_ATOM_ATOM, 32, 1,
		(const xcb_atom_t []) { get_atom("WM_DELETE_WINDOW") }
	);

	xcb_map_window(conn, window);
	xcb_flush(conn);
}

static void
destroy_window(void)
{
	xcb_free_gc(conn, gc_wall);
	xcb_free_gc(conn, gc_space[0]);
	xcb_free_gc(conn, gc_space[1]);
	xcb_free_gc(conn, gc_food);
	xcb_free_gc(conn, gc_snake);
	xcb_key_symbols_free(ksyms);
	xcb_disconnect(conn);
}

static void
render_map(void)
{
	int block_size;
	int map_x1, map_x2;
	int map_y1, map_y2;
	xcb_gcontext_t gc;

	block_size = 20 + (zoom < -18 ? -18 : zoom);
	map_x1 = -((map.head_col * block_size) -  (width - block_size) / 2);
	map_y1 = -((map.head_row * block_size) - (height - block_size) / 2);
	map_x2 = map_x1 + map.n_cols * block_size;
	map_y2 = map_y1 + map.n_rows * block_size;

	if (map_x1 > 0)
		xcb_clear_area(conn, 0, window, 0, 0, map_x1, height);
	if (map_y1 > 0)
		xcb_clear_area(conn, 0, window, 0, 0, width, map_y1);
	if (map_x2 < width)
		xcb_clear_area(conn, 0, window, map_x2, 0, width - map_x2, height);
	if (map_y2 < height)
		xcb_clear_area(conn, 0, window, 0, map_y2, width, height - map_y2);

	MAP_FOR_EACH_BLOCK(&map, row, col, block) {
		switch (block) {
		case MAP_BLOCK_SPACE:
			gc = gc_space[(row + col) % 2];
			break;
		case MAP_BLOCK_FOOD:
			gc = gc_food;
			break;
		case MAP_BLOCK_WALL:
			gc = gc_wall;
			break;
		case MAP_BLOCK_SNAKE_UP:
		case MAP_BLOCK_SNAKE_LEFT:
		case MAP_BLOCK_SNAKE_RIGHT:
		case MAP_BLOCK_SNAKE_DOWN:
			gc = gc_snake;
			break;
		}

		xcb_poly_fill_rectangle(
			conn,
			window,
			gc,
			1,
			&(const xcb_rectangle_t) {
				.x = map_x1 + col * block_size,
				.y = map_y1 + row * block_size,
				.width = block_size,
				.height = block_size
			}
		);
	}

	xcb_flush(conn);
}

static void
h_client_message(xcb_client_message_event_t *ev)
{
	/* check if the wm sent a delete window message */
	/* https://www.x.org/docs/ICCCM/icccm.pdf */
	if (ev->data.data32[0] == get_atom("WM_DELETE_WINDOW"))
		should_close = true;
}

static void
h_expose(xcb_expose_event_t *ev)
{
	render_map();
}

static void
h_key_press(xcb_key_press_event_t *ev)
{
	xcb_keysym_t key;
	enum map_block_type dir;

	key = xcb_key_symbols_get_keysym(ksyms, ev->detail, 0);
	dir = MAP_BLOCK_INVALID;

	switch (key) {
	case XKB_KEY_h: dir = MAP_BLOCK_SNAKE_LEFT; break;
	case XKB_KEY_j: dir = MAP_BLOCK_SNAKE_DOWN; break;
	case XKB_KEY_k: dir = MAP_BLOCK_SNAKE_UP; break;
	case XKB_KEY_l: dir = MAP_BLOCK_SNAKE_RIGHT; break;
	case XKB_KEY_space: paused = !paused; break;
	}

	// FIXME: pressing j and then l should move
	//        the snake down and in the next game frame
	//        move the snake to the right
	if (dir != MAP_BLOCK_INVALID) {
		paused = false;
		map_set_snake_direction(&map, dir);
	}
}

static void
h_button_press(xcb_button_press_event_t *ev)
{
	switch (ev->detail) {
	case XCB_BUTTON_INDEX_4: zoom += 1; break;
	case XCB_BUTTON_INDEX_5: zoom -= 1; break;
	}
}

static void
h_configure_notify(xcb_configure_notify_event_t *ev)
{
	if (width == ev->width && height == ev->height)
		return;

	width = ev->width;
	height = ev->height;
}

static void
h_mapping_notify(xcb_mapping_notify_event_t *ev)
{
	if (ev->count > 0)
		xcb_refresh_keyboard_mapping(ksyms, ev);
}

int
main(int argc, char **argv)
{
	xcb_generic_event_t *ev;
	enum map_snake_state state;

	/* seed rand with the current process id */
	srand((unsigned int)(getpid()));


	if (argc < 2 || map_parse_file(&map, argv[1]) < 0) {
		fprintf(stderr, "usage: viborita_xcb [valid_map_path]\n");
		return 1;
	}

	create_window();
	render_map();

	paused = true;
	while (!should_close) {
		while (!should_close && (ev = xcb_poll_for_event(conn))) {
			switch (ev->response_type & ~0x80) {
			case XCB_CLIENT_MESSAGE:    h_client_message((void *)(ev)); break;
			case XCB_EXPOSE:            h_expose((void *)(ev)); break;
			case XCB_KEY_PRESS:         h_key_press((void *)(ev)); break;
			case XCB_BUTTON_PRESS:      h_button_press((void *)(ev)); break;
			case XCB_CONFIGURE_NOTIFY:  h_configure_notify((void *)(ev)); break;
			case XCB_MAPPING_NOTIFY:    h_mapping_notify((void *)(ev)); break;
			}

			free(ev);
		}

		if (!paused) {
			map_advance(&map, &state);
			switch (state) {
			case MAP_SNAKE_DEAD:
				map_parse_file(&map, argv[1]);
				paused = true;
				break;
			case MAP_SNAKE_EATING:
				map_spawn_food(&map);
				break;
			}
		}

		render_map();
		usleep(66666);
	}

	destroy_window();

	return 0;
}
