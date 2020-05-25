#ifndef __REFRACTIVE_SHADER_H__
#define __REFRACTIVE_SHADER_H__

#include <algorithm>
#include "shader.h"

class Refractive_Shader : public Shader
{
public:
    Shader* shader;
    // Default N_outside = N_air = 1.0
    double N_obj;
    
    Refractive_Shader(Render_World& world_input,Shader* shader_input,double N_input)
        :Shader(world_input, true),shader(shader_input),N_obj(N_input)
    {}

    virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const override;
    virtual vec3 Shade_Surface(const vec3& light_intense) const override;
    virtual void Illuminate_Surface(const Light_Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const override;
    virtual void Get_Color(vec3& color) const{
        shader->Get_Color(color);
    }
};
#endif