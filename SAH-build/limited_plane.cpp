#include "limited_plane.h"


Hit Limited_Plane::Intersection(const Ray& ray, int part)
{
    vec3 w = ray.endpoint - min_corner;
    double wn, t;
    // t = d * n. if t==0, ray is parallel with plane
    t = dot(ray.direction, normal);
    // wn = max(w*n, 0). if wn<0, ray is inside
    wn = dot(w, normal);
    if(std::abs(t)<small_t || wn<-small_t) return {NULL,0,0};
    else{
        // (w+td) * n = 0, dist: t = -w*n/d*n
        t = - wn / t;
        vec3 P = ray.Point(t);
        if(P[0]<max_corner[0] && P[1]<max_corner[1] && P[2]<max_corner[2] &&
            P[0]>min_corner[0] && P[1]>min_corner[1] && P[2]>min_corner[2]){
            return {this, t, part};
        }
        else return {NULL,0,0};
    }
}

vec3 Limited_Plane::Normal(const vec3& point, int part) const
{
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Limited_Plane::Bounding_Box(int part) const
{
    Box b;
    b.lo = min_corner;
    b.hi = max_corner;
    return b;
}

void Limited_Plane::Update_GI(const vec3& intense, const vec3& P, int part)
{
    vec3 P, u = (interest_area.P - interest_area.center).normalized(),
                v = cross(u,lc).normalized();
            // stride = degree / n
    for(double alpha=0, s1=pi/2/90/n; alpha<pi/2; alpha+=s1){
        vec3 pu = u * std::cos(alpha), pv = v * std::sin(alpha);
        for(double l=0, s2=interest_area.radius/100/n;
            l<interest_area.radius; l+=s2){
            
            P = interest_area.center + l * (pu + pv); 
            L_ray.push_back(Light_Ray(L->position, P-L->position, L, 1));
            P = interest_area.center + l * (pu - pv); 
            L_ray.push_back(Light_Ray(L->position, P-L->position, L, 1));
            P = interest_area.center + l * (-pu + pv);
            L_ray.push_back(Light_Ray(L->position, P-L->position, L, 1));
            P = interest_area.center + l * (-pu - pv);
            L_ray.push_back(Light_Ray(L->position, P-L->position, L, 1));
        }
    }
}

vec3 Limited_Plane::Get_GI(const vec3& P, int part) const
{

}
