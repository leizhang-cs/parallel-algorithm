#include <stdlib.h>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <ctime>
#include <bitset>
#include <cmath>
#include <memory>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "get_time.h"

uint32_t hash32(uint32_t a);
void quicksort(int* nums, int begin, int end);
void samplesort(int* arr, int* output, int n);
void sampling(int* arr, int n);
void dac_sort_subarray(int* arr, int begin, int end, int th);
void transpose(int bucket_num, int bucket_size);
void dac_merge(int* arr, int* output, int begin, int end, int th);
void dac_sort_bucket(int* arr, int b_begin, int b_end);
void dac_allocate(int begin, int end, int th);
double LOG(int x);

struct subarray{
    int index;
    int begin;
    int end;
    subarray(){}
    subarray(int _index, int _begin, int _end)
        :index(_index), begin(_begin), end(_end)
    {}
};

inline uint32_t hash32(uint32_t a) {
        a = (a+0x7ed55d16) + (a<<12);
        a = (a^0xc761c23c) ^ (a>>19);
        a = (a+0x165667b1) + (a<<5);
        a = (a+0xd3a2646c) ^ (a<<9);
        a = (a+0xfd7046c5) + (a<<3);
        a = (a^0xb55a4f09) ^ (a>>16);
        if (a<0) a = -a;
        return a;
}

inline double LOG(int x){
    return log(x)/log(2);
}