#include "project.hpp"
#include "application/opengl.hpp"
#include "application/imageio.hpp"

#include <iostream>
#include <stdio.h>
using namespace std;
/*
   A namespace declaration. All project files use this namespace.
   Add this declaration (and its closing) to all source/headers you create.
   Note that all #includes should be BEFORE the namespace declaration.
 */
namespace _462 {

// definitions of functions for the GeometryProject class

// constructor, invoked when object is allocated
GeometryProject::GeometryProject() { 
	
    he_verticeslist = NULL;
    he_edgelist = NULL;
    he_facelist = NULL;
    existedVertexlist = NULL;
    existedFacelist = NULL;
    addVertexlist = NULL;
    RGBImage = NULL;

}

// destructor, invoked when object is de-allocated
GeometryProject::~GeometryProject() { }

/**
 * Initialize the project, doing any necessary opengl initialization.
 * @param camera An already-initialized camera.
 * @param mesh The mesh to be rendered and subdivided.
 * @param texture_filename The filename of the texture to use with the mesh.
 *  Is null if there is no texture data with the mesh or no texture filename
 *  was passed in the arguments, in which case textures should not be used.
 * @return true on success, false on error.
 */
bool GeometryProject::initialize( const Camera* camera, const MeshData* mesh, 
                                  const char* texture_filename )
{
    this->mesh = *mesh;

    // Light and material part
    
    GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat ambient[] = { 0.5, 0.5, 0.5};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    

    GLfloat light_position1[] = {1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    glClearColor(0.0, 0.0 ,0.0, 0.0);
    glShadeModel(GL_SMOOTH);

    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {50.0};
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);



    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    frame_material[0]=1.0;
    frame_material[1]=0.0;
    frame_material[2]=0.0;
    frame_material[3]=1.0;


    // Texture mesh

    if (texture_filename != NULL) {
    	
        cout<<texture_filename<<endl;

        int width=-1, height=-1;
	    RGBImage = imageio_load_image(texture_filename, &width, &height);
	    
	    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glGenTextures(1, &textureNum);
        glBindTexture(GL_TEXTURE_2D, textureNum);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, RGBImage);
    }
	
    this->setupmesh();    
    return true;
}

/**
 * Clean up the project. Free any memory, etc.
 */
void GeometryProject::destroy()
{

    if (verticeslist != NULL) {
	delete [] verticeslist;
	verticeslist = NULL;
    }

    if (normallist != NULL) {
	delete [] normallist;
	normallist = NULL;
    }

    if (texturelist != NULL) {
	delete [] texturelist;
	texturelist = NULL;
    }

    if (triangleslist != NULL) {
	delete [] triangleslist;
	triangleslist = NULL;
    }

    if (RGBImage != NULL) {
	delete [] RGBImage;
	RGBImage = NULL;
    }

    if (existedFacelist != NULL) {
	delete [] existedFacelist;
	existedFacelist = NULL;
    }

    if (existedVertexlist != NULL) {
	delete [] existedVertexlist;
	existedVertexlist = NULL;
    }

    if (addVertexlist != NULL) {
	delete [] addVertexlist;
	addVertexlist = NULL;
    }


}

/**
 * Clear the screen, then render the mesh using the given camera.
 * @param camera The logical camera to use.
 * @see scene/camera.hpp
 */
void GeometryProject::render( const Camera* camera )
{
    // clear buffer
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
  // camera setting - projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera->get_fov_degrees(), camera->get_aspect_ratio(), 
                   camera->get_near_clip(), camera->get_far_clip());

    
  // camera setting - model view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera->get_position().x, camera->get_position().y, 
              camera->get_position().z, 
              camera->get_position().x + camera->get_direction().x, 
              camera->get_direction().y+camera->get_position().y, 
              camera->get_direction().z + camera->get_position().z, 
              camera->get_up().x, camera->get_up().y, camera->get_up().z);
	
	 
  // texture binding	 
    glBindTexture(GL_TEXTURE_2D, textureNum);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);

  // draw triangulars
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(0.8f, 0.0f, 0.0f, 1.0);
    glVertexPointer(3, GL_DOUBLE, 0 , verticeslist);

    glNormalPointer(GL_DOUBLE, 0, normallist);
    glTexCoordPointer(2, GL_DOUBLE, 0, texturelist);
    glDrawElements( GL_TRIANGLES, 3*num_triangles, GL_UNSIGNED_INT, triangleslist);
  
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    
}


