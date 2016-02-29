For this project, I use half edge as my data structure and implements it with loop algorithm.

So for my half edge data structure, basically I create three clase, which are HE_edge, HE_vertex and HE_face.

    1. HE_edge
	This guy basically represent half edge of my data structure, which have the following members.
	   
	    1. next edge
	       Speicify which edge is the next edge to refer to.
	    2. pair_edge
	       Indicate which edge is the pair edge of this edge.
	    3. start_pt
	       Specify the start point index of this edge.
            4. end_pt
               Specify the end point index of this edge.
            5. insert_pt
               Specify the point which will be inserted during firstpass method(odd process)
            6. flag
               Used for me to indicate whether this edge has been checked when I try to insert point.

    2. HE_vertex
	This class will have information for every vertices of this mesh.

            1. newvertex
               Specify the new vertex information(position, normal, and text_coord) 
            2. he_edge
               Specify which half edge it connect to.
            3. isBoundary
               Specify whether this vertex is on boundary or not.
            4. serch_edge method
               This method will search for the adjacent edges for this vertex.

    3. HE_face
       This class will save the half edge that contains this face.

The process I used to subdive the object is that I will create half edge data structure by using the mesh information first.
Then I will use the half edge data structure to do the subdivision, which is the firstpass method and secondpass method.
After subdivition, then I will update mesh information according to the information of my half edge data structure.
Then I will repeat again, to create half edge data structure again...to iteratively do the subdivision.


