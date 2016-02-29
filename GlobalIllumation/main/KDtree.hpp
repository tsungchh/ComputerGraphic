#ifndef _462_KDTREE_HPP_
#define _462_KDTREE_HPP_


//#include "raytracer.hpp"
#include <algorithm>

#include "math/color.hpp"
#include "scene/scene.hpp"

namespace _462 {


struct Photon {
    
    //Vector3 direction;
    Vector3 position;
    Color3 intensity;
    Color3 materialColor;
    real_t distance;
    real_t dTimesn;

};

struct Node {
	Node* left;
	Node* right;
	Photon data;
};


class KDtree {
public:

	size_t num_map;
	int num_full;
	Node* root;
	Photon* map;

	KDtree();
	~KDtree();
	void insert_list(Photon map[]);
	Node* preorder_insert_node(Node* node, int start, int end, size_t dim_index);
	Color3 calculate_color(Vector3 pt, size_t photon_num);
	void traverse_node(Node* node, size_t index);
	void find_node(Vector3 pt, Photon photons[], size_t dim, int &num_index, Node* node);

};

}

#endif /* _462_KDTREE_ */

