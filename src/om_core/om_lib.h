#ifndef __OM_LIB_H__
#define __OM_LIB_H__

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  OM_OK = 0,
  OM_ERROR = 1,
  OM_ERROR_NULL,
  OM_ERROR_BUSY,
  OM_ERROR_TIMEOUT,
  OM_ERROR_FULL,
  OM_ERROR_EMPTY,
  OM_ERROR_NOT_INIT
} om_status_t;

#define OM_UNUSED(X) ((void)X)

#define om_offset_of(type, member) ((size_t) & ((type*)0)->member)

#define om_member_size_of(type, member) (sizeof(typeof(((type*)0)->member)))

#define om_container_of(ptr, type, member)               \
  ({                                                     \
    const typeof(((type*)0)->member)* __mptr = (ptr);    \
    (type*)((char*)__mptr - om_offset_of(type, member)); \
  })

#endif
