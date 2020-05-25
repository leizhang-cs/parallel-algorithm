#ifndef __SAH_BUILD_H__
#define __SAH_BUILD_H__

#include "object.h"

struct Entry;

struct Node{
    Box box;
    std::vector<Entry*> entry_list;
    Node* lChild;
    Node* rChild;
    
    Node():lChild(nullptr), rChild(nullptr){}
};

class SAH_Build{
public:
    SAH_Build(){}
    SAH_Build(int threshold_input): threshold(threshold_input){}

    int threshold; // for leaf
    std::vector<Entry> entries; // including primitives, import from hierarchy

    void Build(std::vector<Entry>& entries);
    // candidates: pointer of entries
    void Intersection_Candidates(const Ray& ray, std::vector<Entry*>& candidates) const;

private:
    Node* root; // root of BVH
    
    void Sweep_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end);
    bool updateBestCut(int& global_index, double& global_min, Box& currBox, std::vector<Entry>& entries,
        int begin, int end);
};


#endif