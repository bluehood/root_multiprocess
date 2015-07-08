C := g++ -std=c++11 -ggdb -Wall
ROOT-LIBS := `root-config --libs`
ROOT-CFLAGS := `root-config --cflags`
TARGET := TMultiProcess.o TJob.o TNote.o TServer.o TPool.o

all: $(TARGET)

%o: %cxx %h
	$(C) -c -o $@ $< $(ROOT-LIBS) $(ROOT-CFLAGS)

.PHONY: clean
clean:
	rm -f *.o *.so *.pcm *.d $(TARGET)
