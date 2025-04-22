CC 		= gcc
CFLAGS 		= -std=c99 -Wall -g
LIBFLAGS	= -c
LIBPATH	= ./utils
TARGET	= mainServer.o \
			mainClient.o 

.PHONY: all clean test

all: $(TARGET) 


utils.o: $(LIBPATH)/utils.c 
	$(CC) $(CFLAGS) $< $(LIBFLAGS) -o $(LIBPATH)/$@

os_lib.o: $(LIBPATH)/os_lib.c
	$(CC) $(CFLAGS) $< $(LIBFLAGS) -o $(LIBPATH)/$@

libutils.a: os_lib.o utils.o 
	ar rvs $(LIBPATH)/$@ $(LIBPATH)/$<

libobjs.a: utils.o
	ar rvs $(LIBPATH)/$@ $(LIBPATH)/$<


mainServer.o: mainServer.c libobjs.a
	$(CC) $(CFLAGS) mainServer.c -o $@ -L $(LIBPATH) -lobjs -lpthread

mainClient.o: mainClient.c libutils.a
	$(CC) $(CFLAGS) mainClient.c -o $@ -L $(LIBPATH) -lutils -lpthread


clean: 
	rm -rf *o all
	rm -rf *log
	rm -rf utils/*o
	rm -rf data/*

test: all
	./mainServer.o &
	@./tests.sh
	@./testsum.sh
