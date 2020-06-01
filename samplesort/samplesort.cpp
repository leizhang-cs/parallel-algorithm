#include "samplesort.h"
#include "scan.h"
#include "../SAH-build/hierarchy.h"
using namespace std;

/*
* Usage: ./ssort [num_elements]
*/

// explicit instantiation of Samplesort<T>
template class Samplesort<Entry>;


template<class T>
inline uint32_t Samplesort<T>::hash32(uint32_t a) {
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    if (a<0) a = -a;
    return a;
}

template<class T>
inline double Samplesort<T>::LOG(int x){
    return log(x)/log(2);
}


// N^1/2 sort
// N^1/2*logn, every logn -> pivot
template<class T>
void Samplesort<T>::sort(vector<T>& arr, vector<T>& output){
    bool _debug_flag = false;
    int n = arr.size();
    subarray_size = sqrt(n);
    bucket_num = sqrt(n);
    vector<T> _pivots(bucket_num-1);
    swap(pivots, _pivots);
    
    subarray_size = sqrt(n);
    subarray_num = 2*sqrt(n);
    vector<int> _vec_subarray_count(subarray_num*subarray_size);
    swap(vec_subarray_count, _vec_subarray_count);
    //std::cout<<subarray_size<<std::endl;
    
    // sample
    sampling(arr, n);
    if(_debug_flag || debug_flag){
        cout<<endl<<"arr:"<<endl;
        //for(int i=0; i<n; i++) cout<<arr[i]<<" ";
        cout<<endl;
    }

    dac_allocate(0, n, subarray_size);
    // sort subarry
    dac_sort_subarray(arr, 0, n, subarray_size);
    if(_debug_flag || debug_flag){
        cout<<endl<<"subarray:"<<endl;
        //for(int i=0; i<n; i++) cout<<arr[i]<<" ";
        cout<<endl;
    }

    // transpose
    bucket_size = global_index;
    transpose(bucket_num, bucket_size);
    if(_debug_flag || debug_flag){
        cout<<endl<<"T:"<<endl;
        //for(auto x: vec_bucket_T) cout<<x<<" "; cout<<endl;
    }

    // scan
    for(size_t i=1; i<vec_bucket_T.size(); i++) vec_bucket_T[i] += vec_bucket_T[i-1];
    /*int* temp = &vec_bucket_T.front();
    int* e1 = new int[bucket_num*bucket_size];
    int* e2 = new int[bucket_num*bucket_size];
    scan(temp, temp, e1, e2, vec_bucket_T.size());*/
    if(_debug_flag || debug_flag){
        cout<<"scan:"<<endl;
        //for(auto x: vec_bucket_T) cout<<x<<" "; cout<<endl;
    }

    // merge bucket
    dac_merge(arr, output, 0, n, subarray_size);
    if(_debug_flag || debug_flag){
        cout<<endl<<"merge:"<<endl;
        //for(int i=0; i<n; i++) cout<<output[i]<<" "; cout<<endl;
    }
    // sort bucket
    dac_sort_bucket(output, 0, bucket_num);
    if(print_output){
        if(debug_flag || _debug_flag) cout<<endl<<"output:"<<endl;
        //for(int i=0; i<n; i++) cout<<output[i]<<" "; cout<<endl;
    }
}

template<class T>
void Samplesort<T>::dac_sort_bucket(vector<T>& arr, int b_begin, int b_end){
    int n = b_end - b_begin;
    if(n<=0) return;
    if(n==1){
        int arr_begin = b_begin*bucket_size>0? vec_bucket_T[b_begin*bucket_size-1]: 0,
            arr_end = vec_bucket_T[b_end*bucket_size-1];
        if(debug_flag) cout<<"bucket:"<<b_begin<<", arr:"<<arr_begin<<" "<<arr_end<<endl;
        // local_begin, local_end + output_global_begin
        std::sort(arr.begin()+arr_begin+output_begin, arr.begin()+arr_end+output_begin, comp);
    }
    else{
        cilk_spawn
        dac_sort_bucket(arr, b_begin, b_begin+n/2);
        dac_sort_bucket(arr, b_begin+n/2, b_end);
        cilk_sync;
    }
}


