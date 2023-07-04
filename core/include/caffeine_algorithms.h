#pragma once

#include <caffeine_core.h>
#include <caffeine_memory.h>

/**
 * @brief Calculates the hash value of the given data using the specified seed.
 *
 * This function calculates the hash value of the data pointed to by `data_ptr`, which has a size of `data_size`, using
 * the specified `seed`. The resulting hash value is returned as a 64-bit unsigned integer.
 *
 * @param data_ptr Pointer to the data to be hashed.
 * @param data_size Size of the data in bytes.
 * @param seed Seed value for the hash calculation.
 * @return The calculated hash value.
 */
uint64_t cff_generic_hash(uintptr_t data_ptr, cff_size data_size, uint64_t seed);

/**
 * @brief Performs a binary search on a sorted array.
 *
 * This function performs a binary search on the sorted array pointed to by `arr_ptr` to find the specified `value_ptr`.
 * The search is performed within the range of indices from `start` to `length` (exclusive). The size of each element in
 * the array is specified by `data_size`. The comparison function `cmp_fn` is used to determine the order of elements.
 * If the value is found, the index of the found element is stored in `out_index` and the function returns true. If the
 * value is not found, the function returns false and `out_index` is not modified.
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
bool cff_binary_search(uintptr_t arr_ptr, uintptr_t value_ptr, uint64_t start, uint64_t lenght, cff_size data_size, cff_order_function cmp_fn, int64_t *out_index);

/**
 * @brief Performs a quicksort algorithm on the given buffer.
 *
 * This function performs a quicksort algorithm on the buffer pointed to by `buffer`. Each element in the buffer has a
 * size of `data_size`. The sorting is done based on the comparison function `predicate`. The sorting range starts from
 * index `left` and ends at index `right`. The function returns an error code of type `cff_err_e` to indicate the
 * success or failure of the sorting operation.
 *
 * @param buffer Pointer to the buffer to be sorted.
 * @param data_size Size of each element in the buffer.
 * @param predicate Comparison function to determine the order of elements.
 * @param left Starting index of the sorting range.
 * @param right Ending index of the sorting range.
 * @return Error code indicating the success or failure of the sorting operation.
 */
cff_err_e cff_quick_sort(uintptr_t buffer, cff_size data_size, cff_order_function predicate, uint64_t left, uint64_t right);