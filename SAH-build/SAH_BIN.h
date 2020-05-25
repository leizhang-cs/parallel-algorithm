#ifndef __SAH_BIN_H__
#define __SAH_BIN_H__

#include "object.h"

struct Entry;

// TODO new node
struct Node{
    Box box;
    Node* lChild;
    Node* rChild;
    std::vector<Entry*> entry_list;
    
    Node():lChild(nullptr),rChild(nullptr){}
    Node(int th):lChild(nullptr),rChild(nullptr),entry_list(th){}
};

class SAH_BIN{
public:
    SAH_BIN():threshold(0),buckets_num(0){}
    // SAH_BIN(threshold, buckets_num) eg. 2, 16
    SAH_BIN(int threshold_input, int buckets_num_input)
        :threshold(threshold_input),buckets_num(buckets_num_input),centers(entries.size())
    {}

    const int threshold; // for leaf
    const int buckets_num;
    std::vector<Entry> entries; // including primitives, import from hierarchy
    
    void Build(std::vector<Entry>& entries);
    // candidates: pointer of entries
    void Intersection_Candidates(const Ray& ray, std::vector<Entry*>& candidates) const;

private:
    Node* root; // root of BVH
    std::vector<vec3> centers; // 2*center for entries.box

    
    void BIN_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end);
    void findLongestDim(int& dimension, double& largest_dist, double& lo_dist, 
        const std::vector<vec3>& centers);
    void bucketing(int dimension, double largest_dist, double lo_dist, std::vector<Box>& buckets, 
        std::vector<int>& BucketToEntry);
    int findBestPartition(const std::vector<Box>& buckets, const std::vector<int>& BucketToEntry, 
        Box& currBox);
};


#endif