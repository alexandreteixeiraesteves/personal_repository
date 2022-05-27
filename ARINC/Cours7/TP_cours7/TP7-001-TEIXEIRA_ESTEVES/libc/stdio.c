
/*  Copyright (C) 2016, Inria
    Author: Dumitru Potop-Butucaru.

    This file is part of RPi653. 

    RPi653 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    RPi653 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <libc/stdint.h>
#include <libc/stdarg.h>
#include <libc/string.h>
#include <librpi/debug.h>  // For debug printing
#include <libc/arm-eabi.h> // For div/mod
#include <libc/stdio.h>


/* The buffer must be 17 chars long. */
int uint64ascii_hex(uint64_t num,
		    char* out_buf) {
  union {
    struct {
      uint32_t lo ;
      uint32_t hi ;
    } decoder ;
    uint64_t val ;
  } x ;
  x.val = num ;
  int digit ;
  for(digit=0;digit<8;digit++){
    char tmp = x.decoder.hi & 0xf ;
    x.decoder.hi >>= 4 ;
    if(tmp <10) out_buf[7-digit]='0' + tmp ;
    else out_buf[7-digit] = 'a'+(tmp-10) ;
  }
  for(digit=0;digit<8;digit++){
    char tmp = x.decoder.lo & 0xf ;
    x.decoder.lo >>= 4 ;
    if(tmp<10) out_buf[15-digit]='0' + tmp ;
    else out_buf[15-digit] = 'a'+(tmp-10) ;
  }
  out_buf[16]='\0';
  char* tmp = out_buf ;
  while(*tmp != '0') tmp ++ ;
  if(*tmp == '\0') tmp-- ;
  strcpy(out_buf,tmp) ;
  return strlen(out_buf) ;
}

// Needed to establish max storage needed locally.
#define UINT32_BIT (CHAR_BIT*sizeof(uint32_t))
// Local storage for converting integers to
// strings.
static char uint32ascii_buf[UINT32_BIT+1] ;

int convert2ascii(unsigned int num,       /* Unsigned number to 
					     print. */
		  unsigned int base,      /* Base. */
		  unsigned int min_length,/* Min number of digits. */
		  unsigned int max_length,/* Max number of digits.
					     Minimum 1 for positive,
					     2 for negative. */
		  int          sign,      /* Should I add a '-' ?  */
		  char * out_buffer)      /* Out buffer. Attention,
					     it must have at least
					     max_length+1 bytes. */
{  
  // To print an unsigned integer, the worst case is when we print
  // MAXUINT in base 2. In this case, we need one digit for each bit
  // of the unsigned int representation. Storing this string in memory
  // requires at most UINT_BIT+1 chars.
  char *ptr = &uint32ascii_buf[UINT32_BIT] ;
  *ptr-- = '\0';
  // First check whether the base is good. It has to be at least 2,
  // but small enough to allow printing using characters '0'-'9' and
  // 'a'-'z'.
  if((base>=2)&&
     (base<=('z'-'a'+1+'9'-'0'+1))){
    // The base is correct. Perform the conversion.
    uidiv_return divresult ;
    while(num) {
      divresult = rpi_uidivmod(num,base) ;
      *ptr-- = ((divresult.rem<10)?'0':('A'-10))+divresult.rem ;
      num = divresult.quot ;
    }
    if(ptr == (uint32ascii_buf+UINT32_BIT-1)) {
      // If the number is 0, no digit was printed, yet, so
      // put a single 0.
      *ptr-- = '0' ;
    } else {
      // If the number is not 0 and I have a negative sign,
      // add it now.
      if(sign) {
	*ptr-- = '-';
      }
    } 
    if(min_length>0) {
      // Normalize the length to avoid underflow.
      if(min_length > UINT32_BIT) min_length = UINT32_BIT ;
      // If I required a minimal numner of digits, fill in with
      // spaces. 
      while(min_length>(int)(uint32ascii_buf+32-ptr-1)){
	*ptr-- = ' ' ;
      }
    }
  } else {
    // Base is incorrect. Print a sign of error.
    *ptr-- = '#';
  }
  // Reposition ptr to account for the last, useless ptr-- .
  // After this, ptr points on the start of the string.
  ptr++ ;
  // Now, conversion is finished. I still have to check if
  // the result fits the output max size requirements.
  int string_length = (int)(uint32ascii_buf+32-ptr) ;
  if(string_length>max_length){
    ptr[max_length-1] = '#';
    ptr[max_length] = '\0';
    string_length = max_length ;
  }
  memcpy(out_buffer,ptr,string_length+1) ;
  return string_length;
}

