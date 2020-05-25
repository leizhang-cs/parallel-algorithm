#ifndef __TRANSPARENT_SHADER_H__
#define __TRANSPARENT_SHADER_H__

#include <algorithm>
#include "shader.h"

class Transparent_Shader : public Shader
{
public:
    Shader* shader;
    // 0<=reflectivity, transmittance<=1, reflectivity+transmittance<=1
    // Default N_outside = N_air = 1.0
    double transmittance, reflectivity, N_obj=1.5, N_out = 1.0;
    
    Transparent_Shader(Render_World& world_input,Shader* shader_input,double transmittance_input,\
        double reflectivity_input):Shader(world_input),shader(shader_input),\
        transmittance(transmittance_input), reflectivity(reflectivity_input)
    {
        transmittance = std::max(0.0,std::min(1.0,transmittance));
        transparency = transmittance;
        reflectivity = std::max(0.0,std::min(1.0-transmittance,reflectivity));
    }

    virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const override;
    
};
#endif
