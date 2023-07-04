#pragma once

#include <assert.h>

#define concat2(a,b) a ## b
#define concat_token(a,b) concat2(a, b)

#define macro_var(name) concat_token(name, __LINE__ )

#define ZERO_INIT {0}

#define defer(init,end) for(int macro_var(__defer_i_) = ((init),0); !macro_var(__defer_i_); (macro_var(__defer_i_) += 1), (end))

#define scope(action) defer(0,action)

#define define_result_type(T)  typedef struct { T value; cff_err_e error_code;} concat_token(result_, T)

#define result(T) concat_token(result_, T)

#define try(result) cff_err_e macro_var(__ok_var_) = (result); if( macro_var(__ok_var_) != CFF_ERR_NONE){return macro_var(__ok_var_);} else 

#define result_ok(result)  if((result).error_code == CFF_ERR_NONE)

#define result_err(result) if((result).error_code != CFF_ERR_NONE)

#ifdef DEBUG

#define debug_assert(x) assert(x)
#define release_assert(x) assert(x)

#else

#define debug_assert(x)
#define release_assert(x) assert(x)

#endif

#define comptime_assert(x, message) _Static_assert (x,message)