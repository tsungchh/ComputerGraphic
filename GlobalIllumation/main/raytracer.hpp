/**
 * @file raytacer.hpp
 * @brief Raytracer class
 *
 * Implement these functions for project 3.
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#ifndef _462_RAYTRACER_HPP_
#define _462_RAYTRACER_HPP_

#define MAX_DEPTH 5
#define NUM_GLOBAL_MAP 100000
#define NUM_CAUSTIC_MAP 10000
#define NUM_N_GLOBAL 100
#define NUM_N_CAUSTIC 40



#include "math/color.hpp"
#include "math/random462.hpp"
#include "scene/scene.hpp"
#include "KDtree.hpp"


namespace _462 {



// Info for each emitting photon 
class Photon_light {

public:
    Ray r;
    int index;
    Color3 intensity;
    Photon_light(Ray ray, Color3 color, int i) {
        r = ray;
        intensity = color;
        index = i;
    }
};

class Scene;
class Ray;
struct Intersection;
class Raytracer
{
public:

    Raytracer();

    ~Raytracer();

    bool initialize(Scene* scene, size_t num_samples,
                    size_t width, size_t height);

    bool raytrace(unsigned char* buffer, real_t* max_time);


    // ray tracing
    Color3 recursive_raytracing (Ray r, size_t reflectTime); 
    Color3 calDiffuseColor(Vector3 pt, Material_Para material_para);
    Vector3 create_montecarol(Vector3 pt, real_t radius);
    Vector3 create_montecarol_vector();
    Vector3 uniformSampleHemiSphere(const Vector3& normal);
    bool caculate_Refracted_Ray(real_t &R, Material_Para material_para, Ray r, Vector3 &newray);

    // photon mapping
    bool photon_trace(Photon_light p_r, size_t recursion_time, bool caustic_flag);
    Color3 map_color(Ray r, size_t reflectTime, bool caustic_flag);



private:

    Color3 trace_pixel(const Scene* scene,
		       size_t x,
		       size_t y,
		       size_t width,
		       size_t height);

    // the scene to trace
    Scene* scene;

    // the dimensions of the image to trace
    size_t width, height;

    // the next row to raytrace
    size_t current_row;

    unsigned int num_samples;

    //new variables
    Geometry* const* geometries;
    const SphereLight* lights;
    real_t t_max;
    

    // photon mapping
    Photon *global_map;
    Photon *caustic_map;
    size_t num_photons_global;
    size_t num_photons_caustic;
    KDtree global_map_tree;
    KDtree caustic_map_tree;
    real_t shoot_num;
    real_t modified_coe;


};

} /* _462 */

#endif /* _462_RAYTRACER_HPP_ */
