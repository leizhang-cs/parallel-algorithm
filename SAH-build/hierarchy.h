#ifndef __HIERARCHY_H__
#define __HIERARCHY_H__

#include "object.h"


struct Entry
{
    Object* obj;
    int part;
    Box box;
};

struct Node;

class Hierarchy
{
public:
    Hierarchy():threshold(0){}
    Hierarchy(int threshold_input):threshold(threshold_input){}

    // Numbers of primitives per TreeNode
    const int threshold;
    // List of primitives (or parts of primitives) that can be intersected
    std::vector<Entry> entries;
    std::vector<Entry> entries_rfable;
    std::vector<Entry> entries_inf;

    virtual void Build(std::vector<Entry>& entries)=0;
    // Return a list of candidates whose bounding boxes intersect the ray.
    virtual void Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) const=0;
};
#endif

/*
  A hierarchy is a binary tree.  We represent the hierarchy as a complete
  binary tree.  This allows us to represent the tree unambiguously as an
  array.  All rows (except possibly the last) are completely filled.  All
  nodes in the last row are as far to the left as possible.  The tree
  entries occur in the vector called tree in the following order.

          0
     1       2
   3   4   5   6
  7 8 9

  Note that it is possible to compute the indices for the children of a
  node from the index of the parent.  It is also possible to compute the
  index of the parent of a node from the index of a child.  Because of
  this, no pointers need to be stored.

  Note that if entries has n entries, then tree will have 2*n-1 entries.
  The last n elements of tree correspond to the elements of entries (in order).
*/
