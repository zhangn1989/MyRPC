#include <errno.h>

#include "fileio.h"

ssize_t readn(int fd, void *buffer, size_t n)
{
	ssize_t num_read;
	size_t tot_read;
	char *buf;

	buf = buffer;
	for (tot_read = 0; tot_read < n; )
	{
		num_read = read(fd, buf, n - tot_read);

		if (num_read == 0)
			return tot_read;

		if (num_read < 0)
		{
			if (errno == EINTR)
				continue;
			else
				return -1;
		}

		tot_read += num_read;
		buf += num_read;
	}

	return tot_read;
}

ssize_t writen(int fd, const void *buffer, size_t n)
{
	ssize_t num_written;
	size_t tot_written;
	const char *buf;

	buf = buffer;
	for (tot_written = 0; tot_written < n; )
	{
		num_written = write(fd, buf, n - tot_written);

		if (num_written <= 0)
		{
			if (num_written < 0 && errno == EINTR)
				continue;
			else
				return -1;
		}

		tot_written += num_written;
		buf += num_written;
	}

	return tot_written;
}
