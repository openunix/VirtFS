#ifndef _VIRTFS_LOG_H_
#define _VIRTFS_LOG_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

enum virtfs_log_level {
	VIRTFS_LOG_EMERG,
	VIRTFS_LOG_ALERT,
	VIRTFS_LOG_CRIT,
	VIRTFS_LOG_ERR,
	VIRTFS_LOG_WARNING,
	VIRTFS_LOG_NOTICE,
	VIRTFS_LOG_INFO,
	VIRTFS_LOG_DEBUG
};

typedef void (*virtfs_log_func_t)(enum virtfs_log_level level,
				  const char *fmt, va_list ap);

void virtfs_set_log_func(virtfs_log_func_t func);

void virtfs_log(enum virtfs_log_level level, const char *fmt, ...);

#endif /*_VIRTFS_LOG_H_*/