/* --- Generated the 6/5/2022 at 16:4 --- */
/* --- heptagon compiler, version 1.05.00 (compiled wed. mar. 30 19:47:23 CET 2022) --- */
/* --- Command line: /home/teabis/.opam/default/bin/heptc -I /home/teabis/.opam/default/lib/heptagon/c -target c main.ept --- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

void Main__float_to_complex_step(float f, Main__float_to_complex_out* _out) {
  _out->c.re = f;
  _out->c.im = 0.000000;
}

void Main__complex_to_float_step(Complex__complex c,
                                 Main__complex_to_float_out* _out) {
  _out->f = c.re;
}

void Main__main_reset(Main__main_mem* self) {
  {
    int i_2;
    for (i_2 = 0; i_2 < 1024; ++i_2) {
    }
  };
  {
    int i;
    for (i = 0; i < 256; ++i) {
    }
  };
  {
    int _5;
    for (_5 = 0; _5 < 256; ++_5) {
      self->s3[_5] = 0.000000;
    }
  };
  {
    int _6;
    for (_6 = 0; _6 < 256; ++_6) {
      self->s2[_6] = 0.000000;
    }
  };
  {
    int _7;
    for (_7 = 0; _7 < 256; ++_7) {
      self->s1[_7] = 0.000000;
    }
  };
}

void Main__main_step(Main__main_out* _out, Main__main_mem* self) {
  Myprog__myread_out Myprog__myread_out_st;
  Myprog__mywrite_out Myprog__mywrite_out_st;
  Main__float_to_complex_out Main__float_to_complex_out_st;
  Main__complex_to_float_out Main__complex_to_float_out_st;
  
  float v_1[768];
  float v[512];
  float s0[256];
  float t[256];
  float s[1024];
  Complex__complex c0[1024];
  Complex__complex c3[256];
  Myprog__myread_step(256, &Myprog__myread_out_st);
  {
    int _1;
    for (_1 = 0; _1 < 256; ++_1) {
      s0[_1] = Myprog__myread_out_st.samples[_1];
    }
  };
  {
    int i_7;
    for (i_7 = 0; i_7 < 256; ++i_7) {
      v[i_7] = self->s1[i_7];
    }
  };
  {
    int i_8;
    for (i_8 = 0; i_8 < 256; ++i_8) {
      v[(256+i_8)] = s0[i_8];
    }
  };
  {
    int i_5;
    for (i_5 = 0; i_5 < 256; ++i_5) {
      v_1[i_5] = self->s2[i_5];
    }
  };
  {
    int i_6;
    for (i_6 = 0; i_6 < 512; ++i_6) {
      v_1[(256+i_6)] = v[i_6];
    }
  };
  {
    int i_3;
    for (i_3 = 0; i_3 < 256; ++i_3) {
      s[i_3] = self->s3[i_3];
    }
  };
  {
    int i_4;
    for (i_4 = 0; i_4 < 768; ++i_4) {
      s[(256+i_4)] = v_1[i_4];
    }
  };
  {
    int i_2;
    for (i_2 = 0; i_2 < 1024; ++i_2) {
      Main__float_to_complex_step(s[i_2], &Main__float_to_complex_out_st);
      c0[i_2] = Main__float_to_complex_out_st.c;
    }
  };
  {
    int i_1;
    for (i_1 = 0; i_1 < 256; ++i_1) {
      c3[i_1] = c0[(i_1+0)];
    }
  };
  {
    int i;
    for (i = 0; i < 256; ++i) {
      Main__complex_to_float_step(c3[i], &Main__complex_to_float_out_st);
      t[i] = Main__complex_to_float_out_st.f;
    }
  };
  Myprog__mywrite_step(256, t, &Myprog__mywrite_out_st);
  {
    int _2;
    for (_2 = 0; _2 < 256; ++_2) {
      self->s3[_2] = self->s2[_2];
    }
  };
  {
    int _3;
    for (_3 = 0; _3 < 256; ++_3) {
      self->s2[_3] = self->s1[_3];
    }
  };
  {
    int _4;
    for (_4 = 0; _4 < 256; ++_4) {
      self->s1[_4] = s0[_4];
    }
  };;
}

