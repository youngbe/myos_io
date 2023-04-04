#define PRINTF_LDBL_USES_FLOAT128	0x0008
#define PRINTF_LDBL_IS_DBL		0x0001

#include "printf_buffer.h"
#include "printf_in.h"
#include "errno_in.h"
#include "_itoa.h"
#include "array_length.h"

#include <sys/param.h>

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#ifdef COMPILE_WPRINTF
#include <wctype.h>
#endif

#define ARGCHECK(S, Format) \
  do									     \
    {									     \
      /* Check file argument for consistence.  */			     \
      CHECK_FILE (S, -1);						     \
      if (S->_flags & _IO_NO_WRITES)					     \
       {								     \
	 S->_flags |= _IO_ERR_SEEN;					     \
	 __set_errno (EBADF);						     \
	 return -1;							     \
       }								     \
      if (Format == NULL)						     \
       {								     \
	 __set_errno (EINVAL);						     \
	 return -1;							     \
       }								     \
    } while (0)
#define UNBUFFERED_P(S) ((S)->_flags & _IO_UNBUFFERED)

#if __HAVE_FLOAT128_UNLIKE_LDBL
# define PARSE_FLOAT_VA_ARG_EXTENDED(INFO)				      \
  do									      \
    {									      \
      if (is_long_double						      \
	  && (mode_flags & PRINTF_LDBL_USES_FLOAT128) != 0)		      \
	{								      \
	  INFO.is_binary128 = 1;					      \
	  the_arg.pa_float128 = va_arg (ap, _Float128);			      \
	}								      \
      else								      \
	{								      \
	  PARSE_FLOAT_VA_ARG (INFO);					      \
	}								      \
    }									      \
  while (0)
#else
# define PARSE_FLOAT_VA_ARG_EXTENDED(INFO)				      \
  PARSE_FLOAT_VA_ARG (INFO);
#endif

#define PARSE_FLOAT_VA_ARG(INFO)					      \
  do									      \
    {									      \
      INFO.is_binary128 = 0;						      \
      if (is_long_double)						      \
	the_arg.pa_long_double = va_arg (ap, long double);		      \
      else								      \
	the_arg.pa_double = va_arg (ap, double);			      \
    }									      \
  while (0)

#if __HAVE_FLOAT128_UNLIKE_LDBL
# define SETUP_FLOAT128_INFO(INFO)					      \
  do									      \
    {									      \
      if ((mode_flags & PRINTF_LDBL_USES_FLOAT128) != 0)		      \
	INFO.is_binary128 = is_long_double;				      \
      else								      \
	INFO.is_binary128 = 0;						      \
    }									      \
  while (0)
#else
# define SETUP_FLOAT128_INFO(INFO)					      \
  do									      \
    {									      \
      INFO.is_binary128 = 0;						      \
    }									      \
  while (0)
#endif

#ifndef COMPILE_WPRINTF
# include "printf_buffer-char.h"
# define vfprintf	__vfprintf_internal
# define OTHER_CHAR_T   wchar_t
# define UCHAR_T	unsigned char
# define INT_T		int
typedef const char *THOUSANDS_SEP_T;
# define L_(Str)	Str
# define ISDIGIT(Ch)	((unsigned int) ((Ch) - '0') < 10)
# define STR_LEN(Str)	strlen (Str)

# define ORIENT		if (_IO_vtable_offset (s) == 0 && _IO_fwide (s, -1) != -1)\
			  return -1
# define CONVERT_FROM_OTHER_STRING __wcsrtombs
#else
# include "printf_buffer-wchar_t.h"
# define vfprintf	__vfwprintf_internal
# define OTHER_CHAR_T   char
/* This is a hack!!!  There should be a type uwchar_t.  */
# define UCHAR_T	unsigned int /* uwchar_t */
# define INT_T		wint_t
typedef wchar_t THOUSANDS_SEP_T;
# define L_(Str)	L##Str
# define ISDIGIT(Ch)	((unsigned int) ((Ch) - L'0') < 10)
# define STR_LEN(Str)	__wcslen (Str)

