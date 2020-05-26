#include "SAH_build.h"
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

// compare two Entry along a-axis
struct compare{
    compare(int a):axis(a){}
    int axis;
    bool operator()(Entry e1, Entry e2){
        vec3 c1 = (e1.box.lo+e1.box.hi)/2, c2 = (e2.box.lo+e2.box.hi)/2;
        return c1[axis]<c2[axis];
    }
};

void SAH_Build::Build(std::vector<Entry>& entries){
    Sweep_Build(root, entries, 0, entries.size());
}

void SAH_Build::Sweep_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end)
{
    int n = end - begin;
    if(n<=threshold){
        curr = new Node(n);
        Box b; b.Make_Empty();
        for(int i=0; i<n; i++){
            b = b.Union(entries[begin+i].box);
            curr->entry_list[i] = &entries[begin+i];
        }
        curr->box = b;
    }
    else{
        curr = new Node();
        // bestCut index and axis
        int global_index = -1, axis = -1;
        double global_min = std::numeric_limits<double>::infinity();
        Box boxL, boxR;
        for(int i=0; i<3; i++){
            //std::cout<<"axis:"<<i<<std::endl;
            // compare along i-axis
            compare comp(i);
            std::sort(entries.begin()+begin, entries.begin()+end, comp);
            // search and update bestCut, current node box
            if(updateBestCut(global_index, global_min, curr->box, entries, 
                begin, end)){
                axis = i;
            }
        }
        if(axis==-1){ std::cout<<"fail update"<<std::endl; return; }
        else if(axis!=2){
            compare comp(axis);
            std::sort(entries.begin()+begin, entries.begin()+end, comp);
        }

        //std::cout<<"split:"<<global_index<<std::endl;
        
        Sweep_Build(curr->lChild, entries, begin, begin+global_index);
        Sweep_Build(curr->rChild, entries, begin+global_index, end);
    }
}

bool SAH_Build::updateBestCut(int& global_index, double& global_min, Box& currBox, 
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


// candidates: pointer of entries
void SAH_Build::Intersection_Candidates(const Ray& ray, std::vector<Entry*>& candidates) const
{
    //std::cout<<"candidata()"<<std::endl;
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