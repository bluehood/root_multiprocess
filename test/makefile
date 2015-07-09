C := g++ -std=c++11 -ggdb -Wall
ROOT-LIBS := `root-config --libs`
ROOT-CFLAGS := `root-config --cflags`
TARGET := myJob.o

all: $(TARGET)

%o: %cxx %h
	$(C) -c -o $@ $< $(ROOT-LIBS) $(ROOT-CFLAGS)

.PHONY: clean
clean:
	rm -f *.o *.so *.pcm *.d $(TARGET)
