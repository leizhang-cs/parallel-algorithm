#include "SAH_BIN.h"
#include "inline_func.cpp"
#include "../samplesort/samplesort.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>


// 1 threads 0.106, 0.33
// 4 threads 0.061, 0.33

// topnodes 1 threads 4 top 0.086, 0.34
// topnodes 4  0.045, 0.34
//          8  0.043, 0.36
//          16 0.041, 0.37
//          32 0.039, 0.39
//          64 0.037, 0.41
void SAH_BIN::Build(std::vector<Entry>& entries){
    int n = entries.size();
    cilk_for(int i=0; i<n; i++) entries[i].box.center = entries[i].box.Center();
    coarsening.push_back(std::max(n/8, static_cast<int>(log(n)*sqrt(n))));
    coarsening.push_back(std::min(coarsening[0]/8, static_cast<int>(log(n)*sqrt(n))));
    nodes.resize(2*n-1);

    int k = 4;
    topNodes.resize(2*k-1);

    int d = (n-1) / k + 1;
    cilk_for(int i=0; i<n; i+=d) {
        int end = std::min(n, i+d);
        BIN_Build(topNodes[i/d+k-1], entries, i, end, 2*i+1);
    }
    
    for(int i=k-2; i>=0; i--){
        topNodes[i].box = topNodes[2*i+2].box.Union(topNodes[2*i+1].box);
        topNodes[i].lChild = &topNodes[2*i+2];
        topNodes[i].rChild = &topNodes[2*i+1];
    }
    root = &topNodes[0];

    // root = &nodes[0];
    // BIN_Build(nodes[0], entries, 0, n, 1);
}

// binned build
void SAH_BIN::BIN_Build(Node& curr, std::vector<Entry>& entries, int begin, int end, int nth_node)
{
    if(end-begin<=threshold){
        Make_Leaf(curr, entries, begin, end);
    }
    else{
        int local_bucket_num = buckets_num;
        if(begin-end<coarsening[0]){
           local_bucket_num = end-begin<coarsening[1]? 4: std::max(buckets_num/2,4);
        }
        // find longest dimension
        int dimension = 0;
        double largest_dist = -std::numeric_limits<double>::infinity(), lo_dist = 0;
        findLongestDim(dimension, largest_dist, lo_dist, entries, begin ,end);

        // buckets: bounding boxes
        std::vector<Box> buckets(local_bucket_num);
        // map: buckets index -> entries index
        std::vector<int> BucketToEntry(local_bucket_num);
        for(size_t i=0; i<buckets.size(); i++) buckets[i].Make_Empty();

        // Method 1: sort and bucket 
        if(sorting_scheme){
            if(end-begin<1e4){ // stl::sort
                std::sort(entries.begin()+begin, entries.begin()+end, compare(dimension));
            }
            else{ //samplesort
                compare comp(dimension);
                Samplesort<Entry> ssort(comp, begin);
                std::vector<Entry> arr(entries.begin()+begin, entries.begin()+end);
                ssort.sort(arr, entries);
            }
            // bucketing
            bucketing(entries, begin, end, dimension, largest_dist, lo_dist, buckets, BucketToEntry);
        }
        else{ // Method 2: bucketing with partition
            bucketing(entries, begin, end, lo_dist, lo_dist+largest_dist, largest_dist/local_bucket_num, \
                dimension, buckets, BucketToEntry, 0);
        }
	
        // find best partition, right half entries' index
        int en_index = findBestPartition(buckets, BucketToEntry, curr.box);
        
        int lNodes = (en_index-begin)*2-1;
        curr.lChild = &nodes[nth_node];
        curr.rChild = &nodes[nth_node+lNodes];
        if(end-begin<coarsening[1]){
            BIN_Build(*curr.lChild, entries, begin, en_index, nth_node+1);
            BIN_Build(*curr.rChild, entries, en_index, end, nth_node+1+lNodes);
        }
        else{
            cilk_spawn
            BIN_Build(*curr.lChild, entries, begin, en_index, nth_node+1);
            BIN_Build(*curr.rChild, entries, en_index, end, nth_node+1+lNodes);
            cilk_sync;
        }
    }
}


bool SAH_BIN::findLongestDim(int& dimension, double& largest_dist, double& lo_dist,
    const std::vector<Entry>& entries, int begin, int end) //begin, end
{
    vec3 min_d, max_d;
    min_d.fill(std::numeric_limits<double>::infinity());
    max_d.fill(-std::numeric_limits<double>::infinity());
    
    for(int i=begin; i<end; i++){
        for(int j=0; j<3; j++){
            // center at dimension j as position
            min_d[j] = std::min(min_d[j], entries[i].box.center[j]);
            max_d[j] = std::max(max_d[j], entries[i].box.center[j]);
        }
    }
    
    // update return value
    bool found = false;
    for(int i=0; i<3; i++){
        if(max_d[i]-min_d[i]>largest_dist){
            largest_dist = max_d[i] - min_d[i];
            lo_dist = min_d[i];
            dimension = i;
            found = true;
        }
    }
    return found;
}

