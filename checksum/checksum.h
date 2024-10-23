#ifndef CHECKSUM_H_
#define CHECKSUM_H_

#include "stddef.h"

int checksum_add(void* buf, size_t len);
int checksum_valid(void* buf, size_t len);

#endif
