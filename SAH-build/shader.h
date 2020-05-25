#ifndef __SHADER_H__
#define __SHADER_H__

#include "vec.h"
class Render_World;
class Ray;
class Light_Ray;

extern bool debug_pixel;

class Shader
{
public:
    Render_World& world;

    // obj can reflect or refract light
    bool rf_able;

    Shader(Render_World& world_input)
        :world(world_input),rf_able(false)
    {}

    Shader(Render_World& world_input, bool rf_able_input)
        :world(world_input),rf_able(rf_able_input)
    {}

    virtual ~Shader()
    {}

    virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const=0;
    virtual vec3 Shade_Surface(const vec3& light_intense) const
    { return {0,0,0}; }
    // TODO eliminate xx_shader.cpp #include "light_ray.h"
    virtual void Illuminate_Surface(const Light_Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const
    {}
    virtual void Get_Color(vec3& color) const
    {}
};
#endif
