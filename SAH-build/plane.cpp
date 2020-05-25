#include "plane.h"
#include "ray.h"
#include <cfloat>
#include <limits>
#include "vec.h"

// Intersect with the half space defined by the plane.  The plane's normal
// points outside.  If the ray starts on the "inside" side of the plane, be sure
// to record a hit with t=0 as the first entry in hits.
Hit Plane::Intersection(const Ray& ray, int part)
{
    vec3 w = ray.endpoint - x1;
    double wn, t;
    // t = d * n. if t==0, ray is parallel with plane
    t = dot(ray.direction, normal);
    // wn = max(w*n, 0). if wn<0, ray is inside
    wn = dot(w, normal);
    if(std::abs(t)<small_t || wn<-small_t) return {NULL,0,0};
    else{
        // (w+td) * n = 0, dist: t = -w*n/d*n
        t = - wn / t;
        return {this, t, part};
    }
}

vec3 Plane::Normal(const vec3& point, int part) const
{
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Plane::Bounding_Box(int part) const
{
    Box b;
    b.hi.fill(std::numeric_limits<double>::infinity());
    b.lo=-b.hi;
    return b;
}
