First I will check the ray is intersected with the object or not. If there is intersection, then the geometry object will
return a set of parameters which include the material color, normal, and its texture color.
After checking intersection, I do the recursive raytracing by calling recursive function.

For opaque object, there are directed illumination term and specular term I need to calculate. 
	1. For directed illumination, I will check whether the ray from intersected point to light source is blocked by other geometry object or not. 
	2. For specular term, I just create reflected ray and use it as argument for recursive function calling.

For transparent object, there are specular term and refraction term.

	1. For specular term, I calculate it the same way as I do for opaque one.
	2. For refraction one, I create a refractive ray and also calculate frensal value by calling caculate_Refracted_Ray() in 
	raytracer.cpp.


Besides, I use hermite methods to calculate my texture color.



