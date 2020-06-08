#ifndef __SAH_SWEEP_H__
#define __SAH_SWEEP_H__

#include "object.h"
#include "hierarchy.h"
#include "common.h"

struct Entry;

// BVH treeNode
struct Node;

class SAH_Sweep: public Hierarchy
{
public:
    SAH_Sweep(){}
    SAH_Sweep(int threshold_input): Hierarchy(threshold_input),node_index(-1){}

    // Hierarchy::threshold;
    // Hierachy::entries;
    virtual void Build(std::vector<Entry>& entries) override;
    // candidates: pointer of entries
    virtual void Intersection_Candidates(const Ray& ray, std::vector<int>& candidates)
        const override;

private:
    Node* root; // root of BVH
    std::atomic_int node_index;
    std::vector<Node> nodes; // nodes of BVH, nodes[0] is the root
    
    void Sweep_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end);
    bool updateBestPartition(int& global_index, double& global_min, Box& currBox,
        std::vector<Entry>& entries, int begin, int end);
    void Make_Leaf(Node*& curr, std::vector<Entry>& entries, int begin, int end);
};

inline
void SAH_Sweep::Make_Leaf(Node*& curr, std::vector<Entry>& entries, int begin, int end){
    curr->begin = begin;
    curr->end = end;
    curr->box = entries[begin].box;
    for(int i=begin+1; i<end; i++){
        curr->box = curr->box.Union(entries[i].box);
    }
}

#endif