int int32ascii (int num,                /* Number to print. */
		unsigned int base,      /* Base. */
		unsigned int min_length,/* Min number of digits. */
		unsigned int max_length,/* Max number of digits.
					   Minimum 1 for positive,
					   2 for negative. */
		char * out_buffer)      /* Out buffer. Attention,
					   it must have at least
					   max_length+1 bytes. */
{ 
  int sign = ((num<0)?1:0) ;
  unsigned int unsigned_num = (sign?-num:num) ;
  return convert2ascii(unsigned_num,
		       base,
		       min_length,
		       max_length,
		       sign,
		       out_buffer) ;
}

int uint32ascii (unsigned int num,  /* Number to print. */
		 unsigned int base, /* Base. */
		 unsigned int min_length,/* Min number of digits. */
		 unsigned int max_length,/* Max number of digits.
					    Minimum 1 for positive.*/
		 char * out_buffer)      /* Out buffer. Attention,
					    it must have at least
					    max_length+1 bytes. */
{ 
  return convert2ascii(num,
		       base,
		       min_length,
		       max_length,
		       0,
		       out_buffer) ;
}


/* Partial implementation of vsnprintf. Only handles a couple of
 * format strings, sometimes not in the standard way. On errors,
 * introduces a '#' (and there should be enough place for it, 
 * always).
 */
int vsnprintf(char* restrict buf,
	      size_t buf_size,
	      const char* restrict fmt,
	      va_list va) {
  if(buf_size <= 1) {
    // No place to properly print and leave place for the
    // error sign.
    buf[0] = '\0';
    return 0 ;
  }

  // The buffer allows some printing.
  char ch;
  char* result = buf ;
  while ((ch=*(fmt++))) {
    if (ch!='%') {
      if(buf_size==2) {
	*result++ = '#' ;
	goto out ;
      }
      // No format string. Copy to output and check
      // output bounds.
      *result++ = ch ;
      buf_size -- ;
    } else {
      int sharp_modifier = 0 ;
      int min_length = 0 ;
      ch=*(fmt++);
      if(ch=='#') {
	sharp_modifier = 1 ;
	ch=*(fmt++);
      }
      if(('0'<ch)&&('9'>=ch)) {
	min_length = (int)(ch-'0') ;
	ch=*(fmt++);
      }
      switch (ch) {
	int length ;
      case 'l':
	if(buf_size - 17 <= 2) {
	  *result++ = '#' ;
	  goto out ;
	}
	length = uint64ascii_hex(va_arg(va,uint64_t),
				 result);
	result += length ;
	buf_size -= length ;
	break;	
      case 'u':
	if(buf_size - 33 <= 2) {
	  *result++ = '#' ;
	  goto out ;
	}
	length = uint32ascii(va_arg(va,uint32_t),
			     10,
			     min_length,
			     buf_size-2,
			     result);
	result += length ;
	buf_size -= length ;
	break;
      case 'd':
	if(buf_size - 33 <= 2) {
	  *result++ = '#' ;
	  goto out ;
	}
	length = int32ascii(va_arg(va,int32_t),
			    10,
			    min_length,
			    buf_size-2,
			    result);
	result += length ;
	buf_size -= length ;
	break;
      case 'x': 
      case 'X':
	if(buf_size - 11 <= 2) {
	  *result++ = '#' ;
	  goto out ;
	}
	if(sharp_modifier) {
	  result[0] = '0';
	  result[1] = 'x' ;
	  result += 2 ;
	  buf_size -= 2 ;
	}
	length = uint32ascii(va_arg(va,uint32_t),
			     16,
			     min_length,
			     buf_size-2,
			     result);
	result += length ;
	buf_size -= length ;
	break;
      case 's':
	// Notice the use of the safer strlcpy .
	if(strlcpy(result,va_arg(va, char*),buf_size-1)==
	   buf_size-1) {
	  *result++ = '#' ;
	  goto out ;
	}
	length = strlen(result) ;
	result += length ;
	buf_size -= length ;
	break;
      default:
	{
	  const char bad_format[] = "[bad format]";
	  if(strlcpy(result,bad_format,buf_size-1)==
	     buf_size-1) {
	    *result++ = '#' ;
	    goto out ;
	  }
	  length = strlen(result) ;
	  result += length ;
	  buf_size -= length ;
	  goto out ;
	}
      }
    }
  }
  out:
    *result++ = '\0' ; // Trailing \0
    return result-buf ;
}

