#include "caffeine_string.h"

static uint64_t _cff_str_len(const char *restrict str) {
  uint64_t len = 0;
  while (str[len++] != '\0')
    ;
  return len;
}

static uint64_t _cff_str_cpy(const char *str, char *buffer) {
  uint64_t len = 0;
  while (true) {
    buffer[len] = str[len];
    if (str[len] == '\0')
      break;
    len++;
  }
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

// TODO: handle alloc error
cff_string cff_string_create_literal(const char *str, cff_allocator allocator) {

  uint64_t lenght = _cff_str_len(str);

  char *bff = NULL;

  cff_size alloc_size = (cff_size)(lenght * sizeof(char));

  cff_err_e bff_err =
      cff_allocator_alloc(allocator, alloc_size, (uintptr_t *)(&bff));

  if (IS_ERROR(bff_err)) {
    return (cff_string){0};
  }

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
                             cff_allocator allocator) {
  char *bff = NULL;

  cff_size alloc_size = (cff_size)(lenght * sizeof(char));

  cff_err_e bff_err =
      cff_allocator_alloc(allocator, alloc_size, (uintptr_t *)(&bff));

  if (IS_ERROR(bff_err)) {
    return (cff_string){0};
  }

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
                            cff_allocator allocator) {

  if (from->len < lenght) {
    char *bff = NULL;

    cff_size alloc_size = (cff_size)(lenght * sizeof(char));

    cff_err_e bff_err = cff_allocator_realloc(
        allocator, (uintptr_t)from->buffer, alloc_size, (uintptr_t *)(&bff));

    if (IS_ERROR(bff_err)) {
      return bff_err;
    }

    from->buffer = bff;
    from->len = lenght;
  }

  uint64_t copied = _cff_str_cpy(str, from->buffer);

  if (copied > lenght - 1)
    copied = lenght - 1;

  from->buffer[copied] = '\0';
  from->len = lenght;

  return CFF_ERR_NONE;
}

cff_string cff_string_copy(cff_string from, cff_allocator allocator) {
  return cff_string_create(from.buffer, from.len, allocator);
}

void cff_string_destroy(cff_string *from, cff_allocator allocator) {
  cff_allocator_release(allocator, (uintptr_t)from->buffer);
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
                           cff_allocator allocator) {
  uint64_t result_len = 0;
  for (size_t i = 0; i < array_lenght; i++) {
    result_len += strs[i].len - 1;
  }

  result_len += 1;

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
                           uint64_t *array_lenght, cff_allocator allocator) {

  char *tmp_buffer = NULL;

  cff_err_e tmp_err =
      cff_allocator_alloc(allocator, str.len, (uintptr_t *)(&tmp_buffer));

  if (IS_ERROR(tmp_err)) {
    return tmp_err;
  }

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
      *array_lenght = (*array_lenght) + 1;
    }
  }

  cff_allocator_release(allocator, (uintptr_t)tmp_buffer);

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

// TODO : handle error
cff_string cff_string_append(cff_string str, char sep, cff_string other,
                             cff_allocator allocator) {

  cff_size new_len = str.len + other.len;

  if (sep != 0)
    new_len += 1;

  char *tmp_buffer = NULL;

  cff_err_e tmp_err = cff_allocator_realloc(
      allocator, (uintptr_t)str.buffer, new_len + 1, (uintptr_t *)tmp_buffer);

  if (IS_ERROR(tmp_err)) {
    return (cff_string){0};
  }

  uint64_t tmp_buffer_counter = str.len - 1;

  if (sep != 0) {
    tmp_buffer[tmp_buffer_counter++] = sep;
  }

  for (size_t i = 0; i < other.len; i++) {
    tmp_buffer[tmp_buffer_counter] = other.buffer[i];
    tmp_buffer_counter++;
  }

  tmp_buffer[tmp_buffer_counter] = '\0';

  str.buffer = tmp_buffer;
  return str;
}

cff_string cff_string_append_literal(cff_string str, char sep, char *literal,
                                     uint64_t literal_len,
                                     cff_allocator allocator) {
  cff_size new_len = str.len + literal_len;

  if (sep != 0)
    new_len += 1;

  char *tmp_buffer = NULL;

  cff_err_e tmp_err = cff_allocator_realloc(
      allocator, (uintptr_t)str.buffer, new_len + 1, (uintptr_t *)tmp_buffer);

  if (IS_ERROR(tmp_err)) {
    return (cff_string){0};
  }

  uint64_t tmp_buffer_counter = str.len - 1;

  if (sep != 0) {
    tmp_buffer[tmp_buffer_counter++] = sep;
  }

  for (size_t i = 0; i < literal_len; i++) {
    tmp_buffer[tmp_buffer_counter] = literal[i];
    tmp_buffer_counter++;
  }

  tmp_buffer[tmp_buffer_counter] = '\0';

  str.buffer = tmp_buffer;
  return str;
}