#ifndef __SAH_BIN_H__
#define __SAH_BIN_H__

#include "object.h"
#include "hierarchy.h"
#include "common.h"

struct Entry; // primitives

struct Node; // BVH treeNode

class SAH_BIN: public Hierarchy
{
public:
    SAH_BIN():buckets_num(0){}
    // threshold of treeNode, buckets number for partition
    SAH_BIN(int threshold_input, int buckets_num_input)
        :Hierarchy(threshold_input),buckets_num(buckets_num_input),node_index(-1)
    {}

    const int buckets_num;
    //Hierarchy::entries;
    
    virtual void Build(std::vector<Entry>& entries) override;
    // candidates: pointer of entries
    virtual void Intersection_Candidates(const Ray& ray, std::vector<int>& candidates)
        const override;

private:
    Node* root;
    std::atomic_int node_index;
    std::vector<Node> nodes; // nodes of BVH, nodes[0] is the root
    std::vector<Node> topNodes;
    std::vector<int> coarsening;
    bool sorting_scheme = false; // sorting or partition

    void BIN_Build(Node& curr, std::vector<Entry>& entries, int begin, int end, int nth_node);
    bool findLongestDim(int& dimension, double& largest_dist, double& lo_dist, 
        const std::vector<Entry>& entries, int begin, int end);
    void bucketing(std::vector<Entry>& entries, int begin, int end, double left, double right, 
        double split_th, int dimension, std::vector<Box>& buckets, std::vector<int>& BucketToEntry,
        int nth_bucket);
    void bucketing(const std::vector<Entry>& entries, int begin, int end, int dimension,
        double largest_dist, double lo_dist, std::vector<Box>& buckets, std::vector<int>& BucketToEntry);
    int findBestPartition(const std::vector<Box>& buckets, const std::vector<int>& BucketToEntry, 
        Box& currBox);
    void Make_Leaf(Node& curr, std::vector<Entry>& entries, int begin, int end);
};

inline
void SAH_BIN::Make_Leaf(Node& curr, std::vector<Entry>& entries, int begin, int end){
    curr.begin = begin;
    curr.end = end;
    curr.box = entries[begin].box;
    for(int i=begin+1; i<end; i++){
        curr.box = curr.box.Union(entries[i].box);
    }
}

#endif
