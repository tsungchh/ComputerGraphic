/**
 * @file model.hpp
 * @brief Model class
 *
 * @author Eric Butler (edbutler)
 */

#ifndef _462_SCENE_MODEL_HPP_
#define _462_SCENE_MODEL_HPP_

#include "scene/scene.hpp"
#include "scene/mesh.hpp"

namespace _462 {

/**
 * A mesh of triangles.
 */
class Model : public Geometry
{
public:

    const Mesh* mesh;
    const Material* material;
    const MeshTriangle* face_list;
    const MeshVertex* vertices_list;


    Model();
    virtual ~Model();

    virtual void render() const;
    
    virtual bool checkIntersection(Ray r, Solution_info &s, const real_t &t_max);
    virtual Material_Para getMaterial(Ray r, Solution_info s);
    virtual void printname();

};


} /* _462 */

#endif /* _462_SCENE_MODEL_HPP_ */

