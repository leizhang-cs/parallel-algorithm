#ifndef __LIGHT_RAY_H__
#define __LIGHT_RAY_H__

#include "ray.h"
#include "light.h"

class Object;
class Light_Ray: public Ray
{
public:
    Light *L; // Light
    // Attenuation coefficient. Each rf ray can be considered that is generated
    // from a glowing object. Its brightness: original brightness * rf_coefficient
    double attenuation;

    Light_Ray()
        :L(NULL), attenuation(1)
    {}

    Light_Ray(const vec3& endpoint_input, const vec3& direction_input,
        Light* L_input, const double attenuation_input)
        : Ray(endpoint_input, direction_input),
        L(L_input), attenuation(attenuation_input)
    {}

    vec3 Point(double t) const
    {
        return endpoint+direction*t;
    }
};
#endif