/**
 * Subdivide the mesh that we are rendering using Loop subdivison.
 */
void GeometryProject::subdivide()
{
    createHE_data();
    firstpass();
    secondpass();
    updateMesh();
}

void GeometryProject::setupmesh() {

    num_triangles = mesh.num_triangles;
    num_vertices = mesh.num_vertices;
    verticeslist = new Vector3[num_vertices];
    normallist = new Vector3[num_vertices];
    texturelist = new Vector2[num_vertices];
    triangleslist = new Triangle[num_triangles];

    //for subdivision part
    existedVertexlist = new Vertex[num_vertices];
    existedFacelist = new Triangle[num_triangles];
	
    for (unsigned int i=0; i<mesh.num_vertices; i++) {
	
	verticeslist[i].x = existedVertexlist[i].position.x 
                          = mesh.vertices[i].position.x;
	verticeslist[i].y = existedVertexlist[i].position.y 
                          = mesh.vertices[i].position.y;
	verticeslist[i].z = existedVertexlist[i].position.z 
                          = mesh.vertices[i].position.z;

	normallist[i].x = existedVertexlist[i].normal.x 
                        = mesh.vertices[i].normal.x;
	normallist[i].y = existedVertexlist[i].normal.y 
                        = mesh.vertices[i].normal.y;
	normallist[i].z = existedVertexlist[i].normal.z 
                        = mesh.vertices[i].normal.z;
		
	texturelist[i].x = existedVertexlist[i].texture_coord.x 
                         = mesh.vertices[i].texture_coord.x;
	texturelist[i].y = existedVertexlist[i].texture_coord.y 
                         = mesh.vertices[i].texture_coord.y;
    }

    for (unsigned int i=0; i<mesh.num_triangles; i++) {
	
	triangleslist[i].vertices[0] = existedFacelist[i].vertices[0] 
                                     = mesh.triangles[i].vertices[0];
	triangleslist[i].vertices[1] = existedFacelist[i].vertices[1] 
                                     = mesh.triangles[i].vertices[1];
	triangleslist[i].vertices[2] = existedFacelist[i].vertices[2] 
                                     = mesh.triangles[i].vertices[2];
    }
}

void GeometryProject::setupmeshtorender() {

    verticeslist = new Vector3[num_vertices];
    normallist = new Vector3[num_vertices];
    texturelist = new Vector2[num_vertices];
    triangleslist = new Triangle[num_triangles];
	
    for (unsigned int i=0; i<num_vertices; i++) {		
	verticeslist[i].x = existedVertexlist[i].position.x;
	verticeslist[i].y = existedVertexlist[i].position.y;
	verticeslist[i].z = existedVertexlist[i].position.z;

	normallist[i].x = existedVertexlist[i].normal.x;
	normallist[i].y = existedVertexlist[i].normal.y;
	normallist[i].z = existedVertexlist[i].normal.z ;
		
	texturelist[i].x = existedVertexlist[i].texture_coord.x;
	texturelist[i].y = existedVertexlist[i].texture_coord.y;
    }
	
    for (unsigned int i=0; i<num_triangles; i++) {
	triangleslist[i].vertices[0] = existedFacelist[i].vertices[0];
	triangleslist[i].vertices[1] = existedFacelist[i].vertices[1];
	triangleslist[i].vertices[2] = existedFacelist[i].vertices[2];	
    }
}

