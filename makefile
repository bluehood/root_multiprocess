C := g++ -std=c++11 -ggdb
ROOT-LIBS := `root-config --libs`
ROOT-CFLAGS := `root-config --cflags`
TARGET := main

all: $(TARGET)

main: main.cxx TClientServer.o
	$(C) -o $@ $^ $(ROOT-LIBS) $(ROOT-CFLAGS)

%o: %cxx %h
	$(C) -c -o $@ $< $(ROOT-LIBS) $(ROOT-CFLAGS)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
