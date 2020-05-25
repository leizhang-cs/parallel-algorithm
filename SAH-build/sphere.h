#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "object.h"

class Sphere : public Object
{
    vec3 center;
    double radius;
    // attributes map: light intense for RGB
    std::vector<std::vector<vec3>> uv_map;
    //delta_area((4*pi*radius_input*radius_input)/(180*360)),

public:
    Sphere(const vec3& center_input,double radius_input)
        :center(center_input),radius(radius_input),
        uv_map(180, std::vector<vec3>(360))
    {}

    virtual Hit Intersection(const Ray& ray, int part) override;
    virtual vec3 Normal(const vec3& point, int part) const override;
    virtual Box Bounding_Box(int part) const override;
    virtual void Update_GI(const vec3& intense, const vec3& P, int part) override;
    virtual vec3 Get_GI(const vec3& P, int part) const override;
};
#endif
