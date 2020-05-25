#ifndef __LIMITED_PLANE_H__
#define __LIMITED_PLANE_H__

#include "plane.h"

class Limited_Plane : public Plane
{
public:
    vec3 min_corner;
    vec3 max_corner;
    std::vector<std::vector<vec3>> uv_map;

    Limited_Plane(const vec3& point,const vec3& normal,const vec3& min_corner,const vec3& max_corner)
        :Plane(point,normal),min_corner(min_corner),max_corner(max_corner)
    {}

    virtual Hit Intersection(const Ray& ray, int part) override;
    virtual vec3 Normal(const vec3& point, int part) const override;
    virtual Box Bounding_Box(int part) const override;
    virtual void Update_GI(const vec3& intense, const vec3& P, int part) override;
    virtual vec3 Get_GI(const vec3& P, int part) const override;
};
#endif
