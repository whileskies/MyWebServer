CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g

TARGET = MyWebServer
OBJS = ../src/main.cpp ../src/thread/*.cpp ../src/base/*.cpp ../src/log/*.cpp ../src/net/*.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET) -pthread

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)