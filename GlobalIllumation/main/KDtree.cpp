#include "KDtree.hpp"

using namespace std;
namespace _462 {

// Functions used to sort 

bool sort_x(const Photon &a, const Photon &b) {

	return a.position.x < b.position.x;	
}

bool sort_y(const Photon &a, const Photon &b) {

	return a.position.y < b.position.y;	
}

bool sort_z(const Photon &a, const Photon &b) {

	return a.position.z < b.position.z;	
}

bool sort_distance(const Photon &a, const Photon &b) {

	return a.distance < b.distance;
}

KDtree::KDtree(){

	root = NULL;
}

KDtree::~KDtree(){


}

void KDtree::insert_list(Photon map_enter[]) {
	
	map = map_enter;
	// sort it first according to dim_index value
	sort(map, map+num_map, sort_x);
	// use preorder to store nodes into KDtree
	preorder_insert_node(root, 0, num_map-1, 1);
	// printout all the nodes
	//traverse_node(root, 0);

}

Node* KDtree::preorder_insert_node(Node* node, int start, int end, size_t dim_index) {

	// get the mid element, if the total num is even, then take the left of the mid one.
	int num = start + (end-start)/2;
	Node* insert = new Node();
	insert->left = NULL;
	insert->right = NULL;
	insert->data = map[num]; 

	// if this is started from root, insert root first.
	if(root==NULL) {
		root = insert;
	}

	if (start!=end) {
		// insert the left half part
		int mid = num-1;
		
		if (mid>=start) { 

			switch (dim_index%3) {
				case 0:
				sort(map+start, map+mid+1, sort_x); 
				break;
				case 1:
				sort(map+start, map+mid+1, sort_y);
				break;
				case 2:
				sort(map+start, map+mid+1, sort_z);
			}

			insert->left = preorder_insert_node(insert->left, start, num-1, dim_index+1);
		}
		// insert the right half part

		switch(dim_index%3) {
			case 0:
				sort(map+num+1, map+end+1, sort_x); 
				break;
			case 1:
				sort(map+num+1, map+end+1, sort_y);
				break;
			case 2:
				sort(map+num+1, map+end+1, sort_z);
				break;
		}
		insert->right = preorder_insert_node(insert->right, num+1, end, dim_index+1);
	}

	return insert;

}

void KDtree::traverse_node(Node* node, size_t index) {
	if (node !=NULL) {

		//printf("Level %d node = %f %f %f \n", index, node->data.position.x, node->data.position.y, node->data.position.z);
		index++;
		traverse_node(node->left, index);
		traverse_node(node->right, index);
	}
}

Color3 KDtree::calculate_color(Vector3 pt, size_t photon_num) {
	
	Photon photons[photon_num];
	size_t dim = 0;
	int num_index = 0;
    
	// save photons into photons array
	find_node(pt, photons, dim, num_index, root);
	sort(photons, photons+num_index, sort_distance);
	real_t radius = photons[num_index-1].distance;
	Color3 sum(0.0, 0.0, 0.0);

	Color3 color_at_pt = photons[0].materialColor;
	for (size_t i=0; i<photon_num; i++) {
	    sum += photons[i].intensity*color_at_pt*photons[i].dTimesn;	
	}
	Color3 final = sum * (1/(PI*radius*radius));
	return final;

}


void KDtree::find_node(Vector3 pt, Photon photons[], size_t dim, int &num_index, Node* node) {
	
	bool left_flag;
	
	real_t tmp_distance = distance(pt, node->data.position);
	// go back when reaching leave node
	if (node->left==NULL && node->right==NULL) {
		
	    // store photon to array
	    // if the array is not full
	    if (num_index < num_full ) {

		photons[num_index] = node->data;
		photons[num_index].distance = tmp_distance;
		num_index++;
		sort(photons, photons+num_index, sort_distance);

		//if the array is full.
		}else if (tmp_distance<photons[num_index-1].distance) {

		    photons[num_index-1] = node->data;
		    photons[num_index-1].distance = tmp_distance;
		    sort(photons, photons+num_index, sort_distance);
			
		}

		return;

	}else if (node->left!=NULL && node->right==NULL) {

    	    left_flag = true;
	    find_node(pt, photons, dim+1, num_index, node->left);

	}else if (node->right!=NULL && node->left==NULL) {

    	    left_flag = false;
	    find_node(pt, photons, dim+1, num_index, node->right);

	}else {

	    // recursively go to next node
	    switch (dim%3) {
		case 0: 
		    if (pt.x<node->data.position.x) {
			left_flag = true;
			find_node(pt, photons, dim+1, num_index, node->left);
		    }else {
			left_flag = false;
			find_node(pt, photons, dim+1, num_index, node->right);
		    }
		    break;
		case 1: 
		    if (pt.y<node->data.position.y) {
			left_flag = true;
			find_node(pt, photons, dim+1, num_index, node->left);
		    }else {
			left_flag = false;
			find_node(pt, photons, dim+1, num_index, node->right);
		    }
		    break;
		case 2:	
		    if (pt.z<node->data.position.z) {
			left_flag = true;
			find_node(pt, photons, dim+1, num_index, node->left);
		    }else {
			left_flag = false;
			find_node(pt, photons, dim+1, num_index, node->right);
		    }
		    break;
		}
	}

	// decide to go to the other branch 
	real_t nearest_dis;
	switch (dim%3) {
	
	    case 0: 
		nearest_dis = abs(pt.x - node->data.position.x);
		break;
	    case 1:
		nearest_dis = abs(pt.y - node->data.position.y);	
		break;
	    case 2:
		nearest_dis = abs(pt.z - node->data.position.z);
		break;
	}


	if ( (num_index < num_full) || (nearest_dis < photons[num_index-1].distance) ) {

		// go to the other branch
		if (left_flag==true) {
			if (node->right!=NULL) {
				find_node(pt, photons, dim+1, num_index, node->right);
			}
		}else {
			if (node->left!=NULL) {
				find_node(pt, photons, dim+1, num_index, node->left);	
			}
		}

		// store photons after visiting the other branch
		if (num_index < num_full ) {

			photons[num_index] = node->data;
			photons[num_index].distance = tmp_distance;
			num_index++;
			sort(photons, photons+num_index, sort_distance);

		//if the array is full.
		}else if (tmp_distance<photons[num_index-1].distance) {
			
			photons[num_index-1] = node->data;
			photons[num_index-1].distance = tmp_distance;
			sort(photons, photons+num_index, sort_distance);
			
		}
		return;
	}else {
		return;
	}



}




	



}