// move elements of a subarray into corresponding buckets
template<class T>
void Samplesort<T>::dac_merge(vector<T>& arr, vector<T>& output, int begin, int end, int th){
    int n = end - begin;
    if(n<=0) return;
    if(n<=th){
        int s_i = array_subarray[begin];
        //cout<<"s_i:"<<s_i<<endl;

        // j_th bucket
        for(int i=begin, j=0; j<th; j++){
            int o_begin = j*bucket_size+s_i>0? vec_bucket_T[j*bucket_size+s_i-1]: 0,
                o_end = vec_bucket_T[j*bucket_size+s_i];
            // output_index = local_index + output_global_begin
            for(int k=o_begin+output_begin; k<o_end+output_begin; k++, i++){
                output[k] = arr[i];
            }
        }
        //cout<<begin<<" "<<i<<", "<<s->begin<<" "<<s->end<<endl;
    }
    else{
        cilk_spawn
        dac_merge(arr, output, begin, begin+n/2, th);
        dac_merge(arr, output, begin+n/2, end, th);
        cilk_sync;
    }
}

template<class T>
void Samplesort<T>::dac_allocate(int begin, int end, int th){
    int n = end - begin;
    if(n<=th){
        array_subarray[begin] = global_index++;
    }
    else{
        dac_allocate(begin, begin+n/2, th);
        dac_allocate(begin+n/2, end, th);
    }
}

// th = bucket_size = pivots.size()+1
template<class T>
void Samplesort<T>::dac_sort_subarray(vector<T>& arr, int begin, int end, int th){
    int n = end - begin;
    if(n<=0) return;
    if(n<=th){
        int local_index = array_subarray[begin];
        std::sort(arr.begin()+begin, arr.begin()+end, comp);

        int i=begin, j=0, offset=local_index*subarray_size;
        int count=0;
        //cout<<"offset: "<<offset<<endl;

        while(i<end && j<th-1){
            // TODO
            if(comp.operator()(arr[i],pivots[j])){
                count++;
                i++;
            }
            else{
                vec_subarray_count[offset+j] = count;
                count = 0;
                j++;
            }
        }
        if(i<end){ vec_subarray_count[offset+j] = end-i; }
        else if(count!=0){ vec_subarray_count[offset+j] = count; }
        
        if(debug_flag){
            cout<<"count: ";
            for(int j=0; j<subarray_size; j++){
                cout<<vec_subarray_count[offset+j]<<" ";
            }
            cout<<endl;
        }
    }
    else{
        cilk_spawn
        dac_sort_subarray(arr, begin, begin+n/2, th);
        dac_sort_subarray(arr, begin+n/2, end, th);
        cilk_sync;
    }
}

template<class T>
void Samplesort<T>::transpose(int bucket_num, int bucket_size){
    // bucket_num = sqrt(n), bucket_size = global_index
    vector<int> _vec_bucket_T(bucket_num*bucket_size);
    swap(vec_bucket_T, _vec_bucket_T);

    cilk_for(int i=0; i<bucket_num; i++){
        int offset = i * bucket_size;
        for(int j=0; j<bucket_size; j++){
            vec_bucket_T[offset+j] = vec_subarray_count[j*subarray_size+i];
        }
    }
}


template<class T>
void Samplesort<T>::sampling(const vector<T>& arr, int n){
    bool _debug_flag = false;

    int bucket_size = sqrt(n), logn = LOG(n);
    int bucket_num = n/bucket_size;
    vector<int> bucket_index(bucket_num);
    vector<int> sample_bucket(logn);
    if(logn>bucket_num){ cout<<"cannot sample"<<endl; exit(1); }
    // choose logn buckets randomly
    for(int i=0; i<bucket_num; i++) bucket_index[i] = i;
    
    for(int i=0; i<logn; i++){
        int j = hash32(i)%(bucket_num-i);
        sample_bucket[i] = bucket_index[j];
        swap(bucket_index[bucket_num-1-i], bucket_index[j]);
    }
    
    // N^1/2*logn samples
    vector<T> arr_sample(bucket_size*logn);
    for(int i=0; i<logn; i++){
        int offset1 = bucket_size*i;
        int offset2 = bucket_size*sample_bucket[i];
        for(int j=0; j<bucket_size; j++){
            // cout<<"ij: "<<offset1+j<<" "<<arr_sample.size()<<endl;
            // cout<<"ij: "<<sample_bucket[i]<<" "<<offset2+j<<" "<<arr.size()<<endl;
            arr_sample[offset1+j] = arr[offset2+j];
        }
    }
    if(_debug_flag) cout<<"sampling sort"<<endl;
    std::sort(arr_sample.begin(), arr_sample.end(), comp);
    // N^1/2 - 1 pivots

    if(_debug_flag) cout<<"sampling pivots assign"<<endl;
    for(int i=0; i<bucket_size-1; i++){
        pivots[i] = arr_sample[logn*(i+1)];
        // if(_debug_flag){
        //     cout<<pivots[i]<<" ";
        //     if(i==bucket_size-2) cout<<endl;
        // }
    }
}
