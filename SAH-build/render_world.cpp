#include "render_world.h"
#include "light.h"
#include "ray.h"
#include "light_ray.h"
#include "sphere.h"
#include "shader.h"
#include "forward.h"
#include "inline_func.cpp"
#include <time.h>

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3),anti_aliasing_samples(1),
    disable_forward(true),forward_casting_times(1)
{}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
    Hit hit = {NULL, 0, 0}, temp;

    if(incremental_build){
        std::vector<int> candidates;
        hierarchy.Intersection_Candidates(ray, candidates);
        //if(debug_pixel) std::cout<<"entries.size: "<<hierarchy.entries.size()<<std::endl;
        //if(debug_pixel) std::cout<<"candidates.size: "<<candidates.size()<<std::endl;
        for(auto i: candidates){
            temp = hierarchy.entries[i].obj->Intersection(ray, hierarchy.entries[i].part);
            if(temp.dist>=small_t && (!hit.object || temp.dist<hit.dist)){
                hit = temp;
            }
        }
    }
    // if(debug_pixel){
    //     std::cout<<"htree box:"<<hierarchy.tree[0].lo<<" "<<hierarchy.tree[0].hi<<std::endl;
    //     std::cout<<"root box:"<<sah_build.root->box.lo<<" "<<sah_build.root->box.hi<<std::endl;
    //     if(hierarchy.tree[0].Intersection(ray)){ std::cout<<"h"<<std::endl; }
    //     if(sah_build.root->box.Intersection(ray)){ std::cout<<"sah"<<std::endl; }
    // }
    if(sah_sweep){
        std::vector<Entry*> candidates;
        sah_build.Intersection_Candidates(ray, candidates);
        if(debug_pixel) std::cout<<candidates.size()<<std::endl;
        for(auto en: candidates){
            temp = en->obj->Intersection(ray, en->part);
            if(temp.dist>=small_t && (!hit.object || temp.dist<hit.dist)){
                hit = temp;
            }
        }
    }

    for(auto en: hierarchy.entries_inf){
        temp = en.obj->Intersection(ray, en.part);
        if(temp.dist>=small_t && (!hit.object || temp.dist<hit.dist)){
            hit = temp;
        }
    }
    //if(debug_pixel) std::cout<<"dist: "<<hit.dist<<" part: "<<hit.part<<std::endl;
    return hit;
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    // set up the initial view ray here
    vec3 color;
    if(anti_aliasing_samples>1){
        std::vector<Ray> ray;
        camera.Set_Ray(pixel_index, ray, anti_aliasing_samples);
        for(auto r: ray){
            color += Cast_Ray(r, 1);
            // if(hierarchy.tree.empty()||!hierarchy.tree[0].Intersection(r)){
            //     camera.Set_Pixel(pixel_index,Pixel_Color(color));
            //     return;
            // }
        }
        color /= ray.size();
    }
    else{
        Ray ray = camera.Set_Ray(pixel_index);
        color = Cast_Ray(ray,1);
    }
    camera.Set_Pixel(pixel_index,Pixel_Color(color));
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy();

    double start = clock();
    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));

    double end = clock();
    std::cout<<"render time: "<<(end-start)/CLOCKS_PER_SEC<<std::endl;
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    // determine the color here
    vec3 intersection_point, normal;
    Hit hit = Closest_Intersection(ray);
    if(!hit.object){
        return background_shader->Shade_Surface(ray, \
            intersection_point, normal, recursion_depth);
    }
    else{
        vec3 color;
        intersection_point = ray.Point(hit.dist);
        normal = hit.object->Normal(intersection_point, hit.part);
        color += hit.object->material_shader->Shade_Surface(ray, \
            intersection_point, normal, recursion_depth);
        vec3 Intense = hit.object->Get_GI(intersection_point, hit.part);
        color += hit.object->material_shader->Shade_Surface(Intense);
        return color;    
    }
}

// L: Light, I: Intense
void Render_World::Cast_Ray(const Light_Ray& ray, int recursion_depth)
{
    //if(recursion_depth==1) std::cout<<"e:"<<ray.endpoint<<" d:"<<ray.direction<<" ";
    
    Hit hit = Closest_Intersection(ray);
    if(!hit.object){
        //std::cout<<"unhit"<<std::endl;
        return;
    }
    else{
        vec3 intersection_point = ray.Point(hit.dist);
        vec3 normal = hit.object->Normal(intersection_point, hit.part);

        if(hit.object->material_shader->rf_able){
            //std::cout<<"able"<<std::endl;
            hit.object->material_shader->Illuminate_Surface(ray, intersection_point, 
                normal, recursion_depth);
        }
        else{
            if(recursion_depth==1){
                //std::cout<<"unable 1"<<std::endl;
                return;
            }
            else{
                double brightness = ray.L->Light_Brightness(hit.dist, ray.attenuation);
                hit.object->Update_GI(ray.L->color*brightness, intersection_point, hit.part);
            }
        }
    }
}

void Render_World::Initialize_Hierarchy()
{
    for(auto obj: objects){
        for(int i=0; i<obj->number_parts; i++){
            if(std::isinf(obj->Bounding_Box(i).hi[0])){
                hierarchy.entries_inf.push_back({obj, i, obj->Bounding_Box(i)});
            }
            else{
                hierarchy.entries.push_back({obj, i, obj->Bounding_Box(i)});
                sah_build.entries.push_back({obj, i, obj->Bounding_Box(i)});
                if(obj->material_shader->rf_able){
                    hierarchy.entries_rfable.push_back({obj, i, obj->Bounding_Box(i)});
                }
            }
        }
    }
    // Fill in hierarchy.entries; there should be one entry for
    // each part of each object.
    double start = clock();
    if(incremental_build) hierarchy.Build(hierarchy.entries);
    if(sah_sweep) sah_build.Build(sah_build.entries);
    double end = clock();
    std::cout<<"build time: "<<(end-start)/CLOCKS_PER_SEC<<std::endl;
    
    //std::cout<<hierarchy.entries.size()<<" "<<hierarchy.tree.size()<<std::endl;
    //std::cout<<"rfable_size:"<<hierarchy.entries_rfable.size()<<std::endl;
    
    if(!disable_forward){
        double start = clock();
        Forward forward_phase(*this);
        forward_phase.Initialize_GI();
        double end = clock();
        std::cout<<"illuminate time: "<<(end-start)/CLOCKS_PER_SEC<<std::endl;
    }
}
