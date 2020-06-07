#include "bottomup_build.h"
#include "inline_func.cpp"
#include <algorithm>
#include <queue>

static double diff_t = 1e-2;

// compare two Entry
struct local_compare{
    bool operator()(Entry& e1, Entry& e2){
        double diff = e1.box.lo[0] - e2.box.lo[0];
        if(std::abs(diff)>diff_t) return diff<0;
        else{
            diff = e1.box.lo[1] - e2.box.lo[1];
            if(std::abs(diff)>diff_t) return diff<0;
            else{
                return e1.box.lo[2]<e2.box.lo[2];
            }
        }
    }
};


void BottomUp_Build::Build(std::vector<Entry>& entries){
    Reorder_Entries(entries);
    Build_Tree(entries);
    //std::cout<<"h build done"<<std::endl;
}

// Return a list of candidates whose bounding boxes intersect the ray.
void BottomUp_Build::
Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) const
{
    if(!tree.size()) return;
    int i, k, size = tree.size(), size_en = entries.size();
    std::queue<int> q;
    if(tree[0].Intersection(ray)) q.push(0);
    while(!q.empty()){
        i = q.front();
        q.pop();
        k = 2 * i + 1;
        if(k<size){
            if(tree[k].Intersection(ray))
                q.push(k);
            k++;
            if(k<size && tree[k].Intersection(ray))
                q.push(k);
        }
        else{
            candidates.push_back(i-size_en+1);
        }
    }
}

// Reorder the entries vector so that adjacent entries tend to be nearby.
// You may want to implement box.cpp first.
void BottomUp_Build::Reorder_Entries(std::vector<Entry>& entries)
{
    if(!entries.size()) return;
    std::sort(entries.begin(), entries.end(), local_compare());
}

// Populate tree from entries.
void BottomUp_Build::Build_Tree(std::vector<Entry>& entries)
{
    if(!entries.size()) return;
    int i = 0, j, size_en = entries.size(), size = 2*size_en-1;
    tree.resize(size);
    //std::cout<<size<<std::endl;
    for(i=0, j=size-size_en; i<size_en; i++, j++){
        tree[j] = entries[i].box;
    }
    for(i=size-1; i>0; i-=2){
        Box b = tree[i].Union(tree[i-1]);
        tree[i/2-1] = b;
    }
    //std::cout<<tree.size()<<std::endl;
}
