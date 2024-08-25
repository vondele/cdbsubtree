TERARKDBROOT = /home/vondele/chess/noob/terarkdb
CDBDIRECTROOT = /home/vondele/chess/vondele/cdbdirect

LDFLAGS = -L$(TERARKDBROOT)/output/lib -L$(CDBDIRECTROOT)
LIBS = -lcdbdirect -lterarkdb -lterark-zip-r -lboost_fiber -lboost_context -ltcmalloc -pthread -lgcc -lrt -ldl -ltbb -laio -lgomp -lsnappy -llz4 -lz -lbz2

all: cdbsubtree

CXXFLAGS = -std=c++20 -O3 -g -march=native

cdbsubtree: main.cpp
	g++ $(CXXFLAGS) -I$(CDBDIRECTROOT) -o cdbsubtree main.cpp $(LDFLAGS) $(LIBS)

clean:
	rm -f cdbsubtree

format:
	clang-format -i main.cpp
