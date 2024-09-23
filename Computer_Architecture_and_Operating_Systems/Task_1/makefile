seq.o: seq.c
	gcc -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2 -c seq.c

memory_tests.o: memory_tests.c
	gcc -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2 -c memory_tests.c

libseq.so: seq.o memory_tests.o
	gcc -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup seq.o memory_tests.o -o libseq.so

clean: seq.o memory_tests.o libseq.so
	rm seq.o
	rm memory_tests.o
	rm libseq.so
