CFLAGS = -O0 -g3
OBJS =	interface.o \
		switch.o \
		switch_fwd_tbl.o \
		./klist/klist.o 
LIBS = -lpthread

switch_test: switch_test.o $(OBJS)
	$(CC) $(CFLAGS) -o switch_test switch_test.o $(OBJS) $(LIBS)

./klist/klist.o:
	make -C ./klist

clean:
	rm -rf *.o
	rm -rf switch_test