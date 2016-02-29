/**
 * @file triangle.cpp
 * @brief Function definitions for the Triangle class.
 *
 * @author Eric Butler (edbutler)
 */

#include "scene/triangle.hpp"
#include "application/opengl.hpp"

namespace _462 {

Triangle::Triangle()
{
    vertices[0].material = 0;
    vertices[1].material = 0;
    vertices[2].material = 0;
}

Triangle::~Triangle() { }

void Triangle::render() const
{
    bool materials_nonnull = true;
    for ( int i = 0; i < 3; ++i )
        materials_nonnull = materials_nonnull && vertices[i].material;

    // this doesn't interpolate materials. Ah well.
    if ( materials_nonnull )
        vertices[0].material->set_gl_state();

    glBegin(GL_TRIANGLES);

    glNormal3dv( &vertices[0].normal.x );
    glTexCoord2dv( &vertices[0].tex_coord.x );
    glVertex3dv( &vertices[0].position.x );

    glNormal3dv( &vertices[1].normal.x );
    glTexCoord2dv( &vertices[1].tex_coord.x );
    glVertex3dv( &vertices[1].position.x);

    glNormal3dv( &vertices[2].normal.x );
    glTexCoord2dv( &vertices[2].tex_coord.x );
    glVertex3dv( &vertices[2].position.x);

    glEnd();

    if ( materials_nonnull )
        vertices[0].material->reset_gl_state();
}


bool Triangle::checkIntersection(Ray r, Solution_info &s, const real_t &t_max)
{   

    Vector3 e_local = invMat.transform_point(r.e);
    Vector3 d_local = invMat.transform_vector(r.d);
    
    real_t a_b_x = vertices[0].position.x-vertices[1].position.x;
    real_t a_b_y = vertices[0].position.y-vertices[1].position.y;
    real_t a_b_z = vertices[0].position.z-vertices[1].position.z;
    
    real_t a_c_x = vertices[0].position.x-vertices[2].position.x;
    real_t a_c_y = vertices[0].position.y-vertices[2].position.y;
    real_t a_c_z = vertices[0].position.z-vertices[2].position.z;

    real_t right_1 = vertices[0].position.x - e_local.x;
    real_t right_2 = vertices[0].position.y - e_local.y;
    real_t right_3 = vertices[0].position.z - e_local.z;
    
    Vector3 v_solution;
    Vector3 v_right(right_1, right_2, right_3);
    
    Matrix3 m(a_b_x, a_c_x, d_local.x,
              a_b_y, a_c_y, d_local.y,
              a_b_z, a_c_z, d_local.z);

    Matrix3 minv;
    inverse(&minv, m);

    v_solution = minv*v_right;

    if (v_solution.z<0.000001 || v_solution.z > t_max){
        return false;
    }
    if (v_solution.y<0.0 || v_solution.y>1.0) {
        return false;
    }
    if (v_solution.x<0.0 || v_solution.x>(1.0-v_solution.y)) {
        return false;
    }

    s.t = v_solution.z;
    s.beta = v_solution.x;
    s.gamma = v_solution.y;
    s.index = -1;

    return true;
}


Material_Para Triangle::getMaterial(Ray r, Solution_info s) {

    Vector3 e_local = invMat.transform_point(r.e);
    Vector3 d_local = invMat.transform_vector(r.d);
    
    real_t alpha = 1.0-s.beta-s.gamma;
    
    // interpolate the texature 2D coord
    Vector2 tex_coord = this->vertices[0].tex_coord*alpha + this->vertices[1].tex_coord*s.beta + this->vertices[2].tex_coord*s.gamma;


    Material_Para returnPara;
    returnPara.ambient = this->vertices[0].material->ambient*alpha + this->vertices[1].material->ambient*s.beta + this->vertices[2].material->ambient*s.gamma;
    returnPara.diffuse = this->vertices[0].material->diffuse*alpha + this->vertices[1].material->diffuse*s.beta + this->vertices[2].material->diffuse*s.gamma;
    returnPara.specular = this->vertices[0].material->specular*alpha + this->vertices[1].material->specular*s.beta + this->vertices[2].material->specular*s.gamma;
    returnPara.refractive_index = this->vertices[0].material->refractive_index*alpha + this->vertices[1].material->refractive_index*s.beta + this->vertices[2].material->refractive_index*s.gamma;  
    
    // texture
    returnPara.texture = vertices[0].material->texture_lookup(tex_coord)*alpha
                       + this->vertices[1].material->texture_lookup(tex_coord)*s.beta
                       + this->vertices[2].material->texture_lookup(tex_coord)*s.gamma;

    Vector3 local_normal = this->vertices[0].normal*alpha + this->vertices[1].normal*s.beta + this->vertices[2].normal*s.gamma;
    returnPara.normal = normalize(this->normMat*local_normal);  


    return returnPara;  
}

void Triangle::printname() {

    printf("this is triangle\n");
}

} /* _462 */
