CC = g++
CFLAGS = -O3 -I../c++ -mcx16 -march=native -DCILK -fcilkplus -std=c++11

all:    ssort ssort_seq ssort_na


ssort: samplesort.cpp samplesort.h
        $(CC) $(CFLAGS) -DNDEBUG samplesort.cpp -o ssort
        
ssort_seq: samplesort_seq.cpp samplesort.h
        $(CC) $(CFLAGS) -DNDEBUG samplesort_seq.cpp -o ssort_seq

ssort_na: samplesort_naive.cpp samplesort.h
        $(CC) $(CFLAGS) -DNDEBUG samplesort_naive.cpp -o ssort_na

clean:
        rm -f ssort ssort_seq ssort_na
