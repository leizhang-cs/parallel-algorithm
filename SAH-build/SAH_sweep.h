#ifndef __SAH_SWEEP_H__
#define __SAH_SWEEP_H__

#include "object.h"
#include "hierarchy.h"

struct Entry;

// BVH treeNode
struct Node;

class SAH_Sweep: public Hierarchy
{
public:
    SAH_Sweep(){}
    SAH_Sweep(int threshold_input): Hierarchy(threshold_input){}

    // Hierarchy::threshold;
    // Hierachy::entries;
    virtual void Build(std::vector<Entry>& entries) override;
    // candidates: pointer of entries
    virtual void Intersection_Candidates(const Ray& ray, std::vector<int>& candidates)
        const override;

private:
    Node* root; // root of BVH
    
    void Sweep_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end);
    bool updateBestPartition(int& global_index, double& global_min, Box& currBox,
        std::vector<Entry>& entries, int begin, int end);
};


#endif