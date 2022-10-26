#ifndef DATA_OPERATE_H
#define DATA_OPERATE_H

#include <stdint.h>

uint64_t byte_get_little_endian(const unsigned char *field, int size);
void byte_put_little_endian(unsigned char *field, uint64_t value, int size);

#endif