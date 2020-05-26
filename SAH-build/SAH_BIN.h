#ifndef __SAH_BIN_H__
#define __SAH_BIN_H__

#include "object.h"
#include "hierarchy.h"

struct Entry;

// BVH treeNode
struct Node;

class SAH_BIN: public Hierarchy
{
public:
    SAH_BIN():buckets_num(0){}
    // threshold of treeNode, buckets number for partition
    SAH_BIN(int threshold_input, int buckets_num_input)
        :Hierarchy(threshold_input),buckets_num(buckets_num_input)
    {}

    const int buckets_num;
    //Hierarchy::entries;
    
    virtual void Build(std::vector<Entry>& entries) override;
    // candidates: pointer of entries
    virtual void Intersection_Candidates(const Ray& ray, std::vector<const Entry*>& candidates)
        const override;

private:
    Node* root; // root of BVH

    void BIN_Build(Node*& curr, std::vector<Entry>& entries, int begin, int end);
    void findLongestDim(int& dimension, double& largest_dist, double& lo_dist, 
        const std::vector<Entry>& entries, int begin, int end);
    void bucketing(int dimension, double largest_dist, double lo_dist, std::vector<Box>& buckets, 
        std::vector<int>& BucketToEntry, const std::vector<Entry>& entries, int begin, int end);
    int findBestPartition(const std::vector<Box>& buckets, const std::vector<int>& BucketToEntry, 
        Box& currBox);
    void Make_Leaf(Node*& curr, std::vector<Entry>& entries, int begin, int end);
};


#endif