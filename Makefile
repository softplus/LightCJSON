# the compiler: gcc for C program, define as g++ for C++
CC = gcc

VERSION=0.0.1

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall -DVERSION=$(VERSION)
LDFLAGS=

#TARGETS=lightcjson tests/test

.PHONY : all clean test

#all: $(TARGETS)
TARGET=tests/test.o

all: lightcjson.o
	$(CC) $(CFLAGS) lightcjson.o tests/test.c -o $(TARGET) -I .

#$(TARGETS): %: lightcjson.o %.o
#	@echo in D_TARGETS for $@ and $^ 
#	$(CC) $(LDFLAGS) -o $@ $^

#%.o : %.c lightcjson.h
#	@echo in %.o for $< 
#	$(CC) $(CFLAGS) -c $<


clean:
	$(RM) -f *.o *.gcda *.gcno $(TARGETS)
	$(RM) -f tests/*.o tests/*.gcda tests/*.gcno $(TARGETS)
	$(RM) -Rf coverage


#test: all
#	echo "tests go here"
# @(which bundle > /dev/null) || (echo "Ruby Bundler is not installed"; exit -1)
# cd test && bundle install && bundle exec rake test


#$(TARGET): $(TARGET).c
#	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

#clean:
#	$(RM) $(TARGET)
