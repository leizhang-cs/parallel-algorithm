#ifndef __FORWARD_H__
#define __FORWARD_H__

#include "vec.h"
#include "light_ray.h"
#include <vector>
class Render_World;

struct Area
{
    vec3 center;
    vec3 P;
    double radius;
};

class Forward
{
public:
    // TODO: move to class render_world
    Render_World& world;
    Area interest_area;
    std::vector<std::vector<Light_Ray>> light_ray;

    Forward(Render_World& world_input)
        :world(world_input)
    {}

    // Entries_box -> interest areas -> ray
    void Initialize_GI();
    // Set bunch of rays for each light
    void Set_Ray();
    // Cast light ray and update GI
    void Illuminate();
};
#endif