/* The sprintf function that is exported by the header file. 
 */
int snprintf(char* restrict buffer,
	     size_t buf_size,
	     const char * restrict fmt, ...) {
  va_list va;
  va_start(va,fmt);
  int i = vsnprintf(buffer,buf_size,fmt,va);
  va_end(va);
  return i ;
}

/* I have 3 sources of textual data I would want to read: 
   - the UART console 
   - files from the SD card
   - the USB keyboard 
   I need some functions to allow reading simple data from 
   them (integer literals, strings).

   It is assumed that objects to be read are separated by
   spaces, tabs, carriage returns. 

   A second function is needed to allow the push back of
   at most one character in the stream.
 */
typedef uint8_t (*getc_function_type)(void) ;
typedef void (*push_one_char_type)(uint8_t) ;

static inline int is_space(char c) {
  return 
    (c == ' ') ||(c == '\t')||
    (c == '\r')||(c == '\n')||
    (c == '\12') ; // \12 is Form Feed
}
static inline int is_alpha(char c) {
  return 
    ((c>='A')&&(c <= 'Z'))||
    ((c>='a')&&(c <= 'z')) ;
    
}
static inline int is_digit(char c) {
  return (c >= '0') && (c <= '9') ;
}
static inline int is_ascii(char c) {
  return (c >= ' ')&&(c <= '~') ;
}

