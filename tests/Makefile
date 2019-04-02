SHELL:=/bin/bash
all:
	@for t in *.c; do \
	    echo -e "\n>> RUNNING: $$t ...";\
	    gcc -I ../include -L ../build $$t -lctpl -ltcc -ldl && ./a.out;\
	done
