Implement a program in assembly called `scopy`, which takes two file names as parameters:

```bash
./scopy in_file out_file
```

1. The program checks the number of arguments. If it’s not 2, it exits with code 1.
2. It attempts to open `in_file` for reading. If this fails, it exits with code 1.
3. It creates `out_file` with permissions `-rw-r--r--`. If this fails (e.g., file exists), it exits with code 1.
4. The program reads from `in_file` and writes to `out_file`. If an error occurs during reading or writing, it exits with code 1.
5. For each byte from `in_file` that is the ASCII code of 's' or 'S', it writes the byte directly to `out_file`.
6. For each sequence of bytes not containing 's' or 'S', it writes the length of the sequence (modulo 65536) as a 16-bit little-endian number.
7. The program closes the files and exits with code 0 if successful.

System calls used include `sys_open`, `sys_read`, `sys_write`, `sys_close`, and `sys_exit`. The program should buffer reads and writes for efficiency, with optimized buffer sizes noted in comments.