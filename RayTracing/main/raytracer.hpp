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

#include "math/color.hpp"
#include "math/random462.hpp"
#include "scene/scene.hpp"

namespace _462 {

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


    /* not yet implemented */

    Color3 recursive_raytracing (Ray r, size_t reflectTime); 
    Color3 calDiffuseColor(Vector3 pt, Material_Para material_para);
    Vector3 create_montecarol(Vector3 pt, real_t radius);
    bool caculate_Refracted_Ray(real_t &R, Material_Para material_para, Ray r, Vector3 &newray);




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
};

} /* _462 */

#endif /* _462_RAYTRACER_HPP_ */
