/**
 * @file sphere.hpp
 * @brief Class defnition for Sphere.
 *
 * @author Kristin Siu (kasiu)
 * @author Eric Butler (edbutler)
 */

#ifndef _462_SCENE_SPHERE_HPP_
#define _462_SCENE_SPHERE_HPP_

#include "scene/scene.hpp"

namespace _462 {

/**
 * A sphere, centered on its position with a certain radius.
 */
class Sphere : public Geometry
{
public:

    real_t radius;
    const Material* material;

    Sphere();
    virtual ~Sphere();
    virtual void render() const;
    virtual bool checkIntersection(Ray r, Solution_info &s, const real_t &t_max);
    virtual Material_Para getMaterial(Ray r, Solution_info s);
    virtual void printname();
};

} /* _462 */

#endif /* _462_SCENE_SPHERE_HPP_ */

