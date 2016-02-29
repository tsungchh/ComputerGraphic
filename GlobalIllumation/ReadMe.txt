For this project, I continue to reuse my P3 project code, and add several functions to my P4 project.

Beside original ray-tracing, I add caustic and indirect effect to it, and also use KDtree to implement it.

For photons tracing, I will shoot each photon in random direction, and divide its power by how many photons I shoot out.
When the photon hit diffuse surface from specular object(specular and refractive), then I store it into my caustic map(list).
Otherwise, I store it into my global map(list). Once I finish my shooting part, then I start to work on creating KDtree.
I wrote some sort funcitons to help me create the tree, and also wrote the nearest neighbor lookup function inside the tree class.

Once I finish all these parts, then I can calculate color for each pixel by summing up direct illumination, caustic and indirect part.
I specify the scaling number in trace_pixel function.

For cornell box case, I scale my caustic color by 150, and indirect by 250.
And for cyoinder case, I lower my original ray-tracing part by timing 0.5, and scale caustic color by 100, indirect color by 100.

p.s I need to comment out line 642 and comment 641 in raytracer.cpp for cylinder case. The reasons is because I need to make the outer ring of cylinder case absorb light.
