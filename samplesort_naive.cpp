#include "samplesort.h"
#include "scan.h"
using namespace std;


atomic_int global_index(0);
bool debug_flag = false;
bool print_output = false;

//int* pivots;
vector<int> pivots;
//subarray** arr_subarray;
//int* arr_subarray_count;
vector<subarray*> vec_subarray;
vector<int> vec_subarray_count;
vector<int> vec_bucket_T;
int subarray_size;
int bucket_size;
unordered_map<int,int> array_subarray;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: ./ssort [num_elements]" << endl;
        return 0;
    }
    int n = atoi(argv[1]);
    cout << n << endl;

    int* arr = new int[n];
    int* output = new int[n];
    int* e1 = new int[n];
    int* e2 = new int[n];

    cilk_for(int i=0; i<n; i++) arr[i] = hash32(i) % (n*2);
    vector<int> vec(arr, arr+n);

        subarray_size = sqrt(n);
    //cout<<ceil(LOG(n/subarray_size))<<endl;
    int subarray_num = 2*sqrt(n);
    // subarray** _arr_subarray = new subarray*[2*subarray_size];
    // arr_subarray = _arr_subarray;
    // int* _arr_subarray_count = new int[2*subarray_size*subarray_size];
    // arr_subarray_count = _arr_subarray_count;
    vector<subarray*> _vec_subarray(subarray_num);
    swap(vec_subarray, _vec_subarray);
    vector<int> _vec_subarray_count(subarray_num*subarray_size);
    swap(vec_subarray_count, _vec_subarray_count);

    timer t; t.start();
    //scan(A, B, e1, e2, n);
    samplesort(arr, output, n);
    t.stop(); cout << "time: " << t.get_total() << endl;

    sort(vec.begin(), vec.end());
    for(int i=0; i<vec.size(); i++){
        if(vec[i]!=output[i]){
            cout<<"error:"<<i<<endl;
            break;
        }
    }

    return 0;
}

// N^1/2 sort
// N^1/2*logn, every logn -> pivot
// dac_sort write to B
void samplesort(int* arr, int* output, int n){
    bool _debug_flag = false;
    int bucket_num = sqrt(n);
    //int* pivots = new int[bucket_num-1];
    vector<int> _pivots(bucket_num-1);
    swap(pivots, _pivots);

    // sampling
    sampling(arr, n);
    if(_debug_flag || debug_flag){
        cout<<endl<<"arr:"<<endl;
        for(int i=0; i<n; i++) cout<<arr[i]<<" ";
        cout<<endl;
    }

    // sort subarry
    dac_sort_subarray(arr, 0, n, subarray_size);
    if(_debug_flag || debug_flag){
        cout<<endl<<"subarray:"<<endl;
        for(int i=0; i<n; i++) cout<<arr[i]<<" ";
        cout<<endl;
    }

        // transpose
    bucket_size = global_index;
    //cout<<"B: "<<bucket_num<<" "<<bucket_size<<endl;
    vector<int> _vec_bucket_T(bucket_num*bucket_size);
    swap(vec_bucket_T, _vec_bucket_T);
    transpose(bucket_num, bucket_size);
    if(_debug_flag || debug_flag){
        cout<<endl<<"T:"<<endl;
        for(auto x: vec_bucket_T) cout<<x<<" "; cout<<endl;
    }
    // scan
    int *temp = &vec_bucket_T.front();
    int* e1 = new int[bucket_num*bucket_size];
    int* e2 = new int[bucket_num*bucket_size];
    scan(temp, temp, e1, e2, vec_bucket_T.size());
    //for(int i=1; i<vec_bucket_T.size(); i++) vec_bucket_T[i] += vec_bucket_T[i-1];

    if(_debug_flag || debug_flag){
        cout<<"scan:"<<endl;
        for(auto x: vec_bucket_T) cout<<x<<" "; cout<<endl;
    }
    // merge bucket
    dac_merge(arr, output, 0, n, subarray_size);
    if(_debug_flag || debug_flag){
        cout<<endl<<"merge:"<<endl;
        for(int i=0; i<n; i++) cout<<output[i]<<" "; cout<<endl;
    }
    // sort bucket
    dac_sort_bucket(output, 0, bucket_num);
    if(print_output){
        if(debug_flag || _debug_flag) cout<<endl<<"output:"<<endl;
        for(int i=0; i<n; i++) cout<<output[i]<<" "; cout<<endl;
    }
}

