#include "printf_buffer.h"
#include "printf_in.h"

#include <stdint.h>

int
__vsprintf_internal (char *string, size_t,
             const char *format, va_list args,
             unsigned int mode_flags)
{
  struct __printf_buffer buf;

  __printf_buffer_init_end (&buf, string, (char *) ~(uintptr_t) 0,
                  __printf_buffer_mode_sprintf);

  __printf_buffer (&buf, format, args, mode_flags);

  *buf.write_ptr = '\0';

  return __printf_buffer_done (&buf);
}

int 
sprintf(char *s, const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = __vsprintf_internal (s, -1, format, arg, 0);
  va_end (arg);

  return done;
}

int
vsprintf (char *string, const char *format, va_list args)
{
  return __vsprintf_internal (string, -1, format, args, 0);
}
