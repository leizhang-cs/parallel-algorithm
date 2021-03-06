#include "SAH_sweep.h"
#include "inline_func.cpp"


void SAH_Sweep::Build(std::vector<Entry>& entries){
    std::vector<Node> _nodes(entries.size()*2);
    std::swap(_nodes, nodes);
    root = &nodes[++node_index];
    Sweep_Build(root, entries, 0, entries.size());
}

void SAH_Sweep::Sweep_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end)
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
        for(int i=0; i<3; i++){
            //std::cout<<"axis:"<<i<<std::endl;
            // compare along i-axis
            // std::sort(entries.begin()+begin, entries.begin()+end, [i](auto& e1, auto& e2){
            //     vec3 c1 = (e1.box.lo+e1.box.hi)/2, c2 = (e2.box.lo+e2.box.hi)/2;
            //     return c1[i]<c2[i];
            // });
            std::sort(entries.begin()+begin, entries.begin()+end, compare(i));
            // search and update bestCut, current node box
            if(updateBestPartition(global_index, global_min, curr->box, entries, 
                begin, end)){
                axis = i;
            }
        }
        if(axis==-1){ std::cout<<"fail update"<<std::endl; return; }
        else if(axis!=2){
            // std::sort(entries.begin()+begin, entries.begin()+end, [axis](auto& e1, auto& e2){
            //     vec3 c1 = (e1.box.lo+e1.box.hi)/2, c2 = (e2.box.lo+e2.box.hi)/2;
            //     return c1[axis]<c2[axis];
            // });
            std::sort(entries.begin()+begin, entries.begin()+end, compare(axis));
        }

        //std::cout<<"split:"<<global_index<<std::endl;
        curr->lChild = &nodes[++node_index];
        curr->rChild = &nodes[++node_index];
        Sweep_Build(curr->lChild, entries, begin, begin+global_index);
        Sweep_Build(curr->rChild, entries, begin+global_index, end);
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
        if(temp->begin!=-1){
            //std::cout<<"intersect: "<<std::endl;
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
