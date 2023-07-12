#include "caffeine_string.h"

static uint64_t _cff_str_len(const char *restrict str) {
  register uint64_t len = 0;
  while (str[len++] != '\0')
    ;
  return len;
}

static uint64_t _cff_str_cpy(const char *restrict str, char *buffer) {
  register uint64_t len = 0;
  while (str[len++] != '\0')
    buffer[len] = str[len];
  return len;
}

static inline char _cff_char_to_upper(char c) {
  if (c >= 'a' && c <= 'z') {
    c = c - 32;
  }
  return c;
}

static inline char _cff_char_to_lower(char c) {
  if (c >= 'A' && c <= 'Z') {
    c = c + 32;
  }
  return c;
}

cff_string cff_string_create_literal(const char *str,
                                     cff_allocator_t allocator) {
  uint64_t lenght = _cff_str_len(str);
  char *bff =
      cff_allocator_allocate(&allocator, (cff_size)(lenght * sizeof(char)));

  uint64_t copied = lenght;

  if (str != NULL)
    copied = _cff_str_cpy(str, bff);

  if (copied > lenght - 1)
    copied = lenght - 1;

  bff[copied] = '\0';

  return (cff_string){
      .buffer = bff,
      .len = lenght,
  };
}

cff_string cff_string_create(char *str, cff_size lenght,
                             cff_allocator_t allocator) {
  char *bff =
      cff_allocator_allocate(&allocator, (cff_size)(lenght * sizeof(char)));

  uint64_t copied = lenght;

  if (str != NULL)
    copied = _cff_str_cpy(str, bff);

  if (copied > lenght - 1)
    copied = lenght - 1;

  bff[copied] = '\0';

  return (cff_string){
      .buffer = bff,
      .len = lenght,
  };
}

cff_err_e cff_string_update(cff_string *from, char *str, cff_size lenght,
                            cff_allocator_t allocator) {
  if (from->len < lenght) {
    cff_string_destroy(from, allocator);
    char *bff =
        cff_allocator_allocate(&allocator, (cff_size)(lenght * sizeof(char)));
    if (bff == NULL)
      return CFF_ERR_ALLOC;
    from->buffer = bff;
  }

  uint64_t copied = _cff_str_cpy(str, from->buffer);

  if (copied > lenght - 1)
    copied = lenght - 1;

  from->buffer[copied] = '\0';
  from->len = lenght;

  return CFF_ERR_NONE;
}

cff_string cff_string_copy(cff_string from, cff_allocator_t allocator) {
  return cff_string_create(from.buffer, from.len, allocator);
}

void cff_string_destroy(cff_string *from, cff_allocator_t allocator) {
  cff_allocator_release(&allocator, from->buffer);
  from->buffer = 0;
  from->len = 0;
}

bool cff_string_compare(cff_string a, cff_string b) {
  int i = 0;
  const char *str1 = a.buffer;
  const char *str2 = b.buffer;

  while (str1[i] != '\0' && str1[i] == str2[i]) {
    i++;
  }

  return i == a.len;
}

cff_ord_e cff_string_order(cff_string a, cff_string b) {
  const char *str1 = a.buffer;
  const char *str2 = b.buffer;

  while (*str1 && *str2) {
    char ch1 = _cff_char_to_lower(*str1);
    char ch2 = _cff_char_to_lower(*str2);

    if (ch1 != ch2)
      return (cff_ord_e)(ch1 - ch2);

    str1++;
    str2++;
  }

  return (cff_ord_e)(*str1 - *str2);
}

cff_string cff_string_upper(cff_string str) {
  register uint64_t len = 0;
  while (str.buffer[len++] != '\0')
    str.buffer[len] = _cff_char_to_upper(str.buffer[len]);
  return str;
}

cff_string cff_string_lower(cff_string str) {
  register uint64_t len = 0;
  while (str.buffer[len++] != '\0')
    str.buffer[len] = _cff_char_to_lower(str.buffer[len]);
  return str;
}

cff_string cff_string_join(cff_string *strs, char sep, uint64_t array_lenght,
                           cff_allocator_t allocator) {
  uint64_t result_len = 0;
  for (size_t i = 0; i < array_lenght; i++) {
    result_len += strs[i].len;
  }

  if (sep != 0)
    result_len += array_lenght;

  cff_string new_string = cff_string_create(0, result_len, allocator);

  uint64_t acc = 0;

  for (size_t i = 0; i < array_lenght; i++) {
    char *bff = strs[i].buffer;
    while (*bff != '\0') {
      new_string.buffer[acc] = *bff;
      bff++;
      acc++;
    }
    if (sep != 0 && i != array_lenght - 1) {
      new_string.buffer[acc] = sep;
      acc++;
    }
  }
  new_string.buffer[result_len - 1] = '\0';

  return new_string;
}

cff_err_e cff_string_split(cff_string str, char sep, cff_string *out_array,
                           uint64_t *array_lenght, cff_allocator_t allocator) {
  char *tmp_buffer = (char *)cff_allocator_allocate(&allocator, str.len);
  uint64_t acc = 0;
  *array_lenght = 0;

  for (size_t i = 0; i < str.len; i++) {
    if (str.buffer[i] != sep) {
      tmp_buffer[acc] = str.buffer[i];
      acc++;
    } else {
      tmp_buffer[acc] = '\0';
      out_array[*array_lenght] = cff_string_create(tmp_buffer, acc, allocator);
      acc = 0;
      *array_lenght = *array_lenght++;
    }
  }

  cff_allocator_release(&allocator, tmp_buffer);

  return CFF_ERR_NONE;
}

uint64_t cff_string_count_char(cff_string str, char c) {
  register char *bff = str.buffer;
  register uint64_t cc = 0;
  while (*(bff++) != '\0') {
    cc += *bff == c;
  }
  return cc;
}