#include "transparent_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Transparent_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
const vec3& normal,int recursion_depth) const
{
    // color = C_obj + reflectivity*(C_o - C_r) + transmittance*(C_o - C_t)
    vec3 color, Co, Cr, Ct;
    Co = shader->Shade_Surface(ray, intersection_point, normal, recursion_depth);
    if(recursion_depth<world.recursion_depth_limit){
        // v view=-ray.d, r reflection direction, if nv<0, ray from air to obj
        double nv = dot(normal, ray.direction), nv_dir = 1.0;
        // transmission direction: t
        vec3 r, t;
        if(nv<0){ nv = -nv; nv_dir = -nv_dir; }
        if(nv_dir<0) r = 2*nv*normal + ray.direction;
        else r = -2*nv*normal + ray.direction;
        Ray ray_r(intersection_point, r);
        Cr = world.Cast_Ray(ray_r, recursion_depth+1);
        
        // Snell's Law: N_in*sin_view = N_out*sin_trans
        double sint, sinv = sqrt(1-nv*nv);
        if(nv_dir<0) sint = N_out*sinv/N_obj;
        else sint = N_obj*sinv/N_out;
        // If sint>=1, complete internal reflection
        
        if(sint<1){
            double cost = sqrt(1-sint*sint);
            // t = horizontal_vec*sint + vertical_vec*cost
            // horizontal_vec = (v + (-+cos_v*n)).normalized(), ver_vec = -+normal
            if(nv_dir<0){
                t = (ray.direction+normal*nv).normalized()*sint - normal*cost;
            }
            else{
                t = (ray.direction-normal*nv).normalized()*sint + normal*cost;
            }
            Ray ray_t(intersection_point, t);
            Ct = world.Cast_Ray(ray_t, recursion_depth+1);
        }
        if(debug_pixel) std::cout<<"Depth:"<<recursion_depth<<std::endl;
        if(debug_pixel) std::cout<<"nv_dir:"<<nv_dir<<std::endl;
    }
    if(debug_pixel) std::cout<<"this_t:"<<transmittance<<" Shader_t:"<<transparency<<std::endl;
    color = Co + reflectivity*(Cr - Co) + transmittance*(Ct - Co);
    return color;
}
