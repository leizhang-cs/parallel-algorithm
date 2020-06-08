#include "SAH_sweep.h"
#include "inline_func.cpp"
#include "../samplesort/samplesort.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>


void SAH_Sweep::Build(std::vector<Entry>& entries){
    nodes.resize(entries.size()*2-1);
    root = &nodes[0];
    Sweep_Build(root, entries, 0, entries.size(), 0 ,nodes.size());
}

void SAH_Sweep::Sweep_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end,
    int node_begin, int node_end)
{
    int n = end - begin;
    if(n<=threshold){
        Make_Leaf(curr, entries, begin, end);
    }
    else{
        // bestCut index and axis
        int global_index = -1, axis = -1;
        double global_min = std::numeric_limits<double>::infinity();
        Box boxL, boxR;
        // find best partition on three dimensions
        std::vector<Entry> dummy_entries(entries.begin()+begin, entries.begin()+end);
        for(int i=0; i<3; i++){
            if(end-begin<1e4){ // stl::sort
                std::sort(entries.begin()+begin, entries.begin()+end, compare(i));
            }
            else{ //samplesort
                Samplesort<Entry> ssort(compare(i), begin);
                ssort.sort(dummy_entries, entries);
            }
            // search and update bestCut, current node box
            if(updateBestPartition(global_index, global_min, curr->box, entries, 
                begin, end)){
                axis = i;
            }
        }
        if(axis==-1){ std::cout<<"fail update"<<std::endl; return; }
        else if(axis!=2){
            if(end-begin<1e4){ // stl::sort
                std::sort(entries.begin()+begin, entries.begin()+end, compare(axis));
            }
            else{ //samplesort
                Samplesort<Entry> ssort(compare(axis), begin);
                ssort.sort(dummy_entries, entries);
            }
        }

        int lNodes = (global_index)*2-1;
        curr->lChild = &nodes[node_begin+1];
        curr->rChild = &nodes[node_begin+1+lNodes];
        cilk_spawn
        Sweep_Build(curr->lChild, entries, begin, begin+global_index, node_begin+1, node_begin+1+lNodes);
        Sweep_Build(curr->rChild, entries, begin+global_index, end, node_begin+1+lNodes, node_end);
        cilk_sync;
    }
}

bool SAH_Sweep::updateBestPartition(int& global_index, double& global_min, Box& currBox, 
        std::vector<Entry>& entries, int begin, int end){
    bool update = false;
    int n = end - begin;
    std::vector<Box> sweepR(n), sweepL(n);
    sweepR[n-1] = entries[begin+n-1].box;
    sweepL[0] = entries[begin+0].box;

    for(int i=n-2; i>=0; i--){
        sweepR[i] = entries[begin+i].box.Union(sweepR[i+1]);
    }
    for(int i=1; i<n; i++){
        sweepL[i] = entries[begin+i].box.Union(sweepL[i-1]);
    }

    for(int i=0; i<n-1; i++){
        double cost = sweepL[i].Surface_Area()*(i+1) + sweepR[i+1].Surface_Area()*(n-i-1);
        if(cost<global_min){
            global_min = cost;
            global_index = i+1;
            currBox = sweepL[i].Union(sweepR[i+1]);
            update = true;
        }
        //std::cout<<cost<<" "<<global_min<<std::endl;
    }

    return update;
}


// candidates: pointer of entries. Level order traversal
void SAH_Sweep::Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) const
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
