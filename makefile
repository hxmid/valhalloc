all: test_c test_cpp

test_c:
	gcc -o test_c.exe main.c -Wall -Wextra -DVALHALLOC_ENABLE

test_cpp:
	g++ -o test_cpp.exe main.cpp -Wall -Wextra -DVALHALLOC_ENABLE