void SAH_BIN::bucketing(std::vector<Entry>& entries, int begin, int end, double left, double right, 
    double split_th, int dimension, std::vector<Box>& buckets, std::vector<int>& BucketToEntry, int nth_bucket)
{
    if(right-left<=split_th*1.5){ // avoid floating point error
        BucketToEntry[nth_bucket] = begin;
        Box& currBox = buckets[nth_bucket];
        for(int i=begin; i<end; i++) currBox = currBox.Union(entries[i].box);
        return;
    }
    double split = (left+right)/2.0;
    int i = begin, j = begin==end? end: end-1; // in case begin > end
    while(i<j){
        while(i<j && entries[i].box.center[dimension]<split) i++;
        while(i<j && entries[j].box.center[dimension]>=split) j--;
        std::swap(entries[i], entries[j]);
    }
    if(end-begin<coarsening[1]){
        bucketing(entries, begin, j, left, split, split_th, dimension, buckets, 
            BucketToEntry, 2*nth_bucket);
        bucketing(entries, j, end, split, right, split_th, dimension, buckets, 
            BucketToEntry, 2*nth_bucket+1);
    }
    else{
        cilk_spawn
        bucketing(entries, begin, j, left, split, split_th, dimension, buckets, 
            BucketToEntry, 2*nth_bucket);
        bucketing(entries, j, end, split, right, split_th, dimension, buckets, 
            BucketToEntry, 2*nth_bucket+1);
        cilk_sync;
    }
}

void SAH_BIN::bucketing(const std::vector<Entry>& entries, int begin, int end, int dimension,
    double largest_dist, double lo_dist, std::vector<Box>& buckets, std::vector<int>& BucketToEntry)
{
    int n = buckets.size();
    double interval = largest_dist / n;
    // partitions smallest elements | | | ... | largest elements
    std::vector<double> partitions(n-1);

    // calculate partitions
    double t = interval;
    for(auto& p: partitions){
        p = lo_dist + t;
        t += interval;
    }

    // allocate entries into buckets
    int i=begin;
    for(size_t j=0; j<partitions.size();){
        double position = entries[i].box.center[dimension];
        if(position<=partitions[j]){
            buckets[j] = buckets[j].Union(entries[i].box);
            i++;
        }
        else BucketToEntry[++j] = i;
    }
    BucketToEntry[partitions.size()] = i;
    // entries larger than the last partition
    for(; i<end; i++){
        buckets.back() = buckets.back().Union(entries[i].box);
    }
}


// return entries index for the right half of partition
int SAH_BIN::findBestPartition(const std::vector<Box>& buckets, const std::vector<int>& BucketToEntry, Box& currBox)
{
    int en_index = -1; // entry_index
    int n = buckets.size(); // buckets.size() boxes
    //std::cout<<"findBestPartition"<<std::endl;
    std::vector<Box> sweepR(n);
    sweepR.back() = buckets.back();
    Box sweepL = buckets.front();

    for(int i=n-2; i>=0; i--) sweepR[i] = buckets[i].Union(sweepR[i+1]);
    
    double min_cost = std::numeric_limits<double>::infinity();
    for(int i=0; i<n-1; i++){
        double cost = sweepL.Surface_Area()*(i+1) + sweepR[i+1].Surface_Area()*(n-i-1);
        if(cost<min_cost){
            min_cost = cost;
            en_index = BucketToEntry[i+1];
            //std::cout<<"i:"<<i<<std::endl;
            currBox = sweepL.Union(sweepR[i+1]);
        }
        sweepL = sweepL.Union(buckets[i+1]);
    }

    return en_index;
}


// candidates: pointer of entries. Level order traversal
void SAH_BIN::Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) const
{
    //std::cout<<"candidata()"<<std::endl;
    if(!root) return;

    std::queue<Node*> q;
    if(root->box.Intersection(ray)){
        q.push(root);
    }
    
    while(!q.empty()){
        Node* temp = q.front(); q.pop();
        if(temp->begin>=0){
            for(int i=temp->begin; i<temp->end; i++){
                candidates.push_back(i);
            }
            continue;
        }
        if(temp->lChild && temp->lChild->box.Intersection(ray)){
            q.push(temp->lChild);
        }
        if(temp->rChild && temp->rChild->box.Intersection(ray)){
            q.push(temp->rChild);
        }
    }
}