void GeometryProject::createHE_data() {

    clear_HE();        
    num_helist = 3*num_triangles;
    he_edgelist = new HE_edge[num_helist];
    he_verticeslist = new HE_vertex[num_vertices];
    he_facelist = new HE_face[num_triangles];
    
    for (size_t i=0; i<num_triangles; i++) {
    	int vertex_1, vertex_2, vertex_3;
    	vertex_1 = existedFacelist[i].vertices[0];
    	vertex_2 = existedFacelist[i].vertices[1];
    	vertex_3 = existedFacelist[i].vertices[2];
    	
    	HE_edge edge_1; // 3i
		HE_edge edge_2; // 3i+1
    	HE_edge edge_3; // 3i+2

    	edge_1.next_edge = 3*i + 1;
		edge_2.next_edge = 3*i + 2;
    	edge_3.next_edge = 3*i;

    	edge_1.startPt = vertex_1;
    	edge_1.endPt = vertex_2;
    	edge_2.startPt = vertex_2;
    	edge_2.endPt = vertex_3;
    	edge_3.startPt = vertex_3;
    	edge_3.endPt = vertex_1;
  		
  	he_edgelist[3*i] = edge_1;
  	he_edgelist[3*i+1] = edge_2;
  	he_edgelist[3*i+2] = edge_3;

  	// setup vertices list

  	he_verticeslist[vertex_1].he_edge = 3*i;
  	he_verticeslist[vertex_2].he_edge = 3*i+1;
  	he_verticeslist[vertex_3].he_edge = 3*i+2;

  	// setup facelist

  	he_facelist[i].he_edge = 3*i; 
  		
    }

    // search the edge pair of every edge
    size_t boundarycount = 0;
    size_t inneredgecount = 0;

    for (size_t i=0; i<num_helist; i++) {

    	// search whenever the pair has not been set yet.
    	if (he_edgelist[i].pair_edge == -1) {

	    int startPt = he_edgelist[i].startPt;
    	    int endPt = he_edgelist[i].endPt;
	    	
	    for (size_t j=i+1; j<num_helist; j++) {

	    	if (he_edgelist[j].endPt == startPt && he_edgelist[j].startPt == endPt) {
	    	    he_edgelist[i].pair_edge = j;
	    	    he_edgelist[j].pair_edge = i;
	    	    inneredgecount++;
	    	}

	    }

	    //boundary edge
	    if(he_edgelist[i].pair_edge == -1) {
		he_verticeslist[he_edgelist[i].startPt].isBoundary = true;
    		he_verticeslist[he_edgelist[i].endPt].isBoundary = true;
	    	boundarycount++;
	    }
    	}	
    }
    
    num_addvertices = inneredgecount + boundarycount;
    addVertexlist = new Vertex[num_addvertices];
}


void GeometryProject::firstpass() {

    int num_insertPt = 0;
    for (size_t i=0; i<num_helist; i++) {

	// not yet insert 
	if (he_edgelist[i].flag == false) {
			
	    Vertex insert;

	    // Is boundary if pair = -1.
	    if (he_edgelist[i].pair_edge == -1) {

		Vertex pt1 = existedVertexlist[he_edgelist[i].startPt];
		Vertex pt2 = existedVertexlist[he_edgelist[i].endPt];
		insert = pt1*0.5 + pt2*0.5;
		insert.normal = normalize(insert.normal);
		addVertexlist[num_insertPt] = insert;
		he_edgelist[i].insertPt = num_vertices+num_insertPt;
		he_edgelist[i].flag = true;
		num_insertPt++;
				
		// Inner case 
	    }else {

		Vertex pt1 = existedVertexlist[he_edgelist[i].startPt];
		Vertex pt2 = existedVertexlist[he_edgelist[i].endPt];
		int next_ed = he_edgelist[i].next_edge;
		int next_pair_ed = he_edgelist[he_edgelist[i].pair_edge].next_edge;
		Vertex pt3 = existedVertexlist[he_edgelist[next_ed].endPt];
		Vertex pt4 = existedVertexlist[he_edgelist[next_pair_ed].endPt];
	
		insert = pt1*0.375 + pt2*0.375 + pt3*0.125 + pt4*0.125;
		insert.normal = normalize(insert.normal);
		addVertexlist[num_insertPt] = insert;
		he_edgelist[i].insertPt = num_vertices+num_insertPt;
		he_edgelist[i].flag = true;
		he_edgelist[he_edgelist[i].pair_edge].insertPt = num_vertices+num_insertPt;				
		he_edgelist[he_edgelist[i].pair_edge].flag = true;				
		num_insertPt++;	
	     }
	}
    }	
}


