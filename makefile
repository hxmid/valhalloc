all: test_c test_cc

test_c:
	gcc -o tests/test_c.exe tests/main.c tests/separatefile.c -Wall -Wextra -DVALHALLOC_ENABLE

test_cc:
	g++ -o tests/test_cc.exe tests/main.cc tests/separatefile.cc -Wall -Wextra -DVALHALLOC_ENABLE
