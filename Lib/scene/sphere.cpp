/**
 * @file sphere.cpp
 * @brief Function defnitions for the Sphere class.
 *
 * @author Kristin Siu (kasiu)
 * @author Eric Butler (edbutler)
 */

#include "scene/sphere.hpp"
#include "application/opengl.hpp"

namespace _462 {

#define SPHERE_NUM_LAT 80
#define SPHERE_NUM_LON 100

#define SPHERE_NUM_VERTICES ( ( SPHERE_NUM_LAT + 1 ) * ( SPHERE_NUM_LON + 1 ) )
#define SPHERE_NUM_INDICES ( 6 * SPHERE_NUM_LAT * SPHERE_NUM_LON )
// index of the x,y sphere where x is lat and y is lon
#define SINDEX(x,y) ((x) * (SPHERE_NUM_LON + 1) + (y))
#define VERTEX_SIZE 8
#define TCOORD_OFFSET 0
#define NORMAL_OFFSET 2
#define VERTEX_OFFSET 5

static unsigned int Indices[SPHERE_NUM_INDICES];
static float Vertices[VERTEX_SIZE * SPHERE_NUM_VERTICES];

static void init_sphere()
{
    static bool initialized = false;
    if ( initialized )
        return;

    for ( int i = 0; i <= SPHERE_NUM_LAT; i++ ) {
        for ( int j = 0; j <= SPHERE_NUM_LON; j++ ) {
            real_t lat = real_t( i ) / SPHERE_NUM_LAT;
            real_t lon = real_t( j ) / SPHERE_NUM_LON;
            float* vptr = &Vertices[VERTEX_SIZE * SINDEX(i,j)];

            vptr[TCOORD_OFFSET + 0] = lon;
            vptr[TCOORD_OFFSET + 1] = 1-lat;

            lat *= PI;
            lon *= 2 * PI;
            real_t sinlat = sin( lat );

            vptr[NORMAL_OFFSET + 0] = vptr[VERTEX_OFFSET + 0] = sinlat * sin( lon );
            vptr[NORMAL_OFFSET + 1] = vptr[VERTEX_OFFSET + 1] = cos( lat ),
            vptr[NORMAL_OFFSET + 2] = vptr[VERTEX_OFFSET + 2] = sinlat * cos( lon );
        }
    }

    for ( int i = 0; i < SPHERE_NUM_LAT; i++ ) {
        for ( int j = 0; j < SPHERE_NUM_LON; j++ ) {
            unsigned int* iptr = &Indices[6 * ( SPHERE_NUM_LON * i + j )];

            unsigned int i00 = SINDEX(i,  j  );
            unsigned int i10 = SINDEX(i+1,j  );
            unsigned int i11 = SINDEX(i+1,j+1);
            unsigned int i01 = SINDEX(i,  j+1);

            iptr[0] = i00;
            iptr[1] = i10;
            iptr[2] = i11;
            iptr[3] = i11;
            iptr[4] = i01;
            iptr[5] = i00;
        }
    }

    initialized = true;
}

Sphere::Sphere()
    : radius(0), material(0) {}

Sphere::~Sphere() {}

void Sphere::render() const
{
    // create geometry if we haven't already
    init_sphere();

    if ( material )
        material->set_gl_state();

    // just scale by radius and draw unit sphere
    glPushMatrix();
    glScaled( radius, radius, radius );
    glInterleavedArrays( GL_T2F_N3F_V3F, VERTEX_SIZE * sizeof Vertices[0], Vertices );
    glDrawElements( GL_TRIANGLES, SPHERE_NUM_INDICES, GL_UNSIGNED_INT, Indices );
    glPopMatrix();

    if ( material )
        material->reset_gl_state();
}

bool Sphere::checkIntersection(Ray r, Solution_info &s, const real_t &t_max) {

  
    Vector3 e_local = invMat.transform_point(r.e);
    Vector3 d_local = invMat.transform_vector(r.d);
    Vector3 c_local = Vector3(0.0, 0.0, 0.0);

    real_t A = dot(d_local, d_local);
    real_t B = dot(d_local, (e_local-c_local));
    real_t B_square = pow(B, 2);
    real_t C = dot(e_local-c_local, e_local-c_local)-(this->radius*this->radius);
    
    real_t discriminant = B_square - A*C; 
    real_t t1;
    real_t t2;
    

    // if there is a solution.
    if (discriminant >= 0) {

        real_t temp = sqrt(discriminant);
        t1 = ( -B - temp ) / A;
        t2 = ( -B + temp ) / A;
  
        //check whether the t1 is inside the range.        
        if(t1>0.000001 && t1<t_max) {
            
            s.t = t1;
            s.index = -1;
            return true;

        //check t2 too.
        }else if (t2>0.00001 && t2<t_max) {
            s.t = t2;
            s.index = -1;
            return true;
        }
    }

    return false;

}

Material_Para Sphere::getMaterial(Ray r, Solution_info s) {

    Vector3 e_local = invMat.transform_point(r.e);
    Vector3 d_local = invMat.transform_vector(r.d);
    Vector3 pt = e_local + d_local*s.t;
    
    
    Material_Para returnPara;
    returnPara.ambient = this->material->ambient;
    returnPara.diffuse = this->material->diffuse;
    returnPara.specular = this->material->specular;
    returnPara.refractive_index = this->material->refractive_index;  

    Vector3 normal = this->normMat * pt;
    returnPara.normal = normalize(normal);

    // compute texture coordinate of sphere 
    real_t theta = acos(normal.z/radius); 
    real_t phi = atan2(normal.y, normal.x); 
    Vector2 tex_coord; 
    tex_coord.x = phi < 0 ? phi/2/PI + 1.0: phi/2/PI; 
    tex_coord.y = 1.0 - theta/PI; 
    returnPara.texture = this->material->texture_lookup(tex_coord);

    return returnPara;
        
}

void Sphere::printname() {

    printf("this is sphere\n");
}


} /* _462 */

