#ifndef __INCREMENTAL_BUILD_H__
#define __INCREMENTAL_BUILD_H__

#include "object.h"
#include "hierarchy.h"

struct Entry;

class Incremental_Build: public Hierarchy
{
public:
    virtual void Build(std::vector<Entry>& entries) override;
    // Return a list of candidates whose bounding boxes intersect the ray.
    virtual void Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) 
        const override;

private:
    // Flattened hierarchy
    std::vector<Box> tree;

    // Reorder the entries vector so that adjacent entries tend to be nearby.
    void Reorder_Entries(std::vector<Entry>& entries);
    // Populate tree from entries.
    void Build_Tree(std::vector<Entry>& entries);
};
#endif
