#ifndef LINC_TYPES_H
#define LINC_TYPES_H 1

typedef enum {
	LINC_CONNECTION_SSL         = 1 << 0,
	LINC_CONNECTION_NONBLOCKING = 1 << 1
} LINCConnectionOptions;

typedef struct _LincWatch LincWatch;

#endif