void dac_sort_bucket(int* arr, int b_begin, int b_end){
    int n = b_end - b_begin;
    if(n==1){
        int arr_begin = b_begin*bucket_size>0? vec_bucket_T[b_begin*bucket_size-1]: 0,
            arr_end = vec_bucket_T[b_end*bucket_size-1];
        if(debug_flag) cout<<"bucket:"<<b_begin<<", arr:"<<arr_begin<<" "<<arr_end<<endl;
        quicksort(arr, arr_begin, arr_end);
    }
    else{
        //cilk_spawn
        dac_sort_bucket(arr, b_begin, b_begin+n/2);
        dac_sort_bucket(arr, b_begin+n/2, b_end);
        //cilk_sync;
    }
}

// move elements of a subarray into corresponding buckets
void dac_merge(int* arr, int* output, int begin, int end, int th){
    int n = end - begin;
    if(n<=th){
        subarray* s = vec_subarray[array_subarray[begin]];
        int s_i = s->index;
        //cout<<"s_i:"<<s_i<<endl;

        // j_th bucket
        for(int i=begin, j=0; j<th; j++){
            int o_begin = j*bucket_size+s_i>0? vec_bucket_T[j*bucket_size+s_i-1]: 0,
                o_end = vec_bucket_T[j*bucket_size+s_i];

            for(int k=o_begin; k<o_end; k++, i++){
                output[k] = arr[i];
            }
        }
        //cout<<begin<<" "<<i<<", "<<s->begin<<" "<<s->end<<endl;
    }
    else{
        //cilk_spawn
        dac_merge(arr, output, begin, begin+n/2, th);
        dac_merge(arr, output, begin+n/2, end, th);
        //cilk_sync;
    }
}

// th = bucket_size = pivots.size()+1
void dac_sort_subarray(int* arr, int begin, int end, int th){
    int n = end - begin;
    if(n<=th){
        int local_index = global_index;
        // while(!CAS(global_index, local_index, local_index+1)){
        //     local_index = global_index;
        // }
        global_index++;

        subarray* s = new subarray(local_index, begin, end);
        vec_subarray[local_index] = s;
        array_subarray[begin] = local_index;
        //cout<<"subarray: "<<s->index<<" "<<s->begin<<" "<<s->end<<" "<<endl;

        quicksort(arr, begin, end);


        int i=begin, j=0, offset=local_index*subarray_size;
        int count=0;
        //cout<<"offset: "<<offset<<endl;

        while(i<end && j<th-1){
            if(arr[i]<pivots[j]){
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
        //cilk_spawn
        dac_sort_subarray(arr, begin, begin+n/2, th);
        dac_sort_subarray(arr, begin+n/2, end, th);
        //cilk_sync;
    }
}

void transpose(int bucket_num, int bucket_size){
    // bucket_num = sqrt(n), bucket_size = global_index

    cilk_for(int i=0; i<bucket_num; i++){
        int offset = i * bucket_size;
        for(int j=0; j<bucket_size; j++){
            vec_bucket_T[offset+j] = vec_subarray_count[j*subarray_size+i];
        }
    }
}


void sampling(int* arr, int n){
    // TODO
    int bucket_size = sqrt(n), logn = LOG(n);
    int bucket_num = (n-1)/bucket_size + 1;
    int* bucket_index = new int[bucket_num];
    int* sample_bucket = new int[logn];

    // choose logn buckets randomly
    for(int i=0; i<bucket_num; i++) bucket_index[i] = i;
    for(int i=0; i<logn; i++){
        int j = hash32(i)%bucket_num;
        sample_bucket[i] = j;
        swap(bucket_index[i], bucket_index[j]);
    }

    // N^1/2*logn samples
    int* arr_sample = new int[bucket_size*logn];
    for(int i=0; i<logn; i++){
        int offset1 = bucket_size*i;
        int offset2 = bucket_size*sample_bucket[i];
        for(int j=0; j<bucket_size; j++){
            arr_sample[offset1+j] = arr[offset2+j];
        }
    }

    quicksort(arr_sample, 0, bucket_size*logn-1);
    // N^1/2 - 1 pivots

    for(int i=0; i<bucket_size-1; i++){
        if(debug_flag && i==0) cout<<"pivots: ";
        pivots[i] = arr_sample[logn*(i+1)];
        if(debug_flag){
            cout<<pivots[i]<<" ";
            if(i==bucket_size-2) cout<<endl;
        }
    }
}

void quicksort(int* nums, int begin, int end){
    if(begin<end-1){
        int i = begin, j = end-1, key = nums[begin];
        while(i<j){
            while(key<nums[j] && i<j) j--;
            if(i<j) nums[i++] = nums[j];
            while(nums[i]<key && i<j) i++;
            if(i<j) nums[j--] = nums[i];
        }
        nums[i] = key;
        quicksort(nums, begin, i);
        quicksort(nums, i+1, end);
    }
}