int read_uint(getc_function_type pgetc,
	      uint32_t*presult) {
  // Unsigned literals start with a digit.
  // 1975  = decimal
  // 0x123 = hexadecimal
  // 013   = NOT ALLOWED (corresponds to octal)
  uint8_t uc ;
  do {
    uc = (*pgetc)() ;
  } while(is_space(uc)) ;
  // If I don't find a digit here, it's an error.
  if(!is_digit(uc)) return 0 ;
  if(uc == '0') {
    uc = (*pgetc)() ;
    // If it starts with a 0 it should be a hex literal
    // (I don't support octals).
    if((uc != 'x')&&(uc != 'X')) return 0 ;
    *presult = 0 ;
    for(;;) {
      uc = (*pgetc)() ;
      if(is_digit(uc)) {
	*presult = 16*(*presult)+uc-'0' ;
      } else if((uc >= 'a')&&(uc <= 'f')) {
	*presult = 16*(*presult)+uc-'a'+10 ;
      } else if((uc >= 'A')&&(uc <= 'F')) {
	*presult = 16*(*presult)+uc-'A'+10 ;
      } else if(is_space(uc)) {
	// Successfull read.
	return 1 ;
      } else {
	// Error reading.
	return 0 ;
      }
    }
  } else {
    // It should be a decimal literal.
    *presult = uc ;
    for(;;) {
      uc = (*pgetc)() ;
      if(is_digit(uc)) {
	*presult = 10*(*presult)+uc-'0' ;
      } else if(is_space(uc)) {
	// Successfull read.
	return 1 ;
      } else {
	// Error reading.
	return 0 ;
      }
    }
  }
  return 0 ;
}
int read_int(getc_function_type pgetc,
	     push_one_char_type ppush,
	     int32_t*presult) {
  uint8_t uc ;
  do {
    uc = (*pgetc)() ;
  } while(is_space(uc)) ;
  if(uc != '-') {
    // Negative number
    uint32_t tmp ;
    if(read_uint(pgetc,&tmp)){
      *presult = -tmp ;
      return 1 ;
    }
  } else if(is_digit(uc)) {
    // Positive number
    (*ppush)(uc) ;
    uint32_t tmp ;
    if(read_uint(pgetc,&tmp)){
      *presult = tmp ;
      return 1 ;
    }
  } 
  return 0 ;
}
int read_identifier(getc_function_type pgetc,
		    char*presult) {
  uint8_t uc ;
  do {
    uc = (*pgetc)() ;
  } while(is_space(uc)) ;
  // We will require identifiers to start with a letter and
  // only contain certain characters.
  do {
    if(is_alpha(uc)||is_digit(uc)||(uc == '_')) {
      *presult++ = uc ;
    } else {
      return 0 ;
    }
    uc = (*pgetc)() ;
  } while(!is_space(uc)) ;
  *presult = '\0';
  return 1 ;
}
int read_string(getc_function_type pgetc,
		char* presult) {
  uint8_t uc ;
  do {
    uc = (*pgetc)() ;
  } while(is_space(uc)) ;
  // A string must start with a quote.
  if(uc != '\"') return 0 ;
  for(;;) {
    uc = (*pgetc)() ;
    if(!is_ascii(uc)) return 0 ;
    if(uc != '\"') {
      *presult = '\0';
      return 1 ;
    }
    *presult++ = uc ;
  }
}

int convert_digit(char digit,int base) {
  int result = 0 ;
  if((digit>='0')&&(digit<='9')) {
    result = digit - '0';
  } else if((digit>='a')&&(digit<='f')) {
    result = digit - 'a' + 10 ;
  } else if((digit>='A')&&(digit<='F')) {
    result = digit - 'A' + 10 ;
  } else {
    // No such digit in any base
    return -1 ;
  }
  if(result>=base) return -1 ;
  //  debug_printf("convert_digit: base:%d found digit: %d\n",
  //	       base,result) ;
  return result ;
}

/* Very simple vsscanf implementation. Its format can
   include all ascii characters except those represented
   with escape sequences. The following
   specifiers are supported: %s %d %u %x %c.
   
   Space characters are a difficult issue. I assume
   that:
   - If the format contains a space character,
     then the string may contain zero or more 
     space characters.
   - The string may contain space characters before
     and after a specifier, even though the format
     does not.
 */
