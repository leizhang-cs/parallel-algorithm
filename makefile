CC = g++
CFLAGS = -O3 -I../c++ -mcx16 -march=native -DCILK -fcilkplus -std=c++11

all:    reduce scan qsort ssort ssort_seq ssort_na

reduce: reduce.cpp
        $(CC) $(CFLAGS) -DNDEBUG reduce.cpp -o reduce
        
scan:   scan.cpp scan.h
        $(CC) $(CFLAGS) -DNDEBUG scan.cpp -o scan

qsort:  quicksort.cpp qsort.h
        $(CC) $(CFLAGS) -DNDEBUG quicksort.cpp -o qsort

ssort: samplesort.cpp samplesort.h
        $(CC) $(CFLAGS) -DNDEBUG samplesort.cpp -o ssort
        
ssort_seq: samplesort_seq.cpp samplesort.h
        $(CC) $(CFLAGS) -DNDEBUG samplesort_seq.cpp -o ssort_seq

ssort_na: samplesort_naive.cpp samplesort.h
        $(CC) $(CFLAGS) -DNDEBUG samplesort_naive.cpp -o ssort_na

clean:
        rm -f reduce scan qsort ssort ssort_seq ssort_na