CC = /usr/bin/g++

CCFLAGS := -std=c++11 -g -Wall -Werror

INCDIR := -I/usr/include

TARGET = infMapTester
OBJS = InfluenceMap.o infMapTester.o


all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC)  $^ $(CCFLAGS)  -o $@

%.o : %.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIR) $<

clean:
	rm -rf $(OBJS) $(TARGET)