# include <_itowa.h>

# define ORIENT		if (_IO_fwide (s, 1) != 1) return -1
# define CONVERT_FROM_OTHER_STRING __mbsrtowcs

# undef _itoa
# define _itoa(Val, Buf, Base, Case) _itowa (Val, Buf, Base, Case)
# define _itoa_word(Val, Buf, Base, Case) _itowa_word (Val, Buf, Base, Case)
# undef EOF
# define EOF WEOF
#endif

/* Include the shared code for parsing the format string.  */
#include "printf-parse.h"

/* For handling long_double and longlong we use the same flag.  If
   `long' and `long long' are effectively the same type define it to
   zero.  */
#if LONG_MAX == LONG_LONG_MAX
# define is_longlong 0
#else
# define is_longlong is_long_double
#endif

/* If `long' and `int' is effectively the same type we don't have to
   handle `long separately.  */
#if INT_MAX == LONG_MAX
# define is_long_num	0
#else
# define is_long_num	is_long
#endif


/* Global constants.  */
static const CHAR_T null[] = L_("(null)");

/* Size of the work_buffer variable (in characters, not bytes.  */
enum { WORK_BUFFER_SIZE = 1000 / sizeof (CHAR_T) };

/* This table maps a character into a number representing a class.  In
   each step there is a destination label for each class.  */
static const uint8_t jump_table[] =
  {
    /* ' ' */  1,            0,            0, /* '#' */  4,
	       0, /* '%' */ 14,            0, /* '\''*/  6,
	       0,            0, /* '*' */  7, /* '+' */  2,
	       0, /* '-' */  3, /* '.' */  9,            0,
    /* '0' */  5, /* '1' */  8, /* '2' */  8, /* '3' */  8,
    /* '4' */  8, /* '5' */  8, /* '6' */  8, /* '7' */  8,
    /* '8' */  8, /* '9' */  8,            0,            0,
	       0,            0,            0,            0,
	       0, /* 'A' */ 26, /* 'B' */ 30, /* 'C' */ 25,
	       0, /* 'E' */ 19, /* F */   19, /* 'G' */ 19,
	       0, /* 'I' */ 29,            0,            0,
    /* 'L' */ 12,            0,            0,            0,
	       0,            0,            0, /* 'S' */ 21,
	       0,            0,            0,            0,
    /* 'X' */ 18,            0, /* 'Z' */ 13,            0,
	       0,            0,            0,            0,
	       0, /* 'a' */ 26, /* 'b' */ 30, /* 'c' */ 20,
    /* 'd' */ 15, /* 'e' */ 19, /* 'f' */ 19, /* 'g' */ 19,
    /* 'h' */ 10, /* 'i' */ 15, /* 'j' */ 28,            0,
    /* 'l' */ 11, /* 'm' */ 24, /* 'n' */ 23, /* 'o' */ 17,
    /* 'p' */ 22, /* 'q' */ 12,            0, /* 's' */ 21,
    /* 't' */ 27, /* 'u' */ 16,            0,            0,
    /* 'x' */ 18,            0, /* 'z' */ 13
  };

#define NOT_IN_JUMP_RANGE(Ch) ((Ch) < L_(' ') || (Ch) > L_('z'))
#define CHAR_CLASS(Ch) (jump_table[(INT_T) (Ch) - L_(' ')])
#define LABEL(Name) do_##Name
#ifdef SHARED
  /* 'int' is enough and it saves some space on 64 bit systems.  */
# define JUMP_TABLE_TYPE const int
# define JUMP_TABLE_BASE_LABEL do_form_unknown
# define REF(Name) &&do_##Name - &&JUMP_TABLE_BASE_LABEL
# define JUMP(ChExpr, table)						      \
      do								      \
	{								      \
	  int offset;							      \
	  void *ptr;							      \
	  spec = (ChExpr);						      \
	  offset = NOT_IN_JUMP_RANGE (spec) ? REF (form_unknown)	      \
	    : table[CHAR_CLASS (spec)];					      \
	  ptr = &&JUMP_TABLE_BASE_LABEL + offset;			      \
	  goto *ptr;							      \
	}								      \
      while (0)
