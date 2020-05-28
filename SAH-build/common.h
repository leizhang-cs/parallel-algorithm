#ifndef __COMMON_H__
#define __COMMON_H__

#include "box.h"
#include "hierarchy.h"

struct compare{
    compare(int dimension_input):dimension(dimension_input){}
    bool operator()(const Entry& e1, const Entry& e2){
        return (e1.box.lo+e1.box.hi)[dimension]<(e2.box.lo+e2.box.hi)[dimension];
    }
    int dimension;
};

#endif