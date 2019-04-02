all: build/libctpl.a

build/ctpl.o: source/ctpl.c
	@echo "Compiling sources"
	@gcc -std=c11 -Wall -Werror -I include -c $< -o $@ -ltcc -ldl

build/libctpl.a: build/ctpl.o
	@echo "Building Libctpl"
	@ar rcs $@ $<
	@echo "Run 'make test' to run testcases"

test:
	@make -C tests

clean:
	@rm -f build/*.[ao] tests/a.out
