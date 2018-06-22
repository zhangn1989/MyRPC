#ifndef __PUBLIC_HEAD_H
#define __PUBLIC_HEAD_H

#include <errno.h>
#include "log.h"

#define handle_warning(msg) \
        do { write_logfile(SC_LOG_ERROR, stderr, \
            "file:%s line:%d errorno:%d usrmsg:%s", \
            __FILE__, __LINE__, errno, msg); \
            } while (0)

#define handle_error(msg) \
        do { write_logfile(SC_LOG_ERROR, stderr, \
            "file:%s line:%d errorno:%d usrmsg:%s", \
            __FILE__, __LINE__, errno, msg); \
            exit(errno); } while (0)

#endif  //__PUBLIC_HEAD_H
