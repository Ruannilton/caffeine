#pragma once
#include "../caffeine_types.h"

void cff_memory_init();
void cff_memory_end();

void *cff_mem_alloc(uint64_t size);

void *cff_mem_realloc(const void *ptr_owning, uint64_t size);

void cff_mem_release(const void *const ptr_owning);

void cff_mem_copy(const void *const from_ref, void *const dest_mut_ref, uint64_t size);

void cff_mem_move(const void *const from_ref, void *const dest_mut_ref, uint64_t size);

bool cff_mem_cmp(const void *const from_ref, const void *const dest_mut_ref, uint64_t size);

void cff_mem_set(const void *const data_ref, void *const dest_mut_ref, uint64_t data_size,
                 uint64_t buffer_lenght);
void cff_mem_zero(void *const dest_mut_ref, uint64_t buffer_lenght);

#ifdef CFF_DEBUG

void *cff_mem_alloc_dbg(uint64_t size, const char *const block_name, const char *const file, uint64_t line);

void *cff_mem_realloc_dbg(const void *ptr_owning, uint64_t size, const char *const file, uint64_t line);

void cff_mem_release_dbg(const void *const ptr_owning, const char *const file, uint64_t line);

void cff_mem_copy_dbg(const void *const from_ref, void *const dest_mut_ref, uint64_t size, const char *const file, uint64_t line);

void cff_mem_move_dbg(const void *const from_ref, void *const dest_mut_ref, uint64_t size, const char *const file, uint64_t line);

bool cff_mem_cmp_dbg(const void *const from_ref, const void *const dest_ref, uint64_t size, const char *const file, uint64_t line);

void cff_mem_set_dbg(const void *const data_ref, void *const dest_mut_ref, uint64_t data_size, uint64_t buffer_lenght, const char *const file, uint64_t line);

void cff_mem_zero_dbg(void *const dest_mut_ref, uint64_t buffer_lenght, const char *const file, uint64_t line);

#endif

#ifdef CFF_DEBUG

#define CFF_ALLOC(SIZE, NAME) cff_mem_alloc_dbg(SIZE, NAME, __CFF_FILE_NAME__, __LINE__)

#define CFF_REALLOC(PTR_OWNING, SIZE) cff_mem_realloc_dbg(PTR_OWNING, SIZE, __CFF_FILE_NAME__, __LINE__)

#define CFF_RELEASE(PTR_OWNING) cff_mem_release_dbg(PTR_OWNING, __CFF_FILE_NAME__, __LINE__)

#define CFF_COPY(FROM_REF, DEST_MUT_REF, SIZE) cff_mem_copy_dbg(FROM_REF, DEST_MUT_REF, SIZE, __CFF_FILE_NAME__, __LINE__)

#define CFF_MOVE(FROM_REF, DEST_MUT_REF, SIZE) cff_mem_move_dbg(FROM_REF, DEST_MUT_REF, SIZE, __CFF_FILE_NAME__, __LINE__)

#define CFF_CMP(FROM_REF, DEST_REF, SIZE) cff_mem_cmp_dbg(FROM_REF, DEST_REF, SIZE, __CFF_FILE_NAME__, __LINE__)

#define CFF_SET(FROM_REF, DEST_MUT_REF, DATA_SIZE, BUFFER_LENGHT) cff_mem_set_dbg(FROM_REF, DEST_MUT_REF, DATA_SIZE, BUFFER_LENGHT, __CFF_FILE_NAME__, __LINE__)

#define CFF_ZERO(DEST_MUT_REF, BUFFER_LENGHT) cff_mem_zero_dbg(DEST_MUT_REF, BUFFER_LENGHT, __CFF_FILE_NAME__, __LINE__)

#else

#define CFF_ALLOC(SIZE, NAME) cff_mem_alloc(SIZE, NAME)

#define CFF_REALLOC(PTR_OWNING, SIZE) cff_mem_realloc(PTR_OWNING, SIZE)

#define CFF_RELEASE(PTR_OWNING) cff_mem_release(PTR_OWNING)

#define CFF_COPY(FROM_REF, DEST_MUT_REF, SIZE) cff_mem_copy(FROM_REF, DEST_MUT_REF, SIZE)

#define CFF_MOVE(FROM_REF, DEST_MUT_REF, SIZE) cff_mem_move(FROM_REF, DEST_MUT_REF, SIZE)

#define CFF_CMP(FROM_REF, DEST_REF, SIZE) cff_mem_cmp(FROM_REF, DEST_REF, SIZE)

#define CFF_SET(FROM_REF, DEST_MUT_REF, DATA_SIZE, BUFFER_LENGHT) cff_mem_set(FROM_REF, DEST_MUT_REF, DATA_SIZE, BUFFER_LENGHT)

#define CFF_ZERO(DEST_MUT_REF, BUFFER_LENGHT) cff_mem_zero(DEST_MUT_REF, BUFFER_LENGHT)

#endif

#define CFF_ARR_NEW(TYPE, LENGHT, NAME) ((TYPE *)CFF_ALLOC(sizeof(TYPE) * LENGHT, NAME))

#define CFF_ARR_RESIZE(ARRAY, LENGHT) ((__typeof__(*ARRAY) *)CFF_REALLOC((void *)(ARRAY), sizeof(__typeof__(*ARRAY)) * (LENGHT)))

#define CFF_ARR_COPY(FROM, TO, LEN)                                                   \
    TO = ((__typeof__(*FROM) *)CFF_ALLOC(sizeof(__typeof__(*FROM)) * (LEN), "COPY")); \
    CFF_COPY(FROM, TO, sizeof(__typeof__(*FROM)) * (LEN));