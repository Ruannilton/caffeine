#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "caffeine_flags.h"

/**
 * @typedef cff_size
 * @brief Alias for the unsigned int type representing the size of data in
 * bytes.
 */
typedef uint64_t cff_size;

/**
 * @enum cff_ord_e
 * @brief Represents the ordering relationship between two values.
 */
typedef enum {
  CFF_LESS =
      -1, /**< Indicates that the first value is less than the second value. */
  CFF_EQUAL =
      0, /**< Indicates that the first value is equal to the second value. */
  CFF_GREATER = 1, /**< Indicates that the first value is greater than the
                      second value. */
} cff_ord_e;

/**
 * @enum cff_err_e
 * @brief Error codes that can be returned by the library functions.
 */
typedef enum {
  CFF_ERR_NONE = 1,           /**< No error occurred. */
  CFF_ERR_ALLOC = -1,         /**< Error during memory allocation. */
  CFF_ERR_REALLOC = -2,       /**< Error during memory reallocation. */
  CFF_ERR_INVALID_PARAM = -3, /**< Invalid parameter passed to a function. */
  CFF_ERR_INVALID_OPERATION = -4, /**< Invalid operation performed. */
  CFF_ERR_OUT_OF_BOUNDS = -5,     /**< Accessing data out of bounds. */
  CFF_ERR_UNKNOW = -6,            /**< Unknown error occurred. */
} cff_err_e;

/**
 * @typedef cff_order_function
 * @brief Function pointer type representing a comparison function for ordering
 * data.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @param data_size Size of the data.
 * @return Ordering relationship between the two values (cff_ord_e).
 */
typedef cff_ord_e (*cff_order_function)(uintptr_t a, uintptr_t b,
                                        cff_size data_size);

/**
 * @typedef cff_comparer_function
 * @brief Function pointer type representing a comparison function for equality.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @param data_size Size of the data.
 * @return True if the values are equal, false otherwise.
 */
typedef bool (*cff_comparer_function)(uintptr_t a, uintptr_t b,
                                      cff_size data_size);

/**
 * @typedef cff_hash_function
 * @brief Function pointer type representing a hash function.
 * @param data_ptr Pointer to the data to be hashed.
 * @param data_size Size of the data.
 * @param seed Seed value for hashing.
 * @return Hash value (uint64_t).
 */
typedef uint64_t (*cff_hash_function)(uintptr_t data_ptr, cff_size data_size,
                                      uint64_t seed);

/**
 * @brief Calculates the hash value of the given data using the specified seed.
 *
 * This function calculates the hash value of the data pointed to by `data_ptr`,
 * which has a size of `data_size`, using the specified `seed`. The resulting
 * hash value is returned as a 64-bit unsigned integer.
 *
 * @param data_ptr Pointer to the data to be hashed.
 * @param data_size Size of the data in bytes.
 * @param seed Seed value for the hash calculation.
 * @return The calculated hash value.
 */
uint64_t cff_generic_hash(uintptr_t data_ptr, cff_size data_size,
                          uint64_t seed);

/**
 * @brief Performs a binary search on a sorted array.
 *
 * This function performs a binary search on the sorted array pointed to by
 * `arr_ptr` to find the specified `value_ptr`. The search is performed within
 * the range of indices from `start` to `length` (exclusive). The size of each
 * element in the array is specified by `data_size`. The comparison function
 * `cmp_fn` is used to determine the order of elements. If the value is found,
 * the index of the found element is stored in `out_index` and the function
 * returns true. If the value is not found, the function returns false and
 * `out_index` is not modified.
 *
 * @param arr_ptr Pointer to the sorted array.
 * @param value_ptr Pointer to the value to search for.
 * @param start Starting index of the search range.
 * @param length Length of the search range.
 * @param data_size Size of each element in the array.
 * @param cmp_fn Comparison function to determine the order of elements.
 * @param out_index Pointer to store the index of the found element (if found).
 * @return True if the value is found, false otherwise.
 */
bool cff_binary_search(uintptr_t arr_ptr, uintptr_t value_ptr, uint64_t start,
                       uint64_t lenght, cff_size data_size,
                       cff_order_function cmp_fn, int64_t *out_index);

/**
 * @brief Performs a quicksort algorithm on the given buffer.
 *
 * This function performs a quicksort algorithm on the buffer pointed to by
 * `buffer`. Each element in the buffer has a size of `data_size`. The sorting
 * is done based on the comparison function `predicate`. The sorting range
 * starts from index `left` and ends at index `right`. The function returns an
 * error code of type `cff_err_e` to indicate the success or failure of the
 * sorting operation.
 *
 * @param buffer Pointer to the buffer to be sorted.
 * @param data_size Size of each element in the buffer.
 * @param predicate Comparison function to determine the order of elements.
 * @param left Starting index of the sorting range.
 * @param right Ending index of the sorting range.
 * @return Error code indicating the success or failure of the sorting
 * operation.
 */
cff_err_e cff_quick_sort(uintptr_t buffer, cff_size data_size,
                         cff_order_function predicate, uint64_t left,
                         uint64_t right);

#ifdef DEBUG
#define debug_assert(x) assert(x)
#define release_assert(x) assert(x)
#else
#define debug_assert(x)
#define release_assert(x) assert(x)
#define CFF_DEBUG
#endif

#define ADDRESS_SIZE_BYTES (sizeof(void *))

#define ADDRESS_SIZE_BITS (ADDRESS_SIZE_BYTES * 8)

#define comptime_assert(x, message) _Static_assert(x, message)

#define ____concat2(a, b) a##b

#define concat_token(a, b) ____concat2(a, b)

#define macro_var(name) concat_token(name, __LINE__)

#define defer(init, end)                                                       \
  for (int macro_var(__defer_i_) = ((init), 0); !macro_var(__defer_i_);        \
       (macro_var(__defer_i_) += 1), (end))

#define scope(action) defer(0, action)