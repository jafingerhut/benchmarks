CFLAGS=-O2 -Wall
CC=gcc
OBJECTS1=lcg-test
OBJECTS2=benchmark-branch
OBJECTS3=benchmark-core
OBJECTS4=benchmark-thrash
OBJECTS5=benchmark-thrash2

all : $(OBJECTS1) $(OBJECTS2) $(OBJECTS3) $(OBJECTS4) $(OBJECTS5)

$(OBJECTS1) : 
	$(CC) $(CFLAGS) -o $(OBJECTS1) $(OBJECTS1).c

$(OBJECTS2) : 
	$(CC) $(CFLAGS) -o $(OBJECTS2) $(OBJECTS2).c

$(OBJECTS3) : 
	$(CC) $(CFLAGS) -o $(OBJECTS3) $(OBJECTS3).c

$(OBJECTS4) : 
	$(CC) $(CFLAGS) -o $(OBJECTS4) $(OBJECTS4).c

$(OBJECTS5) : 
	$(CC) $(CFLAGS) -o $(OBJECTS5) $(OBJECTS5).c

clean :
		rm $(OBJECTS1) $(OBJECTS2) $(OBJECTS3) $(OBJECTS4) $(OBJECTS5)
