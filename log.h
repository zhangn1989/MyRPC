#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef enum {
	SC_LOG_NONE = 0,
	SC_LOG_EMERGENCY,
	SC_LOG_ALERT,
	SC_LOG_CRITICAL,
	SC_LOG_ERROR,
	SC_LOG_WARNING,
	SC_LOG_NOTICE,
	SC_LOG_INFO,
	SC_LOG_DEBUG,

	SC_LOG_LEVEL_MAX,
} log_level;

inline static FILE *open_logfile(const char *path)
{
	return fopen(path, "a+");
}

inline static void write_logfile(log_level type, FILE *fp, const char *format, ...)
{
	char log[128];
	va_list arg_list;

	memset(log, 0, sizeof(log));

	switch (type)
	{
	case SC_LOG_EMERGENCY:
		sprintf(log, "[log type:%s] ", "emergency");
		break;
	case SC_LOG_ALERT:
		sprintf(log, "[log type:%s] ", "alert");
		break;
	case SC_LOG_CRITICAL:
		sprintf(log, "[log type:%s] ", "critical");
		break;
	case SC_LOG_ERROR:
		sprintf(log, "[log type:%s] ", "error");
		break;
	case SC_LOG_WARNING:
		sprintf(log, "[log type:%s] ", "warning");
		break;
	case SC_LOG_NOTICE:
		sprintf(log, "[log type:%s] ", "notice");
		break;
	case SC_LOG_INFO:
		sprintf(log, "[log type:%s] ", "info");
		break;
	case SC_LOG_DEBUG:
		sprintf(log, "[log type:%s] ", "debug");
		break;
	default:
		sprintf(log, "[log type is error] ");
		break;
	}

	fprintf(fp, log);

	va_start(arg_list, format);
	vfprintf(fp, format, arg_list);
	va_end(arg_list);

	fprintf(fp, "\n");
}

inline static void close_logfile(FILE *fp)
{
	fclose(fp);
}

#define	writelog_emergency(fp, format, ...) \
	do { write_logfile(SC_LOG_EMERGENCY, fp, format, __VA_ARGS__); } while (0)

#define	writelog_alert(fp, format, ...) \
	do { write_logfile(SC_LOG_ALERT, fp, format, __VA_ARGS__); } while (0)

#define	writelog_critical(fp, format, ...) \
	do { write_logfile(SC_LOG_CRITICAL, fp, format, __VA_ARGS__); } while (0)

#define	writelog_error(fp, format, ...) \
	do { write_logfile(SC_LOG_ERROR, fp, format, __VA_ARGS__); } while (0)

#define	writelog_warning(fp, format, ...) \
	do { write_logfile(SC_LOG_WARNING, fp, format, __VA_ARGS__); } while (0)

#define	writelog_notice(fp, format, ...) \
	do { write_logfile(SC_LOG_NOTICE, fp, format, __VA_ARGS__); } while (0)

#define	writelog_info(fp, format, ...) \
	do { write_logfile(SC_LOG_INFO, fp, format, __VA_ARGS__); } while (0)

#define	writelog_debug(fp, format, ...) \
	do { write_logfile(SC_LOG_DEBUG, fp, format, __VA_ARGS__); } while (0)

#endif // !__LOG_H