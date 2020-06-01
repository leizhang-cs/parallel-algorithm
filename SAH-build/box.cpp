#include <limits>
#include "box.h"
#include "object.h"


// Compute the smallest box that contains both *this and bb.
Box Box::Union(const Box& bb) const
{
    Box box;
    box.lo = componentwise_min(bb.lo, lo);
    box.hi = componentwise_max(bb.hi, hi);
    return box;
}

// Enlarge this box (if necessary) so that pt also lies inside it.
void Box::Include_Point(const vec3& pt)
{
    lo = componentwise_min(pt, lo);
    hi = componentwise_max(pt, hi);
}

// Create a box to which points can be correctly added using Include_Point.
void Box::Make_Empty()
{
    lo.fill(std::numeric_limits<double>::infinity());
    hi=-lo;
}

// half of surface area
double Box::Surface_Area() const{
    double x = hi[0]-lo[0], y = hi[1]-lo[1], z = hi[2]-lo[2];
    return x*y + y*z + x*z;
}

// 2*center
vec3 Box::Center() const
{
    return lo + hi;
}
/*
bool Box::Intersection(const Ray& ray) const
{
    // t = (lo-ray.e)*n/(ray.d*n), normal = (0,0,1) or (0,1,0) or (0,0,1)
    double t, dn, xj, xk;
    vec3 xmin, xmax;
    xmin.fill(std::numeric_limits<double>::max());
    xmax.fill(-std::numeric_limits<double>::max());
    int i, j, k;

    for(i=0; i<3; i++){
        dn = ray.direction[i];
        if(std::abs(dn)>1e-4){
            dn = 1.0 / dn;
            t = (lo-ray.endpoint)[i] * dn;
            j = (i+1)%3; k = (i+2)%3;
            if(t>=small_t){
                xj = ray.endpoint[j] + t*ray.direction[j];
                xk = ray.endpoint[k] + t*ray.direction[k];
                if(xj<xmin[j]) xmin[j] = xj;
                xmax[k] = max(xmax[k], xk);
            }
            t = (hi-ray.endpoint)[i] * dn;
            if(t>=small_t){
                xj = ray.endpoint[j] + t*ray.direction[j];
                xk = ray.endpoint[k] + t*ray.direction[k];
                if(xj<xmin[j]) xmin[j] = xj;
                xmax[k] = max(xmax[k], xk);
            }
        }
    }
    if((xmax[0]<=hi[j]+small_t && xmin[0]>=lo[j]-small_t) &&\
        (xmax[1]<=hi[k]+small_t && xmin[1]>=lo[k]-small_t) &&\
        (xmax[2]<=hi[k]+small_t && xmin[2]>=lo[k]-small_t))
        return true;
    return false;
}*/
