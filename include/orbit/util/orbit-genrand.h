#ifndef ORBIT_GENRAND_H
#define ORBIT_GENRAND_H 1

#include <glib.h>

#ifdef ORBIT2_INTERNAL_API

typedef enum {
	ORBIT_GENUID_STRONG,
	ORBIT_GENUID_SIMPLE,
} ORBitGenUidType;

void ORBit_genuid_init   (ORBitGenUidType type);
void ORBit_genuid_fini   (void);
void ORBit_genuid_buffer (guchar *buffer, int length);

#endif /* ORBIT2_INTERNAL_API */

#endif /* ORBIT_GENRAND_H */
