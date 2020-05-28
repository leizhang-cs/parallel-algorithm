#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"


vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    //determine the color = ambient + diffuse + specular
    //l light, r reflect, v view(ray), L light intense, I intense 
    vec3 color, l, r, v, L_d, I;
    double nl;
    v = (ray.endpoint - intersection_point).normalized();
    //ambient = R_a*L_a;
    color = color_ambient * world.ambient_color * world.ambient_intensity;

    for(auto L: world.lights){
        l = L->position - intersection_point;
        if(world.enable_shadows){
            // Ray of light
            Hit hit = world.Closest_Intersection(Ray(intersection_point, l));
            // hit.dist<l.magnitute(), make sure object behind light
            if(hit.dist>small_t && hit.dist<l.magnitude()){
                // if shadow, color = phong_shader_color*obj.transmittance
                /*if(hit.object->material_shader->transmittance>small_t){
                    l = l.normalized();
                    nl = dot(normal, l);
                    if(nl<=0) continue;
                    L_d = L->Emitted_Light(intersection_point - L->position);
                    I = color_diffuse * L_d * nl;
                    r = 2 * nl * normal - l;
                    I += color_specular * L_d * std::pow(max(dot(v, r), 0.0), specular_power);
                    color += I*hit.object->material_shader->transmittance;   
                }*/
                continue;
            }
        }
        // nl = max(n*l, 0)
        l = l.normalized();
        nl = dot(normal, l);
        if(nl<=0) continue;

        // diffuse = R_d*L_d*max(n*l, 0)
        L_d = L->Emitted_Light(intersection_point - L->position);
        I = color_diffuse * L_d * nl;
        
        // specular = R_s*L_s(L_d)*max(v*r, 0)^pow
        r = 2 * nl * normal - l;
        I += color_specular * L_d * std::pow(max(dot(v, r), 0.0), specular_power);

        //if(debug_pixel){ std::cout<<std::endl; }
        color += I;
    }
    return color;
}