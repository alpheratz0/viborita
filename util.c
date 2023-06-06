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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

static int __is_regular_file(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) < 0)
		return 0;

	return !!S_ISREG(sb.st_mode);
}

int dump_file_cts(const char *path, size_t max_size, char *out)
{
	FILE *fp;
	unsigned long n_bytes_read;

	if (!__is_regular_file(path))
		return -1;

	fp = fopen(path, "r");

	if (NULL == fp)
		return -1;

	n_bytes_read = fread(out, 1, max_size - 1, fp);
	if (n_bytes_read == 0 && !feof(fp))
		return -1;

	out[n_bytes_read] = '\0';

	return 0;
}
