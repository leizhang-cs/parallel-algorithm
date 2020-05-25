#include "reflective_shader.h"
#include "ray.h"
#include "light_ray.h"
#include "render_world.h"
#include "point_light.h"

vec3 Reflective_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    // color = C_obj + reflectivity*(C_o - C_rflection), 0<=reflectivity<=1
    vec3 color, Co, Cr;
    Co = shader->Shade_Surface(ray, intersection_point, normal, recursion_depth);
    if(recursion_depth<world.recursion_depth_limit){
        // v view, r reflection direction
        //vec3 v = (ray.endpoint - intersection_point).normalized();
        double nv = dot(normal, -ray.direction);
        if(nv>0){
            vec3 r = 2*nv*normal + ray.direction;
            Ray ray_r(intersection_point, r);
            Cr = world.Cast_Ray(ray_r, recursion_depth+1);
        }
    }
    if(debug_pixel) std::cout<<"reflect depth:"<<recursion_depth<<std::endl;
    color = Co + reflectivity * (Cr - Co);
    return color;
}

vec3 Reflective_Shader::
Shade_Surface(const vec3& light_intense) const 
{
    return shader->Shade_Surface(light_intense);
}

void Reflective_Shader::
Illuminate_Surface(const Light_Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    if(recursion_depth<world.recursion_depth_limit){
        double cosv = dot(normal, -ray.direction);
        if(cosv>0){
            vec3 r = 2*cosv*normal + ray.direction;
            vec3 color;
            shader->Get_Color(color);
            color *= ray.L->color;
            Point_Light* L = new Point_Light(ray.L->position, color, ray.L->brightness);
            Light_Ray ray_r(intersection_point, r, L, 
                ray.attenuation*reflectivity);
            world.Cast_Ray(ray_r, recursion_depth+1);
        }
    }
}
