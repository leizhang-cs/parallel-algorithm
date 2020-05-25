#include "sphere.h"
#include "ray.h"
#include "light_ray.h"
#include "light.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part)
{
    vec3 w = ray.endpoint - center;
    // t = t1 - sqrt(t2);
    // w * u > 0?
    double t1 = - dot(w, ray.direction), t2;
    t2 = t1 * t1 - dot(w, w) + radius * radius;
    if(t2<0 || t1-sqrt(t2)<0) return {NULL, 0, 0};
    else return {this, t1 - sqrt(t2), part};
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
    vec3 normal;
    // compute the normal direction
    normal = (point - center).normalized();
    return normal;
}

Box Sphere::Bounding_Box(int part) const
{
    Box box;
    box.lo = center;
    box.hi = center;
    for(int i=0; i<3; i++){
        box.lo[i] -= radius;
        box.hi[i] += radius;
    }
    return box;
}

void Sphere::Update_GI(const vec3& energy, const vec3& P, int part)
{
    //std::cout<<"sphere update"<<std::endl;
    vec3 CP = (P - center) / radius;
    double cosa = CP[1], sina = sqrt(1-cosa*cosa);
    // sina ~= 0, update intense at top or btm
    if(sina<small_t){
        if(CP[1]>0) uv_map.back()[0] += energy;
        else uv_map[0][0] += energy;
    }
    else{
        double u, v, p, q;
        int u1, v1, u2, v2;
        // radian => angle degree: k
        double cosb = CP[0] / sina, k = 180 / pi;
        // surface function = -R^2 * (sina*sina*cosb, sina*cosa, sina*sina*sinb)
        // magnitude of sf: R^2 * sina
        // intensity = energy / delta_area
        // TODO ???
        vec3 intensity = energy / (sina*radius*radius) * 3;
        u = std::acos(cosa) * k;
        v = std::acos(cosb) * k;
        // z<0, beta = 2*pi - beta, beta: [0,pi]
        if(CP[2]<-small_t){ v = 360 - v; }
        u1 = u, u2 = (u1+1)%uv_map.size();
        v1 = v, v2 = (v1+1)%uv_map[0].size();
        p = u - u1, q = v - v1;
        // Interpolate intensity
        uv_map[u1][v1] += intensity*(1-p)*(1-q);
        uv_map[u1][v2] += intensity*(1-p)*q;
        uv_map[u2][v1] += intensity*p*(1-q);
        uv_map[u2][v2] += intensity*p*q;
    }
}

vec3 Sphere::Get_GI(const vec3& P, int part) const
{
    vec3 CP = (P - center) / radius;
    double cosa = CP[1], sina = sqrt(1-cosa*cosa);
    // sina ~= 0, update intensity at top or btm
    if(sina<small_t){
        if(CP[1]>0) return uv_map.back()[0];
        else return uv_map[0][0];
    }
    else{
        vec3 intensity;
        double u, v, p, q;
        int u1, v1, u2, v2;
        double cosb = CP[0] / sina, k = 1 / pi * 180;
        u = std::acos(cosa) * k;
        v = std::acos(cosb) * k;
        // z<0, beta = 2*pi - beta, beta: [0,pi]
        if(CP[2]<-small_t){ v = 360 - v; }
        u1 = u, u2 = (u1+1)%uv_map.size();
        v1 = v, v2 = (v1+1)%uv_map[0].size();
        p = u - u1, q = v - v1;
        intensity += uv_map[u1][v1]*(1-p)*(1-q);
        intensity += uv_map[u1][v2]*(1-p)*q;
        intensity += uv_map[u2][v1]*p*(1-q);
        intensity += uv_map[u2][v2]*p*q;
        return intensity;
    }
}
