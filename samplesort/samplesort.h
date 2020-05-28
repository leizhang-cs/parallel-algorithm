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
#include "../SAH-build/common.h"
//#include "common.h"


template<class T>
class Samplesort{
public:
    Samplesort(compare comp_input)
    :comp(comp_input),output_begin(0)
    {}
    // if output starts from vector.begin()+output_begin
    Samplesort(compare comp_input, int output_begin_input)
    :comp(comp_input), output_begin(output_begin_input)
    {}

    void sort(std::vector<T>& arr, std::vector<T>& output);
    uint32_t hash32(uint32_t a);

private:
    compare comp;
    int output_begin;
    //atomic_int global_index(0);
    int global_index = 0;
    int subarray_size;
    int bucket_size;
    int bucket_num;
    int subarray_num;
    std::vector<T> pivots;
    std::vector<int> vec_subarray_count;
    std::vector<int> vec_bucket_T;
    std::unordered_map<int,int> array_subarray;  

    bool debug_flag = false;
    bool print_output = false;  

    void sampling(const std::vector<T>& arr, int n);
    void dac_sort_subarray(std::vector<T>& arr, int begin, int end, int th);
    void transpose(int bucket_num, int bucket_size);
    void dac_merge(std::vector<T>& arr, std::vector<T>& output, int begin, int end, int th);
    void dac_sort_bucket(std::vector<T>& arr, int b_begin, int b_end);
    void dac_allocate(int begin, int end, int th);
    //bool compare(const T& a, const T& b, const int dimension);
    double LOG(int x);
};