int vsscanf(const char* restrict string, 
	    const char* restrict fmt,va_list va) {
  //  debug_printf("ENTER sscanf called on %s and %s\n",
  //	       string,fmt) ;
#define SKIP_SPACES(str) while(is_space(*str))str++
#define CHECK_STRING_END(str) if(*str=='\0') return scanned_items
  char ch;
  int scanned_items = 0 ;
  while ((ch=*(fmt++))) {
    if(is_space(ch)) {
      //      debug_printf("\tsscanf(100)\n") ;
      // One or more spaces in the format allow
      // one or more spaces in the string.
      SKIP_SPACES(string) ;
      // As an optimization, skip all successive format spaces.
      SKIP_SPACES(fmt) ;
    } else {
      //      debug_printf("\tsscanf(200)\n") ;
      switch(ch) {
      case '%':
	//	debug_printf("\tsscanf(300)\n") ;
	// Should be a format specifier
	SKIP_SPACES(string) ;
	// No space can be left between % and the format
	// qualifier.
	CHECK_STRING_END(fmt) ;
	ch = *(fmt++) ;
	switch(ch) {
	case 'i':
	case 'd':
	case 'x':
	case 'u':
	  //	  debug_printf("\tsscanf(310)\n") ;
	  {
	    // Common start for the numerical types.
	    int sign = 1 ; //1 for positive, -1 for negative.
	    CHECK_STRING_END(string) ;
	    if(*string=='-') {
	      sign = -1 ;
	      string ++ ;
	    }
	    int base = 10 ;
	    // Check if it's hex or octal.
	    CHECK_STRING_END(string) ;
	    if(*string == '0') {
	      if(*(string+1)!='\0') {
		if((*(string+1) == 'x')||
		   (*(string+1) == 'X')) {
		  base = 16 ;
		  string += 2 ;
		} else {
		  if((*(string+1) >= '1')&&
		     (*(string+1) <= '7')) {
		    base = 8 ;
		    string += 1 ;
		  } 
		}
	      }
	    }
	    // Now, read the digits until there remain none
	    uint32_t number = 0 ;
	    int x ;
	    do {
	      CHECK_STRING_END(string) ;
	      x = convert_digit(*string,base) ;
	      if(x>=0) {
		number = number*base + x ;
		string ++ ;
	      }
	    } while(x>=0) ;
	    // I have finished reading the number
	    if((ch=='x')||(ch=='u')) {
	      uint32_t* p_result = (uint32_t*)va_arg(va,uint32_t*) ;
	      *p_result = number ;
	    } else {
	      int32_t* p_result = (int32_t*)va_arg(va,int32_t*) ;
	      *p_result = sign*number ;
	    }
	    scanned_items ++ ;
	  }
	  break ;
	case 's':
	  //	  debug_printf("\tsscanf(320)\n") ;
	  {
	    // I do not accept strings longer than 128 characters.
#define MAX_STRING_SCANF 128
	    char str[MAX_STRING_SCANF] ;
	    char * ptr = str ;
	    for(ptr = str ;
		(*string!='\0')
		  &&(!is_space(*string))
		  &&(ptr-str<MAX_STRING_SCANF-1);
		ptr++,string++){
	      *ptr = *string ;
	    }
	    *ptr = '\0';
	    char* p_result = (char*)va_arg(va,char*) ;
	    strcpy(p_result,str) ;
	    scanned_items ++ ;
	  }
	  break ;
	case 'c':
	  //	  debug_printf("\tsscanf(330)\n") ;
	  {
	    CHECK_STRING_END(string) ;
	    char* p_result = (char*)va_arg(va,char*) ;
	    *p_result = *string++ ;
	    scanned_items ++ ;
	  }
	  break ;
	default:
	  //	  debug_printf("\tsscanf(340)\n") ;
	  // Unhandled format or error in the format.
	  return scanned_items ;
	}
	SKIP_SPACES(string) ;
	break ;
      case '\\':
	//	debug_printf("\tsscanf(400)\n") ;
	// An escape sequence
	CHECK_STRING_END(fmt) ;
	ch=*(fmt++) ;
	switch(ch) {
	case '\"':
	  CHECK_STRING_END(string) ;
	  if(*string++ != '\"') return scanned_items ;
	default:
	  // Unknown escape sequence
	  return scanned_items ;
	}
	break ;
      default:
	//debug_printf("\tsscanf(500)\n") ;
	// Other ASCII characters
	// I have to read the same in the string, or fail.
	CHECK_STRING_END(string) ;
	//	debug_printf("\tsscanf(500): compare %x and %x\n",
	//	     *string,ch) ;
	if(*string++ != ch) return scanned_items ;
      }
    }
  }
  // Finished the format string
  return scanned_items ;
}

/* The sprintf function that is exported by the header file. 
 */
int sscanf(const char* restrict string, 
	   const char * restrict fmt, ...) {
  va_list va;
  va_start(va,fmt);
  int i = vsscanf(string,fmt,va);
  va_end(va);
  return i ;
}

