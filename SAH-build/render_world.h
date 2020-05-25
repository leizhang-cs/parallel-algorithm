#ifndef __RENDER_WORLD_H__
#define __RENDER_WORLD_H__

#include <vector>
#include "camera.h"
#include "hierarchy.h"
#include "SAH_build.h"
class Light;
class Shader;
class Ray;
class Light_Ray;

class Render_World
{
public:
    Camera camera;

    Shader *background_shader;
    std::vector<Object*> objects;
    std::vector<Light*> lights;
    vec3 ambient_color;
    double ambient_intensity;

    bool enable_shadows;
    bool disable_fresnel_refraction;
    bool disable_fresnel_reflection;
    int recursion_depth_limit;
    int anti_aliasing_samples;
    bool disable_forward;
    int forward_casting_times;
    bool incremental_build = false;
    bool sah_sweep = false;


    Hierarchy hierarchy; // incremental
    SAH_Build sah_build; // SAH

    Render_World();
    ~Render_World();

    void Render_Pixel(const ivec2& pixel_index);
    void Render();
    void Initialize_Hierarchy();

    vec3 Cast_Ray(const Ray& ray, int recursion_depth);
    void Cast_Ray(const Light_Ray& ray, int recursion_depth);
    Hit Closest_Intersection(const Ray& ray);
};
#endif
