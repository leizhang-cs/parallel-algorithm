#include "samplesort.h"
#include "common.h"

using namespace std;


int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: ./ssort [num_elements]" << endl;
        return 0;
    }

    using Type = int;

    int n = atoi(argv[1]);
    cout << n << endl;

    compare comp;
    Samplesort<Type> ssort(comp);

    // initialzation
    vector<Type> arr(n);
    vector<Type> output(n);

    /*cilk_for*/for(int i=0; i<n; i++) arr[i] = ssort.hash32(i) % (n*2);
    
    vector<Type> vec(arr);

    //samplesort
    timer t; t.start();
    ssort.sort(arr, output);
    t.stop(); cout << "time: " << t.get_total() << endl;

    timer tt; tt.start();
    sort(vec.begin(), vec.end());
    tt.stop(); cout<<"stl::sort: "<< tt.get_total() <<endl;
    int count = 0;
    for(int i=0; i<vec.size(); i++){
        if(vec[i]!=output[i]){
            count++;
        }
    }
    cout<<"diff:"<<count<<endl;

    return 0;
}