#else
# define JUMP_TABLE_TYPE const void *const
# define REF(Name) &&do_##Name
# define JUMP(ChExpr, table)						      \
      do								      \
	{								      \
	  const void *ptr;						      \
	  spec = (ChExpr);						      \
	  ptr = NOT_IN_JUMP_RANGE (spec) ? REF (form_unknown)		      \
	    : table[CHAR_CLASS (spec)];					      \
	  goto *ptr;							      \
	}								      \
      while (0)
#endif

#define STEP0_3_TABLE							      \
    /* Step 0: at the beginning.  */					      \
    static JUMP_TABLE_TYPE step0_jumps[31] =				      \
    {									      \
      REF (form_unknown),						      \
      REF (flag_space),		/* for ' ' */				      \
      REF (flag_plus),		/* for '+' */				      \
      REF (flag_minus),		/* for '-' */				      \
      REF (flag_hash),		/* for '<hash>' */			      \
      REF (flag_zero),		/* for '0' */				      \
      REF (form_unknown),		/* for '\'' */				      \
      REF (width_asterics),	/* for '*' */				      \
      REF (width),		/* for '1'...'9' */			      \
      REF (precision),		/* for '.' */				      \
      REF (mod_half),		/* for 'h' */				      \
      REF (mod_long),		/* for 'l' */				      \
      REF (mod_longlong),	/* for 'L', 'q' */			      \
      REF (mod_size_t),		/* for 'z', 'Z' */			      \
      REF (form_percent),	/* for '%' */				      \
      REF (form_integer),	/* for 'd', 'i' */			      \
      REF (form_unsigned),	/* for 'u' */				      \
      REF (form_octal),		/* for 'o' */				      \
      REF (form_hexa),		/* for 'X', 'x' */			      \
      REF (form_unknown),		/* for 'E', 'e', 'F', 'f', 'G', 'g' */	      \
      REF (form_character),	/* for 'c' */				      \
      REF (form_string),	/* for 's', 'S' */			      \
      REF (form_pointer),	/* for 'p' */				      \
      REF (form_number),	/* for 'n' */				      \
      REF (form_unknown),	/* for 'm' */				      \
      REF (form_unknown),	/* for 'C' */				      \
      REF (form_unknown),	/* for 'A', 'a' */			      \
      REF (mod_ptrdiff_t),      /* for 't' */				      \
      REF (mod_intmax_t),       /* for 'j' */				      \
      REF (form_unknown),		/* for 'I' */				      \
      REF (form_binary),	/* for 'B', 'b' */			      \
    };									      \
    /* Step 1: after processing width.  */				      \
    static JUMP_TABLE_TYPE step1_jumps[31] =				      \
    {									      \
      REF (form_unknown),						      \
      REF (form_unknown),	/* for ' ' */				      \
      REF (form_unknown),	/* for '+' */				      \
      REF (form_unknown),	/* for '-' */				      \
      REF (form_unknown),	/* for '<hash>' */			      \
      REF (form_unknown),	/* for '0' */				      \
      REF (form_unknown),	/* for '\'' */				      \
      REF (form_unknown),	/* for '*' */				      \
      REF (form_unknown),	/* for '1'...'9' */			      \
      REF (precision),		/* for '.' */				      \
      REF (mod_half),		/* for 'h' */				      \
      REF (mod_long),		/* for 'l' */				      \
      REF (mod_longlong),	/* for 'L', 'q' */			      \
      REF (mod_size_t),		/* for 'z', 'Z' */			      \
      REF (form_percent),	/* for '%' */				      \
      REF (form_integer),	/* for 'd', 'i' */			      \
      REF (form_unsigned),	/* for 'u' */				      \
      REF (form_octal),		/* for 'o' */				      \
      REF (form_hexa),		/* for 'X', 'x' */			      \
      REF (form_unknown),		/* for 'E', 'e', 'F', 'f', 'G', 'g' */	      \
      REF (form_character),	/* for 'c' */				      \
      REF (form_string),	/* for 's', 'S' */			      \
      REF (form_pointer),	/* for 'p' */				      \
      REF (form_number),	/* for 'n' */				      \
      REF (form_unknown),	/* for 'm' */				      \
      REF (form_unknown),	/* for 'C' */				      \
      REF (form_unknown),	/* for 'A', 'a' */			      \
      REF (mod_ptrdiff_t),      /* for 't' */				      \
      REF (mod_intmax_t),       /* for 'j' */				      \
      REF (form_unknown),       /* for 'I' */				      \
      REF (form_binary),	/* for 'B', 'b' */			      \
    };									      \
    /* Step 2: after processing precision.  */				      \
    static JUMP_TABLE_TYPE step2_jumps[31] =				      \
    {									      \
      REF (form_unknown),						      \
      REF (form_unknown),	/* for ' ' */				      \
      REF (form_unknown),	/* for '+' */				      \
      REF (form_unknown),	/* for '-' */				      \
      REF (form_unknown),	/* for '<hash>' */			      \
      REF (form_unknown),	/* for '0' */				      \
      REF (form_unknown),	/* for '\'' */				      \
      REF (form_unknown),	/* for '*' */				      \
      REF (form_unknown),	/* for '1'...'9' */			      \
      REF (form_unknown),	/* for '.' */				      \
      REF (mod_half),		/* for 'h' */				      \
      REF (mod_long),		/* for 'l' */				      \
      REF (mod_longlong),	/* for 'L', 'q' */			      \
      REF (mod_size_t),		/* for 'z', 'Z' */			      \
      REF (form_percent),	/* for '%' */				      \
      REF (form_integer),	/* for 'd', 'i' */			      \
      REF (form_unsigned),	/* for 'u' */				      \
      REF (form_octal),		/* for 'o' */				      \
      REF (form_hexa),		/* for 'X', 'x' */			      \
      REF (form_unknown),		/* for 'E', 'e', 'F', 'f', 'G', 'g' */	      \
      REF (form_character),	/* for 'c' */				      \
      REF (form_string),	/* for 's', 'S' */			      \
      REF (form_pointer),	/* for 'p' */				      \
      REF (form_number),	/* for 'n' */				      \
      REF (form_unknown),	/* for 'm' */				      \
      REF (form_unknown),	/* for 'C' */				      \
      REF (form_unknown),	/* for 'A', 'a' */			      \
      REF (mod_ptrdiff_t),      /* for 't' */				      \
      REF (mod_intmax_t),       /* for 'j' */				      \
      REF (form_unknown),       /* for 'I' */				      \
      REF (form_binary),	/* for 'B', 'b' */			      \
    };									      \
    /* Step 3a: after processing first 'h' modifier.  */		      \
    static JUMP_TABLE_TYPE step3a_jumps[31] =				      \
    {									      \
      REF (form_unknown),						      \
      REF (form_unknown),	/* for ' ' */				      \
      REF (form_unknown),	/* for '+' */				      \
      REF (form_unknown),	/* for '-' */				      \
      REF (form_unknown),	/* for '<hash>' */			      \
      REF (form_unknown),	/* for '0' */				      \
      REF (form_unknown),	/* for '\'' */				      \
      REF (form_unknown),	/* for '*' */				      \
      REF (form_unknown),	/* for '1'...'9' */			      \
      REF (form_unknown),	/* for '.' */				      \
      REF (mod_halfhalf),	/* for 'h' */				      \
      REF (form_unknown),	/* for 'l' */				      \
      REF (form_unknown),	/* for 'L', 'q' */			      \
      REF (form_unknown),	/* for 'z', 'Z' */			      \
      REF (form_percent),	/* for '%' */				      \
      REF (form_integer),	/* for 'd', 'i' */			      \
      REF (form_unsigned),	/* for 'u' */				      \
      REF (form_octal),		/* for 'o' */				      \
      REF (form_hexa),		/* for 'X', 'x' */			      \
      REF (form_unknown),	/* for 'E', 'e', 'F', 'f', 'G', 'g' */	      \
      REF (form_unknown),	/* for 'c' */				      \
      REF (form_unknown),	/* for 's', 'S' */			      \
      REF (form_unknown),	/* for 'p' */				      \
      REF (form_number),	/* for 'n' */				      \
      REF (form_unknown),	/* for 'm' */				      \
      REF (form_unknown),	/* for 'C' */				      \
      REF (form_unknown),	/* for 'A', 'a' */			      \
      REF (form_unknown),       /* for 't' */				      \
      REF (form_unknown),       /* for 'j' */				      \
      REF (form_unknown),       /* for 'I' */				      \
      REF (form_binary),	/* for 'B', 'b' */			      \
    };									      \
    /* Step 3b: after processing first 'l' modifier.  */		      \
    static JUMP_TABLE_TYPE step3b_jumps[31] =				      \
    {									      \
      REF (form_unknown),						      \
      REF (form_unknown),	/* for ' ' */				      \
      REF (form_unknown),	/* for '+' */				      \
      REF (form_unknown),	/* for '-' */				      \
      REF (form_unknown),	/* for '<hash>' */			      \
      REF (form_unknown),	/* for '0' */				      \
      REF (form_unknown),	/* for '\'' */				      \
      REF (form_unknown),	/* for '*' */				      \
      REF (form_unknown),	/* for '1'...'9' */			      \
      REF (form_unknown),	/* for '.' */				      \
      REF (form_unknown),	/* for 'h' */				      \
      REF (mod_longlong),	/* for 'l' */				      \
      REF (form_unknown),	/* for 'L', 'q' */			      \
      REF (form_unknown),	/* for 'z', 'Z' */			      \
      REF (form_percent),	/* for '%' */				      \
      REF (form_integer),	/* for 'd', 'i' */			      \
      REF (form_unsigned),	/* for 'u' */				      \
      REF (form_octal),		/* for 'o' */				      \
      REF (form_hexa),		/* for 'X', 'x' */			      \
      REF (form_unknown),		/* for 'E', 'e', 'F', 'f', 'G', 'g' */	      \
      REF (form_character),	/* for 'c' */				      \
      REF (form_string),	/* for 's', 'S' */			      \
      REF (form_pointer),	/* for 'p' */				      \
      REF (form_number),	/* for 'n' */				      \
      REF (form_unknown),	/* for 'm' */				      \
      REF (form_unknown),	/* for 'C' */				      \
      REF (form_unknown),	/* for 'A', 'a' */			      \
      REF (form_unknown),       /* for 't' */				      \
      REF (form_unknown),       /* for 'j' */				      \
      REF (form_unknown),       /* for 'I' */				      \
      REF (form_binary),	/* for 'B', 'b' */			      \
    }

