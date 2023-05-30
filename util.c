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
