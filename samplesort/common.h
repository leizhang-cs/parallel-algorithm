#ifndef __COMMON_H__
#define __COMMON_H__


struct compare{
    bool operator()(const int& e1, const int& e2){
        return e1<e2;
    }
};

#endif