#define STEP4_TABLE							      \
    /* Step 4: processing format specifier.  */				      \
    static JUMP_TABLE_TYPE step4_jumps[31] =				      \
    {									      \
      REF (form_unknown),						      \
      REF (form_unknown),	/* for ' ' */				      \
      REF (form_unknown),	/* for '+' */				      \
      REF (form_unknown),	/* for '-' */				      \
      REF (form_unknown),	/* for '<hash>' */			      \
      REF (form_unknown),	/* for '0' */				      \
      REF (form_unknown),	/* for '\'' */				      \
      REF (form_unknown),	/* for '*' */				      \
      REF (form_unknown),	/* for '1'...'9' */			      \
      REF (form_unknown),	/* for '.' */				      \
      REF (form_unknown),	/* for 'h' */				      \
      REF (form_unknown),	/* for 'l' */				      \
      REF (form_unknown),	/* for 'L', 'q' */			      \
      REF (form_unknown),	/* for 'z', 'Z' */			      \
      REF (form_percent),	/* for '%' */				      \
      REF (form_integer),	/* for 'd', 'i' */			      \
      REF (form_unsigned),	/* for 'u' */				      \
      REF (form_octal),		/* for 'o' */				      \
      REF (form_hexa),		/* for 'X', 'x' */			      \
      REF (form_unknown),		/* for 'E', 'e', 'F', 'f', 'G', 'g' */	      \
      REF (form_character),	/* for 'c' */				      \
      REF (form_string),	/* for 's', 'S' */			      \
      REF (form_pointer),	/* for 'p' */				      \
      REF (form_number),	/* for 'n' */				      \
      REF (form_unknown),	/* for 'm' */				      \
      REF (form_unknown),	/* for 'C' */				      \
      REF (form_unknown),	/* for 'A', 'a' */			      \
      REF (form_unknown),       /* for 't' */				      \
      REF (form_unknown),       /* for 'j' */				      \
      REF (form_unknown),       /* for 'I' */				      \
      REF (form_binary),	/* for 'B', 'b' */			      \
    }

