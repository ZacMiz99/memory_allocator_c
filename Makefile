all:
	gcc -o memalloc.so -fPIC -shared memalloc.c

indent:
	clang-format -i -style=Microsoft memalloc.c
	clang-format -i -style=Microsoft memalloc.h

clean:
	rm -f *.so