void GeometryProject::secondpass() {


    for (size_t i=0; i<num_vertices; i++) {

	Vertex modifiedPt;

	// search edge which are adjacent to this vertex
	std::vector<int> adjacent_edge = he_verticeslist[i].search_edge(he_edgelist);
		
	// boundary case
	if (he_verticeslist[i].isBoundary == true) {

	    modifiedPt = existedVertexlist[i]*0.75 + 
                         existedVertexlist[adjacent_edge[0]]*0.125 + 
                         existedVertexlist[adjacent_edge[adjacent_edge.size()-1]]*0.125;
	    he_verticeslist[i].newvertex = modifiedPt;
	    // inner case
	}else {

	    double numofedge = adjacent_edge.size();
	    double beta = (5.0/8.0 - 
                          (3.0/8.0 + cos((PI*2.0)/numofedge)/4.0)*(3.0/8.0 + 
                           cos((PI*2.0)/numofedge)/4.0)) / numofedge;

	    modifiedPt = existedVertexlist[i] * (1.0 - beta*numofedge);
	    for (unsigned int j=0; j<adjacent_edge.size(); j++) {
		modifiedPt = modifiedPt + (existedVertexlist[adjacent_edge[j]] * beta);
	    }
	    he_verticeslist[i].newvertex = modifiedPt;
	}
    }

    for (size_t i=0; i<num_vertices; i++) {
	existedVertexlist[i] = he_verticeslist[i].newvertex;
	existedVertexlist[i].normal = normalize(existedVertexlist[i].normal);
    }	
}

void GeometryProject::updateMesh() {

	
    // update vertices information
    Vertex* temp = new Vertex[num_vertices + num_addvertices];
    for(size_t i=0; i<num_vertices; i++) {
	temp[i] = existedVertexlist[i];
    } 
    for(size_t i=0; i<num_addvertices; i++) {	
	temp[num_vertices+i] = addVertexlist[i];
    }
    delete [] existedVertexlist;
    existedVertexlist = NULL;
    delete [] addVertexlist;
    addVertexlist = NULL;
    num_vertices = num_vertices+ num_addvertices;
    existedVertexlist = temp;

    // update faces information
 	
    Triangle* temp_face = new Triangle[4*num_triangles];
    for (size_t i=0 ; i<num_triangles; i++) {

	int edge_1 = he_facelist[i].he_edge;
	int edge_2 = he_edgelist[edge_1].next_edge;
	int edge_3 = he_edgelist[edge_2].next_edge;

	// insert face 1
	temp_face[4*i].vertices[0] = he_edgelist[edge_1].startPt;
	temp_face[4*i].vertices[1] = he_edgelist[edge_1].insertPt;
	temp_face[4*i].vertices[2] = he_edgelist[edge_3].insertPt;

	// insert face 2
	temp_face[4*i+1].vertices[0] = he_edgelist[edge_1].insertPt;
	temp_face[4*i+1].vertices[1] = he_edgelist[edge_1].endPt;
	temp_face[4*i+1].vertices[2] = he_edgelist[edge_2].insertPt;

	// insert face 3
	temp_face[4*i+2].vertices[0] = he_edgelist[edge_3].insertPt;
	temp_face[4*i+2].vertices[1] = he_edgelist[edge_1].insertPt;
	temp_face[4*i+2].vertices[2] = he_edgelist[edge_2].insertPt;

	// insert face 4
	temp_face[4*i+3].vertices[0] = he_edgelist[edge_3].insertPt;
	temp_face[4*i+3].vertices[1] = he_edgelist[edge_2].insertPt;
	temp_face[4*i+3].vertices[2] = he_edgelist[edge_2].endPt;
	
    }

    delete [] existedFacelist;
    existedFacelist = NULL;
    existedFacelist = temp_face;
    num_triangles = 4*num_triangles;

    // ** update mesh information used to render
    setupmeshtorender();
	
}


void GeometryProject::clear_HE() {
    if (he_edgelist != NULL) {
	delete [] he_edgelist;
	he_edgelist = NULL;
    }

    if (he_verticeslist != NULL) {
	delete [] he_verticeslist;
	he_verticeslist = NULL;
    }

    if (he_facelist != NULL) {
	delete [] he_facelist;
	he_facelist = NULL;		
    }
}
} /* _462 */

