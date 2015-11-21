CC=g++ -std=c++11 -Wall -fopenmp

all: PageRank

PageRank: PageRank.cc
	$(CC) -o PageRank PageRank.cc
	
clean:
	rm -f PageRank
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/daniel90/zmq/lib 		debian
#export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/home/daniel90/zmq/lib 	IOS

