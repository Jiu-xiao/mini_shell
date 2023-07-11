#ifndef __MS_LIST_H__
#define __MS_LIST_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct _ms_list_head {
  struct _ms_list_head* next;
} ms_list_head_t;

#if MS_DEBUG
#define MS_ASSERT(arg) \
  if (!(arg)) ms_error(__FILE__, __LINE__);
#define MS_CHECK(arg) \
  if (!(arg)) ms_error(__FILE__, __LINE__);

#else
#define MS_ASSERT(arg) (void)0;
#define MS_CHECK(arg) (void)0;
#endif

#define MS_UNUSED(X) ((void)X)

#define ms_offset_of(type, member) ((size_t) & ((type*)0)->member)

#define ms_member_size_of(type, member) (sizeof(typeof(((type*)0)->member)))

#define ms_container_of(ptr, type, member)               \
  ({                                                     \
    const typeof(((type*)0)->member)* __mptr = (ptr);    \
    (type*)((char*)__mptr - ms_offset_of(type, member)); \
  })

#define ms_list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define ms_list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define ms_list_entry(ptr, type, member) ms_container_of(ptr, type, member)

#define ms_del_all(source, del_fun)                \
  do {                                             \
    ms_list_head_t *pos, *tmp;                     \
    for (pos = (source)->next; pos != (source);) { \
      tmp = pos;                                   \
      pos = pos->next;                             \
      del_fun(tmp);                                \
    }                                              \
  } while (0)

#define MS_PRASE_STRUCT(container, member)            \
  sizeof(container), ms_offset_of(container, member), \
      ms_member_size_of(container, member)

void ms_list_init_head(ms_list_head_t* list);

void ms_list_add(ms_list_head_t* new_data, ms_list_head_t* head);

void ms_list_del(ms_list_head_t* entry);

void ms_list_replace(ms_list_head_t* old, ms_list_head_t* new_data);

int ms_list_empty(const ms_list_head_t* head);

size_t ms_list_get_num(const ms_list_head_t* head);
#endif
