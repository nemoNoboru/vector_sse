//
// Copyright (c) 2015, Robert Glissmann
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

// %% license-end-token %%
// 
// Author: Robert.Glissmann@gmail.com (Robert Glissmann)
// 
// 

#include <string.h>
#include <emmintrin.h>
#include <ruby.h>
#include "vector_sse_sum.h"
#include "vector_sse_common.h"


// Check for overflow
// __m128i sign_left;
// __m128i sign_right;
// const int32_t OVERFLOW_MASK = ( (int32_t)0x1 << (32-1) );
// int32_t overflow[ 4 ];
// __m128i* overflow_vec = (__m128i*)overflow;
// sign_left = _mm_xor_si128(result_vec, left_vec);
// sign_right = _mm_xor_si128(result_vec, right_vec);
// *overflow_vec = _mm_and_si128(sign_left, sign_right);

// for ( vector_pos = 0; vector_pos < 4; ++vector_pos )
// {
//    if ( ( (int32_t)overflow[ vector_pos ] & OVERFLOW_MASK ) )
//    {
//       rb_raise( rb_eRuntimeError, "Vector addition overflow" );
//    }
// }


#define  TEMPLATE_SUM_S( FUNC_NAME, TYPE, CONV_IN, CONV_OUT, EL_PER_VEC, ADDER ) \
VALUE FUNC_NAME( VALUE self, VALUE vector ) \
{ \
   uint32_t length      = 0; \
   uint32_t offset      = 0; \
   uint32_t vector_pos  = 0; \
   uint32_t input_index = 0; \
\
   TYPE  result = 0; \
\
   TYPE result_segment[ EL_PER_VEC ]; \
   TYPE vector_segment[ EL_PER_VEC ]; \
\
   __m128i left_vec;   \
   __m128i right_vec;  \
   __m128i result_vec; \
\
   Check_Type( vector, T_ARRAY ); \
\
   length = RARRAY_LEN( vector ); \
\
   if ( length > 0 ) \
   { \
      memset( &result_vec, 0, sizeof( result_vec ) ); \
\
      for ( offset = 0; offset < length; offset += EL_PER_VEC ) \
      { \
         for ( vector_pos = 0; vector_pos < EL_PER_VEC; ++vector_pos ) \
         { \
            input_index = offset + vector_pos; \
            if ( input_index < length ) \
            { \
               vector_segment[ vector_pos ] = CONV_IN( rb_ary_entry( vector, input_index ) ); \
            } \
            else \
            { \
               vector_segment[ vector_pos ] = 0; \
            } \
         } \
\
         right_vec = _mm_loadu_si128( (const __m128i *)vector_segment ); \
         left_vec  = _mm_loadu_si128( &result_vec ); \
\
         result_vec = ADDER( left_vec, right_vec ); \
      } \
\
      _mm_store_si128( (__m128i*)result_segment, result_vec ); \
\
      for ( vector_pos = 0; vector_pos < EL_PER_VEC; ++vector_pos ) \
      { \
         result += result_segment[ vector_pos ]; \
      } \
   } \
\
   return CONV_OUT( result ); \
}

TEMPLATE_SUM_S( method_vec_sum_s32, int32_t, NUM2INT, INT2NUM, 4, _mm_add_epi32 );
TEMPLATE_SUM_S( method_vec_sum_s64, int64_t, NUM2LL, LL2NUM, 2, _mm_add_epi64 );
TEMPLATE_SUM_S( method_vec_sum_f32, float, NUM2DBL, DBL2NUM, 4, add_f32 );
TEMPLATE_SUM_S( method_vec_sum_f64, double, NUM2DBL, DBL2NUM, 2, add_f64 );

