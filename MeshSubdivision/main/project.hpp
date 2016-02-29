#ifndef _462_GEOMETRY_PROJECT_HPP_
#define _462_GEOMETRY_PROJECT_HPP_

#define debug_print printf


#include "math/camera.hpp"
#include "application/opengl.hpp"
#include <vector>
#include <stdio.h>

/*
   A namespace declaration. All project files use this namespace.
   Add this declaration (and its closing) to all source/headers you create.
   Note that all #includes should be BEFORE the namespace declaration.
 */
namespace _462 {

class HE_edge;
class HE_vertex;
class HE_face;

struct Triangle
{
    // index into vertex list of the 3 vertices of this triangle
    unsigned int vertices[3];
};

struct Vertex
{
    // the position of the vertex
    Vector3 position;
    // the normal of the vertex
    Vector3 normal;
    // the texture coordinate of the vertex
    Vector2 texture_coord;
    
    Vertex operator+ (const Vertex &v) const {
        Vertex result;
        result.position = position + v.position;
        result.normal = normal + v.normal;
        result.texture_coord = texture_coord + v.texture_coord; 
        return result;
    }

    Vertex operator*(const double &k) const {
        Vertex result;
        result.position = position * k;
        result.normal = normal * k;
        result.texture_coord = texture_coord * k;
        return result;
    }
};

struct MeshData
{
    // array of vertices
    Vertex* vertices;
    // size of vertex array
    size_t num_vertices;

    // array of triangles
    Triangle* triangles;
    // size of triangle array
    size_t num_triangles;
};

class GeometryProject
{
public:

    // constructor, invoked when object is created
    GeometryProject();
    // destructor, invoked when object is destroyed
    ~GeometryProject();

    // more detailed specifications for each function are in project.cpp.

    // Initialize the project, loading the mesh from the given filename.
    // Returns true on success.
    bool initialize( const Camera* camera, const MeshData* mesh, const char* texture_filename );
    // Clean up the project, free any memory, etc.
    void destroy();
    // Render the mesh using the given camera.
    void render( const Camera* camera );
    // Subdivide the mesh
    void subdivide();
    void setupmesh();
    void createHE_data();
    void firstpass();
    void secondpass();
    void updateMesh();
    void clear_HE();
    void setupmeshtorender();
 
private:

    MeshData mesh;

    size_t num_triangles;
    size_t num_vertices;
    size_t num_helist;
    size_t num_addvertices;
    
    // for rendering
    Vector3* verticeslist;
    Vector3* normallist;
    Vector2* texturelist;
    Triangle* triangleslist;
    GLuint textureNum; 
    GLubyte* RGBImage;


    //for subdivision
    HE_edge* he_edgelist;
    HE_vertex* he_verticeslist;
    HE_face* he_facelist;

    Vertex* addVertexlist;
    Vertex* existedVertexlist;
    Triangle* existedFacelist;
    float frame_material[4];
    
    // since this has no meaningful assignment/copy, prevent the compiler from
    // automatically generating those functions
    GeometryProject( const GeometryProject& );
    GeometryProject& operator=( const GeometryProject& );

};

class HE_edge {
public:
    //save half edge index
    int next_edge;
    int pair_edge;
    //save vertex index
    int startPt;
    int endPt;
    int insertPt;
    
    bool flag;
    
    HE_edge() {
        next_edge = -1;
        pair_edge = -1;
        startPt = -1;
        endPt = -1;
        insertPt = -1;
        flag = false;
    }

};

class HE_vertex {
public:

    Vertex newvertex;

    int he_edge;
    bool isBoundary;

    std::vector<int> search_edge(const HE_edge* list) const {

        std::vector<int> result;
        result.push_back(list[he_edge].endPt);
        int start = he_edge;

        // inner edge and inner edge( but will go to boundary) go in here
        while ((list[start].pair_edge!=-1) && (list[ list[start].pair_edge ].next_edge != he_edge)) {

            start = list[ list[start].pair_edge ].next_edge;
            result.push_back(list[start].endPt);   
        }   
        
        // boundary case
        if (  list[start].pair_edge == -1) {
            
            result.clear();
            result.push_back(list[start].endPt);
            
            while (list[ list[ list[start].next_edge ].next_edge].pair_edge!=-1) {

                start = list[list[ list[start].next_edge ].next_edge].pair_edge;
                result.push_back(list[start].endPt);   
                
            }

            start = list[ list[start].next_edge ].endPt;
            result.push_back(start);                
        }


        return result;
    };  

    HE_vertex() {
        he_edge = -1;
        isBoundary = false;
    }

};


class HE_face {
public:
    int he_edge;
};

} /* _462 */

#endif /* _462_GEOMETRY_PROJECT_HPP_ */

