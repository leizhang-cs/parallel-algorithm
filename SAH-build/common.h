#ifndef __COMMON_H__
#define __COMMON_H__

#include "box.h"
#include "hierarchy.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <atomic>

struct Node{
    Box box;
    Node* lChild;
    Node* rChild;
    int begin; // Entry indices
    int end;
    
    Node():lChild(nullptr),rChild(nullptr),begin(-1),end(-1){}
};

struct compare{
    compare(int dimension_input):dimension(dimension_input){}
    bool operator()(const Entry& e1, const Entry& e2){
        return (e1.box.lo+e1.box.hi)[dimension]<(e2.box.lo+e2.box.hi)[dimension];
    }
    int dimension;
};

#endif