#ifndef __PUBLIC_HEAD_H
#define __PUBLIC_HEAD_H

#include "log.h"

#define handle_info(msg) \
        do { write_logfile(SC_LOG_INFO, stderr, \
            "file:%s line:%d errorno:%d message:%s", \
            __FILE__, __LINE__, errno, msg); \
            } while (0)

#define handle_warning(msg) \
        do { write_logfile(SC_LOG_WARNING, stderr, \
            "file:%s line:%d errorno:%d message:%s", \
            __FILE__, __LINE__, errno, msg); \
            } while (0)

#define handle_error(msg) \
        do { write_logfile(SC_LOG_ERROR, stderr, \
            "file:%s line:%d errorno:%d message:%s", \
            __FILE__, __LINE__, errno, msg); \
            exit(errno); } while (0)

#endif  //__PUBLIC_HEAD_H
