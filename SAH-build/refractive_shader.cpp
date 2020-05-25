#include "refractive_shader.h"
#include "ray.h"
#include "light_ray.h"
#include "render_world.h"

vec3 Refractive_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
const vec3& normal,int recursion_depth) const
{
    vec3 color, Co, Cr, Ct;
    // color = C_obj + reflectivity*(C_o - C_r) + transmittance*(C_o - C_t)
    double reflectivity = 0, transmittance = 0;

    Co = shader->Shade_Surface(ray, intersection_point, normal, recursion_depth);
    if(recursion_depth<=world.recursion_depth_limit){
        // v view=-ray.d, r reflection direction, if cosv<0, ray from air to obj
        double cosv = dot(normal, ray.direction), nv_dir = 1;
        double N_v = 1.0, N_t = N_obj;
        // transmission direction: t
        vec3 r, t;
        if(cosv<0){ cosv = -cosv; nv_dir = -1; }
        else std::swap(N_v, N_t);
        r = 2*cosv*(-nv_dir)*normal + ray.direction;
        Ray ray_r(intersection_point, r);
        Cr = world.Cast_Ray(ray_r, recursion_depth+1);
        
        // Snell's Law: N_in*sin_view = N_t*sin_trans
        double sint, sinv = sqrt(1-cosv*cosv);
        sint = N_v*sinv/N_t;
        // If sint>=1, complete internal reflection
        
        if(sint>=1){ reflectivity = 1; }
        else{
            double cost = sqrt(1-sint*sint);
            // Fresnel equations: reflectivity = (r_vertical^2 + r_parallel^2) / 2
            double r1, r2;
            r1 = (N_t*cosv - N_v*cost) / (N_t*cosv + N_v*cost);
            r2 = (N_v*cosv - N_t*cost) / (N_v*cosv + N_t*cost);
            reflectivity = (r1*r1 + r2*r2) / 2;
            // t = horizontal_vec*sint + vertical_vec*cost
            // horizontal_vec = (v + (-+cos_v*n)).normalized(), vertical_vec = -+normal
            vec3 v = nv_dir * normal, h = (ray.direction - nv_dir*cosv*normal)/sinv;
            t = h*sint + v*cost;
            Ray ray_t(intersection_point, t);
            Ct = world.Cast_Ray(ray_t, recursion_depth+1);
        }
        transmittance = 1 - reflectivity;
    }
    color = Co + reflectivity*(Cr-Co) + transmittance*(Ct-Co);

    return color;
}

vec3 Refractive_Shader::
Shade_Surface(const vec3& light_intense) const
{
    return shader->Shade_Surface(light_intense);
}

void Refractive_Shader::
Illuminate_Surface(const Light_Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    if(recursion_depth<=world.recursion_depth_limit){
        double reflectivity = 0, transmittance = 0;
        // v view=-ray.d, r reflection direction, if cosv<0, ray from air to obj
        double cosv = dot(normal, ray.direction), nv_dir = 1;
        double N_v = 1.0, N_t = N_obj;
        
        if(cosv<0){ cosv = -cosv; nv_dir = -1; }
        else std::swap(N_v, N_t);
        
        // Snell's Law: N_in*sin_view = N_t*sin_trans
        double sint, sinv = sqrt(1-cosv*cosv);
        sint = N_v*sinv/N_t;
        // If sint>=1, complete internal reflection
        
        if(sint>=1){ reflectivity = 1; }
        else{
            double cost = sqrt(1-sint*sint);
            // Fresnel equations: reflectivity = (r_vertical^2 + r_parallel^2) / 2
            double r1, r2;
            r1 = (N_t*cosv - N_v*cost) / (N_t*cosv + N_v*cost);
            r2 = (N_v*cosv - N_t*cost) / (N_v*cosv + N_t*cost);
            reflectivity = (r1*r1 + r2*r2) / 2;
            transmittance = 1 - reflectivity;
            // t = horizontal_vec*sint + vertical_vec*cost
            // horizontal_vec = (v + (-+cos_v*n)).normalized(), vertical_vec = -+normal
            vec3 v = nv_dir * normal, h = (ray.direction - nv_dir*cosv*normal)/sinv;
            // refraction direction: t
            vec3 t = h*sint + v*cost;
            Light_Ray ray_t(intersection_point, t, ray.L, ray.attenuation*transmittance);
            world.Cast_Ray(ray_t, recursion_depth+1);
        }
        // reflection direction
        vec3 r = 2*cosv*(-nv_dir)*normal + ray.direction;
        Light_Ray ray_r(intersection_point, r, ray.L, ray.attenuation*reflectivity);
        world.Cast_Ray(ray_r, recursion_depth+1);
    }
}   