#ifndef CORBA_STRING_H
#define CORBA_STRING_H 1

CORBA_char *CORBA_string_alloc(CORBA_unsigned_long len);

/* ORBit extension */
CORBA_char *CORBA_string_dup(CORBA_char *str);

/* Internals, do not use */
gpointer CORBA_string__freekids(gpointer mem, gpointer data);
#endif