/* Handle unknown format specifier.  */
static void printf_unknown (struct Xprintf_buffer *,
			    const struct printf_info *) __THROW;

/* The buffer-based function itself.  */
void
Xprintf_buffer (struct Xprintf_buffer *buf, const CHAR_T *format,
		  va_list ap, unsigned int)
{
  /* Current character in format string.  */
  const UCHAR_T *f;

  /* End of leading constant string.  */
  const UCHAR_T *lead_str_end;

  /* Points to next format specifier.  */
  const UCHAR_T *end_of_spec;

  /* Buffer intermediate results.  */
  CHAR_T work_buffer[WORK_BUFFER_SIZE];
  CHAR_T *workend;

  /* We have to save the original argument pointer.  */
  va_list ap_save;

#ifdef __va_copy
  /* This macro will be available soon in gcc's <stdarg.h>.  We need it
     since on some systems `va_list' is not an integral type.  */
  __va_copy (ap_save, ap);
#else
  ap_save = ap;
#endif

#ifdef COMPILE_WPRINTF
  /* Find the first format specifier.  */
  f = lead_str_end = __find_specwc ((const UCHAR_T *) format);
#else
  /* Find the first format specifier.  */
  f = lead_str_end = __find_specmb ((const UCHAR_T *) format);
#endif

  /* Write the literal text before the first format.  */
  Xprintf_buffer_write (buf, format,
			  lead_str_end - (const UCHAR_T *) format);
  if (Xprintf_buffer_has_failed (buf))
    return;

  /* If we only have to print a simple string, return now.  */
  if (*f == L_('\0'))
    return;

  /* Use the slow path in case any printf handler is registered.  */
  /*
  if (__glibc_unlikely (__printf_function_table != NULL
			|| __printf_modifier_table != NULL
			|| __printf_va_arg_table != NULL))
    goto do_positional;
    */

  /* Process whole format string.  */
  do
    {
      STEP0_3_TABLE;
      STEP4_TABLE;

      int is_negative;	/* Flag for negative number.  */
      union
      {
	unsigned long long int longlong;
	unsigned long int word;
      } number;
      int base;
      CHAR_T *string;	/* Pointer to argument string.  */
      int alt = 0;	/* Alternate format.  */
      int space = 0;	/* Use space prefix if no sign is needed.  */
      int left = 0;	/* Left-justify output.  */
      int showsign = 0;	/* Always begin with plus or minus sign.  */
      /* Argument is long double/long long int.  Only used if
	 double/long double or long int/long long int are distinct.  */
      int is_long_double __attribute__ ((unused)) = 0;
      int is_short = 0;	/* Argument is short int.  */
      int is_long = 0;	/* Argument is long int.  */
      int is_char = 0;	/* Argument is promoted (unsigned) char.  */
      int width = 0;	/* Width of output; 0 means none specified.  */
      int prec = -1;	/* Precision of output; -1 means none specified.  */
      /* This flag is set by the 'I' modifier and selects the use of the
	 `outdigits' as determined by the current locale.  */
      UCHAR_T pad = L_(' ');/* Padding character.  */
      CHAR_T spec;

      workend = work_buffer + WORK_BUFFER_SIZE;

      /* Get current character in format string.  */
      JUMP (*++f, step0_jumps);

      /* ' ' flag.  */
    LABEL (flag_space):
      space = 1;
      JUMP (*++f, step0_jumps);

      /* '+' flag.  */
    LABEL (flag_plus):
      showsign = 1;
      JUMP (*++f, step0_jumps);

      /* The '-' flag.  */
    LABEL (flag_minus):
      left = 1;
      pad = L_(' ');
      JUMP (*++f, step0_jumps);

      /* The '#' flag.  */
    LABEL (flag_hash):
      alt = 1;
      JUMP (*++f, step0_jumps);

      /* The '0' flag.  */
    LABEL (flag_zero):
      if (!left)
	pad = L_('0');
      JUMP (*++f, step0_jumps);

      /* Get width from argument.  */
    LABEL (width_asterics):
      {
	const UCHAR_T *tmp;	/* Temporary value.  */

	tmp = ++f;
	if (ISDIGIT (*tmp))
	  {
	    int pos = read_int (&tmp);

	    if (pos == -1)
	      {
		__set_errno (EOVERFLOW);
		Xprintf_buffer_mark_failed (buf);
		goto all_done;
	      }
	  }
	width = va_arg (ap, int);

	/* Negative width means left justified.  */
	if (width < 0)
	  {
	    width = -width;
	    pad = L_(' ');
	    left = 1;
	  }
      }
      JUMP (*f, step1_jumps);

      /* Given width in format string.  */
    LABEL (width):
      width = read_int (&f);

      if (__glibc_unlikely (width == -1))
	{
	  __set_errno (EOVERFLOW);
	  Xprintf_buffer_mark_failed (buf);
	  goto all_done;
	}

      JUMP (*f, step1_jumps);

    LABEL (precision):
      ++f;
      if (*f == L_('*'))
	{
	  const UCHAR_T *tmp;	/* Temporary value.  */

	  tmp = ++f;
	  if (ISDIGIT (*tmp))
	    {
	      int pos = read_int (&tmp);

	      if (pos == -1)
		{
		  __set_errno (EOVERFLOW);
		  Xprintf_buffer_mark_failed (buf);
		  goto all_done;
		}

	    }
	  prec = va_arg (ap, int);

	  /* If the precision is negative the precision is omitted.  */
	  if (prec < 0)
	    prec = -1;
	}
      else if (ISDIGIT (*f))
	{
	  prec = read_int (&f);

	  /* The precision was specified in this case as an extremely
	     large positive value.  */
	  if (prec == -1)
	    {
	      __set_errno (EOVERFLOW);
	      Xprintf_buffer_mark_failed (buf);
	      goto all_done;
	    }
	}
      else
	prec = 0;
      JUMP (*f, step2_jumps);

      /* Process 'h' modifier.  There might another 'h' following.  */
    LABEL (mod_half):
      is_short = 1;
      JUMP (*++f, step3a_jumps);

      /* Process 'hh' modifier.  */
    LABEL (mod_halfhalf):
      is_short = 0;
      is_char = 1;
      JUMP (*++f, step4_jumps);

      /* Process 'l' modifier.  There might another 'l' following.  */
    LABEL (mod_long):
      is_long = 1;
      JUMP (*++f, step3b_jumps);

      /* Process 'L', 'q', or 'll' modifier.  No other modifier is
	 allowed to follow.  */
    LABEL (mod_longlong):
      is_long_double = 1;
      is_long = 1;
      JUMP (*++f, step4_jumps);

    LABEL (mod_size_t):
      is_long_double = sizeof (size_t) > sizeof (unsigned long int);
      is_long = sizeof (size_t) > sizeof (unsigned int);
      JUMP (*++f, step4_jumps);

    LABEL (mod_ptrdiff_t):
      is_long_double = sizeof (ptrdiff_t) > sizeof (unsigned long int);
      is_long = sizeof (ptrdiff_t) > sizeof (unsigned int);
      JUMP (*++f, step4_jumps);

    LABEL (mod_intmax_t):
      is_long_double = sizeof (intmax_t) > sizeof (unsigned long int);
      is_long = sizeof (intmax_t) > sizeof (unsigned int);
      JUMP (*++f, step4_jumps);

      /* Process current format.  */
      while (1)
	{
#define process_arg_int() va_arg (ap, int)
#define process_arg_long_int() va_arg (ap, long int)
#define process_arg_long_long_int() va_arg (ap, long long int)
#define process_arg_pointer() va_arg (ap, void *)
#define process_arg_string() va_arg (ap, const char *)
#define process_arg_unsigned_int() va_arg (ap, unsigned int)
#define process_arg_unsigned_long_int() va_arg (ap, unsigned long int)
#define process_arg_unsigned_long_long_int() va_arg (ap, unsigned long long int)
#define process_arg_wchar_t() va_arg (ap, wchar_t)
#define process_arg_wstring() va_arg (ap, const wchar_t *)
#include "vfprintf-process-arg.c"
#undef process_arg_int
#undef process_arg_long_int
#undef process_arg_long_long_int
#undef process_arg_pointer
#undef process_arg_string
#undef process_arg_unsigned_int
#undef process_arg_unsigned_long_int
#undef process_arg_unsigned_long_long_int
#undef process_arg_wchar_t
#undef process_arg_wstring

	LABEL (form_unknown):
      {
	    if (spec == L_('\0'))
	      {
	        /* The format string ended before the specifier is complete.  */
            __set_errno (EINVAL);
            Xprintf_buffer_mark_failed (buf);
            goto all_done;
          }
        struct printf_info info =
	      {
            .prec = prec,
            .width = width,
            .spec = spec,
            .is_long_double = is_long_double,
            .is_short = is_short,
            .is_long = is_long,
            .alt = alt,
            .space = space,
            .left = left,
            .showsign = showsign,
            .group = 0,
            .pad = pad,
            .extra = 0,
            .i18n = 0,
            .wide = sizeof (CHAR_T) != 1,
            .is_binary128 = 0
	      };
        printf_unknown(buf, &info);
      }
      break;
	}

      /* Look for next format specifier.  */
#ifdef COMPILE_WPRINTF
      f = __find_specwc ((end_of_spec = ++f));
#else
      f = __find_specmb ((end_of_spec = ++f));
#endif

      /* Write the following constant string.  */
      Xprintf_buffer_write (buf, (const CHAR_T *) end_of_spec,
			      f - end_of_spec);
    }
  while (*f != L_('\0') && !Xprintf_buffer_has_failed (buf));

 all_done:
  /* printf_positional performs cleanup under its all_done label, so
     vfprintf-process-arg.c uses it for this function and
     printf_positional below.  */
  return;
}

