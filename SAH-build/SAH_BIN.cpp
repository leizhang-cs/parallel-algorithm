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

// compare two Entry along a-dimension
struct compare{
    compare(int a):dimension(a){}
    int dimension;
    bool operator()(Entry e1, Entry e2){
        // center = (e.box.lo+e.box.hi)/2;
        vec3 c1 = e1.box.lo+e1.box.hi, c2 = e2.box.lo+e2.box.hi;
        return c1[dimension]<c2[dimension];
    }
};


void SAH_BIN::Build(std::vector<Entry>& entries){
    // TODO cilk_for
    // initialization
    for(size_t i=0; i<centers.size(); i++){
        centers[i] = entries[i].box.lo+entries[i].box.hi;
    }
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
        findLongestDim(dimension, largest_dist, lo_dist, centers);
        if(dimension==-1){ std::cout<<"fail update"<<std::endl; return; }

        // buckets: bounding boxes
        std::vector<Box> buckets(buckets_num);
        // map: buckets index -> entries index
        std::vector<int> BucketToEntry;
        for(auto& b: buckets) b.Make_Empty();

        // TODO bucket sorting
        //compare comp(dimension);
        std::sort(entries.begin()+begin, entries.begin()+end, compare(dimension));
        // bucketing
        bucketing(dimension, largest_dist, lo_dist, buckets, BucketToEntry);

        // find best partition, right half entries' index
        int en_index = findBestPartition(buckets, BucketToEntry, curr->box);
        //std::cout<<"partition:"<<en_index<<std::endl;
        
        BIN_Build(curr->lChild, entries, begin, begin+en_index);
        BIN_Build(curr->rChild, entries, begin+en_index, end);
    }
}


void SAH_BIN::findLongestDim(int& dimension, double& largest_dist, double& lo_dist,
    const std::vector<vec3>& centers)//begin, end
{
    vec3 min_d, max_d;
    min_d.fill(std::numeric_limits<double>::infinity());
    max_d.fill(-std::numeric_limits<double>::infinity());
    
    for(size_t i=0; i<centers.size(); i++){
        for(int j=0; j<3; j++){
            min_d[j] = std::min(min_d[j], centers[i][j]);
            max_d[j] = std::max(max_d[j], centers[i][j]);
        }
    }
    for(int i=0; i<3; i++){
        if(max_d[i]-min_d[i]>largest_dist){
            largest_dist = max_d[i] - min_d[i];
            dimension = i;
        }
    }
}


void SAH_BIN::bucketing(int dimension, double largest_dist, double lo_dist,
    std::vector<Box>& buckets, std::vector<int>& BucketToEntry)
{
    int n = buckets.size();
    double interval = largest_dist / n;
    // partitions smallest elements | | | ... | largest elements
    std::vector<double> partitions(n-1);

    // calculate partitions
    double t = interval;
    for(int i=0; i<n; i++,t+=interval){
        partitions[i] = lo_dist + t;
    }

    // debug
    // allocate entries into buckets
    size_t i=0;
    for(size_t j=0; j<partitions.size();){
        if(centers[i][dimension]<=partitions[j]){
            buckets[j] = buckets[j].Union(entries[i].box);
            i++;
        }
        else{
            BucketToEntry[++j] = i;
        }
    }
    BucketToEntry[partitions.size()] = i;
    // entries larger than the last partition
    for(; i<entries.size(); i++){
        buckets.back() = buckets.back().Union(entries[i].box);
    }
}


// return entries index for the right half of partition
int SAH_BIN::findBestPartition(const std::vector<Box>& buckets, const std::vector<int>& BucketToEntry, 
    Box& currBox)
{
    int en_index = -1; // entry_index
    int n = buckets.size(); // buckets.size() boxes
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
        //std::cout<<cost<<" "<<min_cost<<std::endl;
    }

    return en_index;
}


// TODO: base class
// candidates: pointer of entries
void SAH_BIN::Intersection_Candidates(const Ray& ray, std::vector<Entry*>& candidates) const
{
    //std::cout<<"in: "<<std::endl;
    //std::cout<<"candi"<<std::endl;
    if(!root) return;

    if(debug_pixel){
        std::cout<<"root box:"<<root->box.lo<<" "<<root->box.hi<<std::endl;
        if(root->box.Intersection(ray)){ std::cout<<"sah"<<std::endl; }
        else{ std::cout<<"sah not intersect"<<std::endl; }
    }

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
    //std::cout<<"out: "<<std::endl;
}