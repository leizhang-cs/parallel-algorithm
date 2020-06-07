#ifndef __BOX_H__
#define __BOX_H__

#include "ray.h"
#include <limits>

class Box
{
public:
    // lowermost and uppermost corners of bounding box
    vec3 lo,hi;

    // Return whether the ray intersects this box.
    bool Intersection(const Ray& ray) const;

    // Compute the smallest box that contains both *this and bb.
    Box Union(const Box& bb) const {
        Box box;
        box.lo = componentwise_min(bb.lo, lo);
        box.hi = componentwise_max(bb.hi, hi);
        return box;
    }

    // Enlarge this box (if necessary) so that pt also lies inside it.
    void Include_Point(const vec3& pt){
        lo = componentwise_min(pt, lo);
        hi = componentwise_max(pt, hi);
    }

    // Create a box to which points can be correctly added using Include_Point.
    void Make_Empty(){ lo.fill(std::numeric_limits<double>::infinity()); hi=-lo; }
    
    // half of surface area
    double Surface_Area() const {
        double x = hi[0]-lo[0], y = hi[1]-lo[1], z = hi[2]-lo[2];
        return x*y + y*z + x*z;
    }
    
    // 2*center
    vec3 Center() const { return lo + hi; }
};


#endif
