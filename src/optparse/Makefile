test-debug:
	$(CC) $(CFLAGS) -Wall -Wextra -Wfatal-errors -g optparse_test.c
	du -b a.out

test-release:
	$(CC) $(CFLAGS) -Wall -Wextra -Wfatal-errors -Os -s -DNDEBUG optparse_test.c
	du -b a.out

optparse.o: optparse.c optparse.h
	$(CC) $(CFLAGS) -Wall -Wextra -Wfatal-errors -DNDEBUG -O2 -c optparse.c -o optparse.o

clean:
	rm -rf a.out test-debug test-release optparse.o
