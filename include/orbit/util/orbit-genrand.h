#ifndef ORBIT_GENRAND_H
#define ORBIT_GENRAND_H 1

#include <glib.h>

typedef struct ORBit_genrand_type {
    int		fd;
} ORBit_genrand;

void ORBit_genrand_init(ORBit_genrand *gr);
void ORBit_genrand_fini(ORBit_genrand *gr);

void ORBit_genrand_buf(ORBit_genrand *gr, guchar *buffer, int buf_len);

#endif /* ORBIT_GENRAND_H */
