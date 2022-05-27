#ifndef MYFLOAT_H
#define MYFLOAT_H

struct math_float_out_o {
  float o ;
} ;
typedef struct math_float_out_o Float__float_asin_out ;
typedef struct math_float_out_o Float__float_acos_out ;
typedef struct math_float_out_o Float__float_round_out ;
typedef struct math_float_out_o Float__float_sqrt_out ;
typedef struct math_float_out_o Float__float_sin_out ;
typedef struct math_float_out_o Float__float_cos_out ;
typedef struct math_float_out_o Float__float_pow_out ;
typedef struct math_float_out_o Float__int2float_out ;

void Float__float_asin_step (float,      Float__float_asin_out*) ;
void Float__float_acos_step (float,      Float__float_acos_out*) ;
void Float__float_round_step(float,      Float__float_round_out*) ;
void Float__float_sqrt_step (float,      Float__float_sqrt_out*) ;
void Float__float_sin_step  (float,      Float__float_sin_out*) ;
void Float__float_cos_step  (float,      Float__float_cos_out*) ;
void Float__float_pow_step  (float,float,Float__float_pow_out*) ;
void Float__int2float_step  (int  ,      Float__int2float_out*) ;

typedef struct {
  float ang ;
} Float__float_atan2_out ;
void Float__float_atan2_step(float,float,Float__float_atan2_out*) ;

struct math_int_out_o {
  int o ;
} ;
typedef struct math_int_out_o Float__float2int_out ;
typedef struct math_int_out_o Float__int_is_odd_out ;

void Float__float2int_step (float,Float__float2int_out*) ;
void Float__int_is_odd_step(int  ,Float__int_is_odd_out*) ;

#endif
