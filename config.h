#ifndef RF_CONFIG_H
#define RF_CONFIG_H

#include <ctype.h>
#include <stdio.h>

#define KV_STRING_LIMIT 1024

extern char config_key[KV_STRING_LIMIT];
extern char config_value[KV_STRING_LIMIT];

int config_get(size_t *len, FILE *fp);

#endif
