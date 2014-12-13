CC=gcc
CFLAGS="-Wall"

debug:clean
	$(CC) $(CFLAGS) -g -o opsyshwiii *.c -lpthread
stable:clean
	$(CC) $(CFLAGS) -o opsyshwiii *.c -lpthread
clean:
	rm -vfr *~ opsyshwiii