/* Handle an unknown format specifier.  This prints out a canonicalized
   representation of the format spec itself.  */
static void
printf_unknown (struct Xprintf_buffer *buf, const struct printf_info *info)
{
  CHAR_T work_buffer[MAX (sizeof (info->width), sizeof (info->prec)) * 3];
  CHAR_T *const workend
    = &work_buffer[sizeof (work_buffer) / sizeof (CHAR_T)];
  CHAR_T *w;

  Xprintf_buffer_putc (buf, L_('%'));

  if (info->alt)
    Xprintf_buffer_putc (buf, L_('#'));
  if (info->group)
    Xprintf_buffer_putc (buf, L_('\''));
  if (info->showsign)
    Xprintf_buffer_putc (buf, L_('+'));
  else if (info->space)
    Xprintf_buffer_putc (buf, L_(' '));
  if (info->left)
    Xprintf_buffer_putc (buf, L_('-'));
  if (info->pad == L_('0'))
    Xprintf_buffer_putc (buf, L_('0'));
  if (info->i18n)
    Xprintf_buffer_putc (buf, L_('I'));

  if (info->width != 0)
    {
      w = _itoa_word (info->width, workend, 10, 0);
      while (w < workend)
	Xprintf_buffer_putc (buf, *w++);
    }

  if (info->prec != -1)
    {
      Xprintf_buffer_putc (buf, L_('.'));
      w = _itoa_word (info->prec, workend, 10, 0);
      while (w < workend)
	Xprintf_buffer_putc (buf, *w++);
    }

  if (info->spec != L_('\0'))
    Xprintf_buffer_putc (buf, info->spec);
}
