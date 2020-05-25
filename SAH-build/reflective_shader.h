#ifndef __REFLECTIVE_SHADER_H__
#define __REFLECTIVE_SHADER_H__

#include <algorithm>
#include "shader.h"

class Reflective_Shader : public Shader
{
public:
    Shader* shader;
    double reflectivity;
    
    Reflective_Shader(Render_World& world_input,Shader* shader_input,double reflectivity_input)
        :Shader(world_input, true),shader(shader_input),
        reflectivity(std::max(0.0,std::min(1.0,reflectivity_input)))
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
