
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++17 -Wall

all: correctness persistence 

correctness: memtable.o sstable.o kvstore.o correctness.o 

persistence: memtable.o sstable.o kvstore.o persistence.o

clean:
	-rm -f memtable sstable correctness persistence *.o
