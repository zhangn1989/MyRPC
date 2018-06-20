#ifndef __FILEIO_H
#define	__FILEIO_H

#include <unistd.h>

ssize_t readn(int fd, void *buffer, size_t n);

ssize_t writen(int fd, const void *buffer, size_t n);

#endif // !__FILEIO_H