#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"

int is_regular_file(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) < 0)
		return 0;

	return !!S_ISREG(sb.st_mode);
}

int dump_file_cts(const char *path, size_t max_size, char *out)
{
	FILE *fp;
	extern char *app_name;
	unsigned long n_bytes_read;

	if (!is_regular_file(path))
	{
		dbg_print(stderr, "%s: dump_file_cts: %s is not a regular file\n",
				app_name, path);
		return -1;
	}

	fp = fopen(path, "r");
	if (NULL == fp)
	{
		dbg_print(stderr, "%s: dump_file_cts: could not open %s: %s\n",
				app_name, path, strerror(errno));
		return -1;
	}

	n_bytes_read = fread(out, 1, max_size - 1, fp);
	if (n_bytes_read == 0 && !feof(fp))
	{
		dbg_print(stderr, "%s: dump_file_cts: could not read from %s: %s\n",
				app_name, path, strerror(errno));
		return -1;
	}

	out[n_bytes_read] = '\0';

	return 0;
}
