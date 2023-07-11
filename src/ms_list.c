#include "ms_list.h"

void ms_list_init_head(ms_list_head_t* list) { list->next = list; }

static void __list_add(ms_list_head_t* new_data, ms_list_head_t* prev,
                       ms_list_head_t* next) {
  new_data->next = next;
  prev->next = new_data;
}

void ms_list_add(ms_list_head_t* new_data, ms_list_head_t* head) {
  __list_add(new_data, head, head->next);
}

static void __list_del(ms_list_head_t* prev, ms_list_head_t* next) {
  prev->next = next;
}

static ms_list_head_t* __list_get_prev(ms_list_head_t* entry) {
  ms_list_head_t* prev = entry;
  while (prev->next != entry) {
    prev = prev->next;
  }
  return prev;
}

void ms_list_del(ms_list_head_t* entry) {
  __list_del(__list_get_prev(entry), entry->next);
}

void ms_list_replace(ms_list_head_t* old, ms_list_head_t* new_data) {
  new_data->next = old->next;
  __list_get_prev(old)->next = new_data;
}

int ms_list_empty(const ms_list_head_t* head) { return head->next == head; }

size_t ms_list_get_num(const ms_list_head_t* head) {
  uint32_t num = 0;

  ms_list_head_t* pos;
  ms_list_for_each(pos, head) { num++; }

  return num;
}
