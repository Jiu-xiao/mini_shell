#include "ms_list.h"

void ms_list_init_head(ms_list_head_t* list) {
  list->next = list;
  list->prev = list;
}

static void __list_add(ms_list_head_t* new_data, ms_list_head_t* prev,
                ms_list_head_t* next) {
  next->prev = new_data;
  new_data->next = next;
  new_data->prev = prev;
  prev->next = new_data;
}

void ms_list_add(ms_list_head_t* new_data, ms_list_head_t* head) {
  __list_add(new_data, head, head->next);
}

void ms_list_add_tail(ms_list_head_t* new_data, ms_list_head_t* head) {
  __list_add(new_data, head->prev, head);
}

static void __list_del(ms_list_head_t* prev, ms_list_head_t* next) {
  next->prev = prev;
  prev->next = next;
}

void ms_list_del(ms_list_head_t* entry) {
  __list_del(entry->prev, entry->next);
}

static void __list_del_entry(ms_list_head_t* entry) {
  __list_del(entry->prev, entry->next);
}

void ms_list_del_init(ms_list_head_t* entry) {
  __list_del_entry(entry);
  ms_list_init_head(entry);
}

void ms_list_replace(ms_list_head_t* old, ms_list_head_t* new_data) {
  new_data->next = old->next;
  new_data->next->prev = new_data;
  new_data->prev = old->prev;
  new_data->prev->next = new_data;
}

int ms_list_empty(const ms_list_head_t* head) { return head->next == head; }

size_t ms_list_get_num(const ms_list_head_t* head) {
  uint32_t num = 0;

  ms_list_head_t* pos;
  ms_list_for_each(pos, head) { num++; }

  return num;
}
