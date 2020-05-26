#include "forward.h"
#include "render_world.h"
#include "light.h"
#include "sphere.h"


void Forward::Initialize_GI(){
    // entries_able
    std::vector<Entry>& GI_entries = world.hierarchy->entries;
    if(GI_entries.empty()) return;

    Box interest_box = GI_entries[0].box;
    for(auto en: GI_entries){
        interest_box = interest_box.Union(en.box);
    }

    // interest area: circle
    interest_area.center = (interest_box.hi + interest_box.lo) / 2;
    interest_area.radius = (interest_box.hi - interest_box.lo).magnitude() / 2;
    interest_area.P = interest_box.lo;
    //std::cout<<(interest_box.hi - interest_box.lo)<<std::endl;
    std::cout<<"interest radius: "<<interest_area.radius<<std::endl;
    Set_Ray();

    //Illuminate();
}

void Forward::Illuminate(){
    std::cout<<"ray_size:"<<light_ray[0].size()<<std::endl;
    for(auto L_ray: light_ray){
        for(auto ray: L_ray){
            world.Cast_Ray(ray, 1);
        }
    }
}

void Forward::Set_Ray(){
    for(auto L: world.lights){
        //  light to center
        vec3 lc = interest_area.center - L->position;
        bool outside = lc.magnitude()>interest_area.radius+small_t;
        std::cout<<"lc:"<<lc.magnitude()<<" r:"<<interest_area.radius<<std::endl;
        int n = world.forward_casting_times;
        
        // light inside or outside the interest area 
        // TODO: outside bug
        outside = false;
        if(outside){
            std::cout<<"outside"<<std::endl;
            double L_radius = lc.magnitude();
            //std::vector<Light_Ray> L_ray;
            // P = L + (u * cosa +- v * sina)
            vec3 P, u = (interest_area.P - interest_area.center).normalized(),
                v = cross(u,lc).normalized();
            // stride = degree / n
            for(double alpha=0, s1=pi/2/90/n; alpha<pi/2; alpha+=s1){
                vec3 pu = u * std::cos(alpha), pv = v * std::sin(alpha);
                //double portion = alpha / (pi/2) / (n*n);
                for(double l=0, s2=L_radius/30/n;
                    l<L_radius; l+=s2){
                    P = interest_area.center + l * (pu + pv); 
                    double cos_theta = dot((P-L->position).normalized(),vec3(0,-1,0));
                    double sin_theta = sqrt(1-cos_theta*cos_theta);
                    double portion = sin_theta / (n*n)/1.3;
                    world.Cast_Ray(Light_Ray(L->position, P-L->position, L, portion), 1);
                    P = interest_area.center + l * (pu - pv); 
                    world.Cast_Ray(Light_Ray(L->position, P-L->position, L, portion), 1);
                    P = interest_area.center + l * (-pu + pv);
                    world.Cast_Ray(Light_Ray(L->position, P-L->position, L, portion), 1);
                    P = interest_area.center + l * (-pu - pv);
                    world.Cast_Ray(Light_Ray(L->position, P-L->position, L, portion), 1);
                }
            }
            //light_ray.push_back(L_ray);
        }
        else{
        // 360 degree cast light
        // light inside the interest area
            std::cout<<"inside"<<std::endl;
            //std::vector<Light_Ray> L_ray;
            // P = {x0+R*sina*cosb, y0+R*cosa, z0+R*sina*sinb}, R = 1
            // u = {1,0,0}, v = {0,1,0}, w = {0,0,1}
            vec3 P;
            for(double alpha=0, s1=pi/2/90/n; alpha<pi/2; alpha+=s1){
                double cosa = std::cos(alpha), sina = sqrt(1-cosa*cosa);
                // TODO adjust: Portion of energy
                double portion = sina / (n*n);
                for(double beta=0, s2=pi/2/90/n; beta<pi/2; beta+=s2){
                    double cosb = std::cos(beta), sinb = sqrt(1-cosb*cosb);
                    double px = sina * cosb, py = cosa, pz = sina * sinb;
                    // Do not cast light ray if light source is higher than objects
                    if(L->position[1]<interest_area.center[1]+interest_area.radius){
                        P = {+px, +py, +pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                        P = {+px, +py, -pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                        P = {-px, +py, +pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                        P = {-px, +py, -pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                    }
                    P = {+px, -py, +pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                    P = {+px, -py, -pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                    P = {-px, -py, +pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                    P = {-px, -py, -pz}; world.Cast_Ray(Light_Ray(L->position, P, L, portion),1);
                }
            }
            //light_ray.push_back(L_ray);
        }    
    }
}



