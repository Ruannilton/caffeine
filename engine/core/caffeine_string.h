#pragma once

#include "caffeine_memory.h"

typedef struct {
  cff_size len;
  char *buffer;
} cff_string;

typedef struct {
  cff_size len;
  const char *buffer;
} cff_string_span;

cff_string cff_string_create_literal(const char *str,
                                     cff_allocator_t allocator);
cff_string cff_string_create(char *str, cff_size lenght,
                             cff_allocator_t allocator);
cff_err_e cff_string_update(cff_string *from, char *str, cff_size lenght,
                            cff_allocator_t allocator);
cff_string cff_string_copy(cff_string from, cff_allocator_t allocator);
void cff_string_destroy(cff_string *from, cff_allocator_t allocator);
bool cff_string_compare(cff_string a, cff_string b);
cff_ord_e cff_string_order(cff_string a, cff_string b);
cff_string cff_string_upper(cff_string str);
cff_string cff_string_lower(cff_string str);
cff_string cff_string_join(cff_string *strs, char sep, uint64_t array_lenght,
                           cff_allocator_t allocator);
cff_string cff_string_append(cff_string str, char sep, cff_string other,
                             cff_allocator_t allocator);
cff_err_e cff_string_split(cff_string str, char sep, cff_string *out_array,
                           uint64_t *array_lenght, cff_allocator_t allocator);
uint64_t cff_string_count_char(cff_string str, char c);

cff_string cff_string_append_literal(cff_string str, char sep, char *literal,
                                     uint64_t literal_len,
                                     cff_allocator_t allocator);