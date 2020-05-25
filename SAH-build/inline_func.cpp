#include "camera.h"
#include "vec.h"
#include <queue>

inline
vec3 Camera::World_Position(const ivec2& pixel_index, const ivec2& cell_index, const int n){
    vec2 index;
    if(n>1) index = Cell_Postion(pixel_index, cell_index, n);
    else index = Cell_Center(pixel_index);
    vec3 result = film_position;
    result += horizontal_vector * index[0];
    result += vertical_vector * index[1];
    return result;
}

inline
void Camera::Set_Ray(const ivec2& pixel_index, std::vector<Ray>& ray, int samples){
    vec3 endpoint = position, direction;
    // Ray numbers: n*n
    int i, j, n;
    ivec2 cell_index;
    for(n=1; n*n<samples; n++);
    //if(debug_pixel) std::cout<<n*n<<std::endl;
    for(i=0; i<n; i++){
        for(j=0; j<n; j++){
            direction = World_Position(pixel_index, ivec2(i, j), n) - endpoint;
            ray.push_back(Ray(endpoint, direction));
        }
    }
}

// Return whether the ray intersects this box.
inline
bool Box::Intersection(const Ray& ray) const
{
    double rmin = 0.0, rmax = std::numeric_limits<double>::infinity(),\
        tmin, tmax, temp, dir;
    vec3 xmin = lo - ray.endpoint, xmax = hi - ray.endpoint;

    dir = ray.direction[0];
    if(dir<small_t && dir>-small_t){
        if(xmin[0]>0.0 || xmax[0]<0.0) return false;
    }
    else{
        dir = 1.0/dir;
        rmin = xmin[0] * dir;
        rmax = xmax[0] * dir;
        if(rmin>rmax){ temp = rmin; rmin = rmax; rmax = temp; }
    }
    dir = ray.direction[1];
    if(dir<small_t && dir>-small_t){
        if(xmin[1]>0.0 || xmax[1]<0.0) return false;
    }
    else{
        dir = 1.0/dir;
        tmin = xmin[1] * dir;
        tmax = xmax[1] * dir;
        if(tmin>tmax){ temp = tmin; tmin = tmax; tmax = temp; }

        if(rmin>tmax || tmin>rmax) return false;

        if(tmin>rmin) rmin = tmin;
        if(tmax<rmax) rmax = tmax;
    }
    dir = ray.direction[2];
    if(dir<small_t && dir>-small_t){
        if(xmin[2]>0.0 || xmax[2]<0.0) return false;
    }
    else{
        dir = 1.0/dir;
        tmin = xmin[2] * dir;
        tmax = xmax[2] * dir;
        if(tmin>tmax){ temp = tmin; tmin = tmax; tmax = temp; }

        if(rmin>tmax || tmin>rmax) return false;
    }
    return true;
}

/*
// Return a list of candidates (indices into the entries list) whose
// bounding boxes intersect the ray.
inline
void Hierarchy::Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) const
{
    if(!tree.size()) return;
    int i, k, size = tree.size(), size_top = entries.size()-1;
    std::queue<int> q;
    if(tree[0].Intersection(ray)) q.push(0);
    //if(debug_pixel && q.empty()) std::cout<<"m"<<std::endl;
    while(!q.empty()){
        i = q.front();
        q.pop();
        k = 2 * i + 1;
        if(k<size){
            if(tree[k].Intersection(ray))
                q.push(k);
            k++;
            if(k<size && tree[k].Intersection(ray))
                q.push(k);
        }
        else{
            candidates.push_back(i-size_top);
        }
    }
}*/