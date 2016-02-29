/**
 * @file model.cpp
 * @brief Model class
 *
 * @author Eric Butler (edbutler)
 * @author Zeyang Li (zeyangl)
 */

#include "scene/model.hpp"
#include "scene/material.hpp"
#include "application/opengl.hpp"
#include "scene/triangle.hpp"
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>


namespace _462 {

Model::Model() : mesh( 0 ), material( 0 ) {}
Model::~Model() { }

void Model::render() const
{
    if ( !mesh )
        return;
    if ( material )
        material->set_gl_state();
    mesh->render();
    if ( material )
        material->reset_gl_state();
}

bool Model::checkIntersection(Ray r, Solution_info &s, const real_t &t_max) {


    face_list = this->mesh->get_triangles();
    vertices_list = this->mesh->get_vertices();

    
    Vector3 e_local = invMat.transform_point(r.e);
    Vector3 d_local = invMat.transform_vector(r.d);

    real_t t_min = t_max;
    real_t beta = -1;
    real_t gamma = -1;
    int index = 0;
    bool hit = false;


    for (size_t i=0; i<this->mesh->num_triangles(); i++) {
        
        Vector3 PointA = vertices_list[face_list[i].vertices[0]].position;
        Vector3 PointB = vertices_list[face_list[i].vertices[1]].position;
        Vector3 PointC = vertices_list[face_list[i].vertices[2]].position;

        real_t a_b_x = PointA.x-PointB.x;
        real_t a_b_y = PointA.y-PointB.y;
        real_t a_b_z = PointA.z-PointB.z;
    
        real_t a_c_x = PointA.x-PointC.x;
        real_t a_c_y = PointA.y-PointC.y;
        real_t a_c_z = PointA.z-PointC.z;

        real_t right_1 = PointA.x - e_local.x;
        real_t right_2 = PointA.y - e_local.y;
        real_t right_3 = PointA.z - e_local.z;
    
        Vector3 v_solution;
        Vector3 v_right(right_1, right_2, right_3);
    
        Matrix3 m(a_b_x, a_c_x, d_local.x,
                  a_b_y, a_c_y, d_local.y,
                  a_b_z, a_c_z, d_local.z);

        Matrix3 minv;
        inverse(&minv, m);

        v_solution = minv*v_right;

        if (v_solution.z<0.0001 || v_solution.z > t_max){
            continue;
        }
        if (v_solution.y<0.0 || v_solution.y>1.0) {
            continue;
        }
        if (v_solution.x<0.0 || v_solution.x>(1.0-v_solution.y)) {
            continue;
        }

        if (v_solution.z < t_min) {
            hit = true;
            t_min = v_solution.z;
            beta = v_solution.x;
            gamma = v_solution.y;
            index = i;
        }
        
    }

    s.t = t_min;
    s.beta = beta;
    s.gamma = gamma;
    s.index = index;

    if (hit == true) {
        return true;
    }else {
        return false;
    }
    

}


Material_Para Model::getMaterial(Ray r, Solution_info s) {

    real_t alpha = 1.0-s.beta-s.gamma;

    
    MeshVertex PointA = vertices_list[face_list[s.index].vertices[0]];
    MeshVertex PointB = vertices_list[face_list[s.index].vertices[1]];
    MeshVertex PointC = vertices_list[face_list[s.index].vertices[2]];
    
    // interpolate the texature 2D coord
    Vector2 tex_coord = PointA.tex_coord*alpha + PointB.tex_coord*s.beta + PointC.tex_coord*s.gamma;
    Material_Para returnPara;
    returnPara.ambient = this->material->ambient;
    returnPara.diffuse = this->material->diffuse;    
    returnPara.specular = this->material->specular;
    returnPara.refractive_index = this->material->refractive_index;

    // texture
    returnPara.texture = this->material->texture_lookup(tex_coord);
    Vector3 local_normal = PointA.normal*alpha + PointB.normal*s.beta + PointC.normal*s.gamma;
    returnPara.normal = normalize(this->normMat*local_normal);  

    return returnPara;  
}


void Model::printname() {

    printf("this is model\n");
}

} /* _462 */
