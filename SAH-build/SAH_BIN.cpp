#include "SAH_BIN.h"
#include <algorithm>
#include <limits>
#include <queue>
#include "inline_func.cpp"


struct Entry
{
    Object* obj;
    int part;
    Box box;
};

struct Node{
    Box box;
    Node* lChild;
    Node* rChild;
    std::vector<Entry*> entry_list;
    
    Node():lChild(nullptr),rChild(nullptr){}
    Node(int n):lChild(nullptr),rChild(nullptr),entry_list(n){}
};

void SAH_BIN::Build(std::vector<Entry>& entries){
    // TODO cilk_for
    BIN_Build(root, entries, 0, entries.size());
}


void SAH_BIN::BIN_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end)
{
    int n = end - begin;
    if(n<=threshold){
        curr = new Node(n);
        curr->box.Make_Empty();
        for(int i=0; i<n; i++){
            curr->box = curr->box.Union(entries[begin+i].box);
            curr->entry_list[i] = &entries[begin+i];
        }
    }
    else{
        curr = new Node();
        // find longest dimension
        int dimension = -1;
        double largest_dist = 0, lo_dist;
        findLongestDim(dimension, largest_dist, lo_dist, entries, begin ,end);
        if(dimension==-1){ std::cout<<"find dim failed"<<std::endl; exit(EXIT_FAILURE); }

        // buckets: bounding boxes
        std::vector<Box> buckets(buckets_num);
        // map: buckets index -> entries index
        std::vector<int> BucketToEntry(buckets_num);
        for(auto& b: buckets) b.Make_Empty();

        // TODO bucket sorting
        // compare two Entry along a-dimension
        std::sort(entries.begin()+begin, entries.begin()+end, 
            [dimension](const auto& e1, const auto& e2){
                return (e1.box.lo+e1.box.hi)[dimension]<(e2.box.lo+e2.box.hi)[dimension];
            }
        );
        
        // bucketing
        bucketing(dimension, largest_dist, lo_dist, buckets, BucketToEntry, entries, begin, end);

        // find best partition, right half entries' index
        int en_index = findBestPartition(buckets, BucketToEntry, curr->box);
        if(en_index==-1){ std::cout<<"partition failed"<<std::endl; exit(EXIT_FAILURE); }
        //std::cout<<"partition:"<<begin<<" "<<en_index<<" "<<end<<std::endl;
        
        BIN_Build(curr->lChild, entries, begin, en_index);
        BIN_Build(curr->rChild, entries, en_index, end);
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
            // TODO box center
            double position = entries[i].box.lo[j]+entries[i].box.hi[j];
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


void SAH_BIN::bucketing(int dimension, double largest_dist, double lo_dist, std::vector<Box>& buckets,
    std::vector<int>& BucketToEntry, const std::vector<Entry>& entries, int begin, int end)
{
    int n = buckets_num;
    double interval = largest_dist / n;
    // partitions smallest elements | | | ... | largest elements
    std::vector<double> partitions(n-1);

    // calculate partitions
    double t = interval;
    for(auto& p: partitions){
        p = lo_dist + t;
        t += interval;
    }

    // debug
    // allocate entries into buckets
    int i=begin;
    for(size_t j=0; j<partitions.size();){
        double position = entries[i].box.lo[dimension]+entries[i].box.hi[dimension];
        
        if(position<=partitions[j]){
            buckets[j] = buckets[j].Union(entries[i].box);
            i++;
        }
        else{
            BucketToEntry[++j] = i;
        }
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
    int n = buckets_num; // buckets.size() boxes
    //std::cout<<"findBestPartition"<<std::endl;
    std::vector<Box> sweepR(n), sweepL(n);
    sweepR.back() = buckets.back();
    sweepL.front() = buckets.front();

    // TODO merge
    for(int i=n-2; i>=0; i--){
        sweepR[i] = buckets[i].Union(sweepR[i+1]);
    }
    for(int i=1; i<n; i++){
        sweepL[i] = buckets[i].Union(sweepL[i-1]);
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


// TODO: base class
// candidates: pointer of entries
void SAH_BIN::Intersection_Candidates(const Ray& ray, std::vector<Entry*>& candidates) const
{
    //std::cout<<"candidata()"<<std::endl;
    if(!root) return;

    std::queue<Node*> q;
    if(root->box.Intersection(ray)){
        q.push(root);
        if(debug_pixel) std::cout<<"enter loop"<<std::endl;
    }
    

    while(!q.empty()){
        Node* temp = q.front(); q.pop();
        if(debug_pixel) std::cout<<"box:"<<temp->box.Surface_Area()<<std::endl;
        if(!temp->entry_list.empty()){
            //std::cout<<"intersect: "<<std::endl;
            for(auto en: temp->entry_list){
                candidates.push_back(en);
            }
        }
        if(temp->lChild && temp->lChild->box.Intersection(ray)){
            q.push(temp->lChild);
        }
        if(temp->rChild && temp->rChild->box.Intersection(ray)){
            q.push(temp->rChild);
        }
    }
}