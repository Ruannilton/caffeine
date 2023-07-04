#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @typedef cff_size
 * @brief Alias for the unsigned int type representing the size of data in bytes.
 */
typedef size_t cff_size;

/**
 * @enum cff_ord_e
 * @brief Represents the ordering relationship between two values.
 */
typedef enum
{
	CFF_LESS = -1,	 /**< Indicates that the first value is less than the second value. */
	CFF_EQUAL = 0,	 /**< Indicates that the first value is equal to the second value. */
	CFF_GREATER = 1, /**< Indicates that the first value is greater than the second value. */
} cff_ord_e;

/**
 * @enum cff_err_e
 * @brief Error codes that can be returned by the library functions.
 */
typedef enum
{
	CFF_ERR_NONE = 1,		   /**< No error occurred. */
	CFF_ERR_ALLOC,			   /**< Error during memory allocation. */
	CFF_ERR_REALLOC,		   /**< Error during memory reallocation. */
	CFF_ERR_INVALID_PARAM,	   /**< Invalid parameter passed to a function. */
	CFF_ERR_INVALID_OPERATION, /**< Invalid operation performed. */
	CFF_ERR_OUT_OF_BOUNDS,	   /**< Accessing data out of bounds. */
	CFF_ERR_UNKNOW			   /**< Unknown error occurred. */
} cff_err_e;

/**
 * @typedef cff_order_function
 * @brief Function pointer type representing a comparison function for ordering data.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @param data_size Size of the data.
 * @return Ordering relationship between the two values (cff_ord_e).
 */
typedef cff_ord_e (*cff_order_function)(uintptr_t a, uintptr_t b, cff_size data_size);

/**
 * @typedef cff_comparer_function
 * @brief Function pointer type representing a comparison function for equality.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @param data_size Size of the data.
 * @return True if the values are equal, false otherwise.
 */
typedef bool (*cff_comparer_function)(uintptr_t a, uintptr_t b, cff_size data_size);

/**
 * @typedef cff_hash_function
 * @brief Function pointer type representing a hash function.
 * @param data_ptr Pointer to the data to be hashed.
 * @param data_size Size of the data.
 * @param seed Seed value for hashing.
 * @return Hash value (uint64_t).
 */
typedef uint64_t (*cff_hash_function)(uintptr_t data_ptr, cff_size data_size, uint64_t seed);
