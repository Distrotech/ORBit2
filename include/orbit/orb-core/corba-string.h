#ifndef CORBA_STRING_H
#define CORBA_STRING_H 1

CORBA_char *CORBA_string_alloc(CORBA_unsigned_long len);
CORBA_wchar *CORBA_wstring_alloc(CORBA_unsigned_long len);

/* ORBit extension */
CORBA_char *CORBA_string_dup(CORBA_char *str);

/* Internals, do not use */
gpointer CORBA_string__freekids(gpointer mem, gpointer data);

CORBA_sequence_octet *ORBit_sequence_octet_dup(const CORBA_sequence_octet *seq);
#endif
