# 编译：
```bash
clang -std=c2x "-Dlibc_hidden_data_def(x)=" -Dattribute_hidden= "-Dlibc_hidden_proto(x)=" "-DIS_IN(x)=0" -D_GNU_SOURCE vfprintf-internal.c printf_buffer_write.c printf_buffer_done.c printf_buffer_putc_1.c printf_buffer_pad_1.c itoa-udigits.c printf_buffer_flush.c test.c sprintf.c
```
