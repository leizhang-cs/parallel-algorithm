#include "SAH_BIN.h"
#include "inline_func.cpp"
#include "../samplesort/samplesort.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>


void SAH_BIN::Build(std::vector<Entry>& entries){
    int n = entries.size();
    constrains.push_back(std::max(n/8, static_cast<int>(log(n)*sqrt(n))));
    constrains.push_back(std::min(constrains[0]/8, static_cast<int>(log(n)*sqrt(n))));
    nodes.resize(2*n-1);
    root = &nodes[++node_index];
    BIN_Build(root, entries, 0, n);
    nodes.resize(node_index);
}

// binned build
void SAH_BIN::BIN_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end)
{
    if(end-begin<=threshold){
        Make_Leaf(curr, entries, begin, end);
    }
    else{
        int local_bucket_num = buckets_num; //end-begin<constrains[0]? buckets_num/2: buckets_num;
        if(begin-end<constrains[0]){
            local_bucket_num = end-begin<constrains[1]? 4: std::max(buckets_num/2,4);
        }
        // find longest dimension
        int dimension = -1;
        double largest_dist = 0, lo_dist;
        findLongestDim(dimension, largest_dist, lo_dist, entries, begin ,end);
        if(dimension==-1){
            if(safe_mode){ dimension=0; }
            else{ std::cout<<"finding dim failed"<<std::endl; exit(EXIT_FAILURE); }
        }

        // buckets: bounding boxes
        std::vector<Box> buckets(local_bucket_num);
        // map: buckets index -> entries index
        std::vector<int> BucketToEntry(local_bucket_num);
        for(size_t i=0; i<buckets.size(); i++) buckets[i].Make_Empty();

        // Method 1: sort and bucket 
        if(sorting_method){
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
            bucketing(dimension, largest_dist, lo_dist, buckets, BucketToEntry, entries, begin, end);
        }
        else{ // Method 2: bucketing with partition
            bucketing(entries, begin, end, lo_dist, lo_dist+largest_dist, largest_dist/local_bucket_num, \
                dimension, buckets, BucketToEntry, 0);
        }
	
        // find best partition, right half entries' index
        int en_index = findBestPartition(buckets, BucketToEntry, curr->box);
        if(en_index==-1){ // when failing to find partition, make leaf or exit(1).
            if(safe_mode){ Make_Leaf(curr,entries,begin,end); return; }
            else{ std::cout<<"partitioning failed"<<std::endl; exit(EXIT_FAILURE); }
        }
        //std::cout<<"partition:"<<begin<<" "<<en_index<<" "<<end<<std::endl;
        curr->lChild = &nodes[++node_index];
        curr->rChild = &nodes[++node_index];
        cilk_spawn
        BIN_Build(curr->lChild, entries, begin, en_index);
        BIN_Build(curr->rChild, entries, en_index, end);
        cilk_sync;
    }
}


void SAH_BIN::findLongestDim(int& dimension, double& largest_dist, double& lo_dist,
    const std::vector<Entry>& entries, int begin, int end)//begin, end
{
    vec3 min_d, max_d;
    min_d.fill(std::numeric_limits<double>::infinity());
    max_d.fill(-std::numeric_limits<double>::infinity());
    
    for(int i=begin; i<end; i++){
        for(int j=0; j<3; j++){
            // center at dimension j as position
            double position = entries[i].box.Center()[j];
            min_d[j] = std::min(min_d[j], position);
            max_d[j] = std::max(max_d[j], position);
        }
    }
    
    // update return value
    for(int i=0; i<3; i++){
        if(max_d[i]-min_d[i]>largest_dist){
            largest_dist = max_d[i] - min_d[i];
            lo_dist = min_d[i];
            dimension = i;
        }
    }
}

void SAH_BIN::bucketing(std::vector<Entry>& entries, int begin, int end, double left, double right, 
    double split_th, int dimension, std::vector<Box>& buckets, std::vector<int>& BucketToEntry,
    int nth_bucket)
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
        while(i<j && entries[i].box.Center()[dimension]<split) i++;
        while(i<j && entries[j].box.Center()[dimension]>=split) j--;
        std::swap(entries[i], entries[j]);
    }
    cilk_spawn
    bucketing(entries, begin, j, left, split, split_th, dimension, buckets, 
        BucketToEntry, 2*nth_bucket);
    bucketing(entries, j, end, split, right, split_th, dimension, buckets, 
        BucketToEntry, 2*nth_bucket+1);
    cilk_sync;
}

void SAH_BIN::bucketing(int dimension, double largest_dist, double lo_dist, std::vector<Box>& buckets,
    std::vector<int>& BucketToEntry, const std::vector<Entry>& entries, int begin, int end)
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
        double position = entries[i].box.Center()[dimension];
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
int SAH_BIN::findBestPartition(const std::vector<Box>& buckets, const std::vector<int>& BucketToEntry, 
    Box& currBox)
{
    int en_index = -1; // entry_index
    int n = buckets.size(); // buckets.size() boxes
    //std::cout<<"findBestPartition"<<std::endl;
    std::vector<Box> sweepR(n), sweepL(n);
    sweepR.back() = buckets.back();
    sweepL.front() = buckets.front();

    for(int i=n-2; i>=0; i--){
        sweepR[i] = buckets[i].Union(sweepR[i+1]);
        sweepL[n-i-1] = buckets[n-i-1].Union(sweepL[n-i-2]);
    }

    double min_cost = std::numeric_limits<double>::infinity();
    for(int i=0; i<n-1; i++){
        double cost = sweepL[i].Surface_Area()*(i+1) + sweepR[i+1].Surface_Area()*(n-i-1);
        if(cost<min_cost){
            min_cost = cost;
            en_index = BucketToEntry[i+1];
            currBox = sweepL[i].Union(sweepR[i+1]);
        }
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
