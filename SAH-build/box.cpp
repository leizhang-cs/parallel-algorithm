#include <limits>
#include "box.h"
#include "object.h"

/*
bool Box::Intersection(const Ray& ray) const
{
    double rmin = 0.0, rmax = std::numeric_limits<double>::infinity(),\
        tmin, tmax, temp, dir;
    vec3 xmin = lo - ray.endpoint, xmax = hi - ray.endpoint;

    dir = ray.direction[0];
    if(dir<small_t && dir>-small_t){
        if(xmin[0]>0.0 || xmax[0]<0.0) return false;
    }
    else{
        dir = 1.0/dir;
        rmin = xmin[0] * dir;
        rmax = xmax[0] * dir;
        if(rmin>rmax){ temp = rmin; rmin = rmax; rmax = temp; }
    }
    dir = ray.direction[1];
    if(dir<small_t && dir>-small_t){
        if(xmin[1]>0.0 || xmax[1]<0.0) return false;
    }
    else{
        dir = 1.0/dir;
        tmin = xmin[1] * dir;
        tmax = xmax[1] * dir;
        if(tmin>tmax){ temp = tmin; tmin = tmax; tmax = temp; }

        if(rmin>tmax || tmin>rmax) return false;

        if(tmin>rmin) rmin = tmin;
        if(tmax<rmax) rmax = tmax;
    }
    dir = ray.direction[2];
    if(dir<small_t && dir>-small_t){
        if(xmin[2]>0.0 || xmax[2]<0.0) return false;
    }
    else{
        dir = 1.0/dir;
        tmin = xmin[2] * dir;
        tmax = xmax[2] * dir;
        if(tmin>tmax){ temp = tmin; tmin = tmax; tmax = temp; }

        if(rmin>tmax || tmin>rmax) return false;
    }
    return true;
}
*/
