/**
 * @file raytacer.cpp
 * @brief Raytracer class
 *
 * Implement these functions for project 4.
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#include "raytracer.hpp"
#include "scene/scene.hpp"
#include "KDtree.hpp"

#include <SDL_timer.h>
#include <iostream>
#include <random>

#ifdef OPENMP // just a defense in case OpenMP is not installed.

#include <omp.h>

#endif
namespace _462 {

// max number of threads OpenMP can use. Change this if you like.
#define MAX_THREADS 8
#define MONTE_CAROL_TIMES 10



static const unsigned STEP_SIZE = 8;

Raytracer::Raytracer()
    : scene(0), width(0), height(0) { }

// random real_t in [0, 1)
static inline real_t random()
{
    return real_t(rand())/RAND_MAX;
}

Raytracer::~Raytracer() { }

/**
 * Initializes the raytracer for the given scene. Overrides any previous
 * initializations. May be invoked before a previous raytrace completes.
 * @param scene The scene to raytrace.
 * @param width The width of the image being raytraced.
 * @param height The height of the image being raytraced.
 * @return true on success, false on error. The raytrace will abort if
 *  false is returned.
 */
bool Raytracer::initialize(Scene* scene, size_t num_samples,
               size_t width, size_t height)
{

    /*
     * omp_set_num_threads sets the maximum number of threads OpenMP will
     * use at once.
     */
#ifdef OPENMP
    omp_set_num_threads(MAX_THREADS);
#endif
    this->scene = scene;
    this->num_samples = num_samples;    
    this->width = width;
    this->height = height;

    current_row = 0;

    Ray::init(scene->camera);
    scene->initialize();

\

    // new by tsungchh
    geometries = scene->get_geometries();
    this->lights = scene->get_lights();
    t_max = scene->camera.get_far_clip();

    // new for photons mapping.	
    global_map = new Photon[NUM_GLOBAL_MAP];
    caustic_map = new Photon[NUM_CAUSTIC_MAP];

    int numoflights = scene->num_lights();
    global_map_tree.num_map = NUM_GLOBAL_MAP;
    global_map_tree.num_full = NUM_N_GLOBAL;
    caustic_map_tree.num_map = NUM_CAUSTIC_MAP;
    caustic_map_tree.num_full = NUM_N_CAUSTIC;
    num_photons_global = 0;
    num_photons_caustic = 0;
    int i = 0;


    // global mapping
    while ((num_photons_global < NUM_GLOBAL_MAP) || (num_photons_caustic < NUM_CAUSTIC_MAP)) {    
            
        // create random photons ray
        int i_index = i % numoflights;  // get different lights
        
        Vector3 lights_position;
        Vector3 d;
        Ray r;
        Color3 intensity = lights[i_index].color;


        if (lights[i_index].radius!=0) {
            lights_position = create_montecarol(lights[i_index].position, lights[i_index].radius);
            d = normalize(lights_position - lights[i_index].position);
            r.e = lights_position;
            r.d = d;
        }else {
            lights_position = lights[i_index].position;
            r.e = lights_position;
            r.d = create_montecarol_vector();
            
        }
        Photon_light p_r(r, intensity, i_index);

        // emit photon ray 
        photon_trace(p_r, 0, false);
        i++;
    }

    shoot_num = (real_t)i;
    modified_coe = 1.0f/shoot_num;
    global_map_tree.insert_list(global_map);
    caustic_map_tree.insert_list(caustic_map);


    return true;
}

bool Raytracer::photon_trace(Photon_light p_r, size_t recursion_time, bool caustic_flag) {
    
    
    size_t const recursion_limit = 5;

    if (recursion_time > recursion_limit) {
        return true;
    }
    recursion_time++;

    // loop search for the smallest t and its index. 
    Ray r = p_r.r;
    bool hit_flag = false;
    Solution_info s;
    Solution_info s_min;
    s_min.t = t_max;
    real_t R; //frensal

    int geometry_index = 0;    
    for (size_t i=0; i<scene->num_geometries(); i++) {

        if (geometries[i]->checkIntersection(r, s, t_max)==true) {
            hit_flag = true;        
            if(s.t < s_min.t) {
                s_min.t = s.t;
                s_min.beta = s.beta;
                s_min.gamma = s.gamma;
                s_min.index = s.index;
                geometry_index = i;
            }
        }
    }
    

    // Caculate the color

    Color3 diffuse_Color(0.0, 0.0, 0.0);
    Color3 direct_Color(0.0, 0.0, 0.0);

    if (hit_flag == true) {

        // Get the intesect point
        Vector3 inter_Pt = r.e + s_min.t*r.d;
        // Get the geometry property
        Material_Para material_para = geometries[geometry_index]->getMaterial(r, s_min);
 
        if (material_para.diffuse != Color3::Black() && material_para.refractive_index == 0 ) {
         
	    Vector3 d = normalize(p_r.r.d);
            diffuse_Color = p_r.intensity*material_para.diffuse;
            direct_Color = diffuse_Color*material_para.texture;   

            // store photon into caustic map list and global map list
            if (recursion_time>1) { 
                Photon p;
                p.dTimesn = dot(-d, material_para.normal);
                p.position = inter_Pt;
                p.intensity = p_r.intensity;
                p.materialColor = material_para.diffuse;


                if (caustic_flag!=true && num_photons_global<NUM_GLOBAL_MAP) {
                    global_map[num_photons_global] = p; 
                    num_photons_global++;
                }

                if (caustic_flag==true && num_photons_caustic<NUM_CAUSTIC_MAP) {
                    caustic_map[num_photons_caustic] = p;
                    num_photons_caustic++;
                }
            }
    
            // create random_ray
            Ray random_ray(inter_Pt, create_montecarol_vector());
            Photon_light p_r_diffuse(random_ray, direct_Color, p_r.index);
             
            // emit another photon light, random direction
            photon_trace(p_r_diffuse, recursion_time, caustic_flag);
    

            return true;

        // specular object    
        }else if(material_para.specular != Color3::Black() && material_para.refractive_index == 0) {
    
            real_t random_num = random_uniform();
            real_t r_tmp = material_para.specular.r;
            real_t g_tmp = material_para.specular.g;
            real_t b_tmp = material_para.specular.b;
            real_t max_RGB;
            if( r_tmp > g_tmp) {
                if(r_tmp>b_tmp) {
                    max_RGB = r_tmp;
                }else {
                    max_RGB = b_tmp;
                }
            }else {
                if(g_tmp>b_tmp) {
                    max_RGB = g_tmp;
                }else {
                    max_RGB = b_tmp;
                }
            }

            if(random_num < max_RGB) {
    
                Vector3 newray_direction = r.d - 2*dot(material_para.normal, r.d)*material_para.normal;
                newray_direction = normalize(newray_direction); 
                Ray newray(inter_Pt, newray_direction);

                Photon_light p_r_specular(newray, p_r.intensity*material_para.specular, p_r.index);
                photon_trace(p_r_specular, recursion_time, true);

            }

            return true;

	// refract object
        }else {
    
            Vector3 refract_direction;
            Ray refract_ray;
            // check whether the full internal reflection happen or not.
            bool tir = caculate_Refracted_Ray(R, material_para, r, refract_direction) ;
            if ( tir ) {
                refract_ray.e = inter_Pt;
                refract_ray.d = refract_direction;      
            } else {
                R = 1;
            }
            

            // specular direction

            Vector3 newray_direction = r.d - 2*dot(material_para.normal, r.d)*material_para.normal;
            newray_direction = normalize(newray_direction); 
            Ray newray(inter_Pt, newray_direction);


            // use Russian roulette to decide which photon shouble emit 
                
            real_t random_num = random_uniform();
            if (random_num < R) {
                // emmit reflected ray

                Photon_light p_r_reflect(newray, p_r.intensity, p_r.index);
                photon_trace(p_r_reflect, recursion_time, true);
            }else {
                // emit refracted ray
               Photon_light p_r_refract(refract_ray, p_r.intensity, p_r.index);
                photon_trace(p_r_refract, recursion_time, true);
            }
        }
        return true;
    }else {
        return false;
    }



}

Vector3 Raytracer::uniformSampleHemiSphere(const Vector3& normal) {

    Vector3 newDir = create_montecarol_vector();
    if (dot(newDir, normal)<0) {
        newDir = -newDir;
    }
    return newDir;

}

Color3 Raytracer::map_color(Ray r, size_t reflectTime, bool caustic_flag){
    
    size_t const recursion_limit = 5;
    if (reflectTime > recursion_limit) {
        return Color3::Black();
    }
    reflectTime++;
    
    bool hit_flag = false;
    Solution_info s;
    Solution_info s_min;
    s_min.t = t_max;
    real_t R; //frensal

    int geometry_index = 0;    
    for (size_t i=0; i<scene->num_geometries(); i++) {

        if (geometries[i]->checkIntersection(r, s, t_max)==true) {
            hit_flag = true;        
            if(s.t < s_min.t) {
                s_min.t = s.t;
                s_min.beta = s.beta;
                s_min.gamma = s.gamma;
                s_min.index = s.index;
                geometry_index = i;
            }
        }
    }
    
    if (hit_flag == true) {

        //Get the intesect point
        Vector3 inter_Pt = r.e + s_min.t*r.d;
        // Get the geometry property
        Material_Para material_para = geometries[geometry_index]->getMaterial(r, s_min);
        
        if (material_para.diffuse != Color3::Black() && material_para.refractive_index == 0 ) {
            if (caustic_flag) {
                Color3 tmp_1 = caustic_map_tree.calculate_color(inter_Pt, NUM_N_CAUSTIC)*modified_coe;    
                
                return tmp_1;
            }else {
                Color3 tmp_2 = global_map_tree.calculate_color(inter_Pt, NUM_N_GLOBAL)*modified_coe;    
         
                return tmp_2;    
            }    
        }else if (material_para.specular != Color3::Black() && material_para.refractive_index == 0) {
            
            // calculate r 
            Vector3 newray_direction = r.d - 2*dot(material_para.normal, r.d)*material_para.normal;
            newray_direction = normalize(newray_direction); 
            Ray newray(inter_Pt, newray_direction);

            return map_color(newray , reflectTime, caustic_flag);

    
        }else if (material_para.refractive_index != 0) {

            Vector3 refract_direction;
            Ray refract_ray;
            // check whether the full internal reflection happen or not.
            bool tir = caculate_Refracted_Ray(R, material_para, r, refract_direction) ;
            if ( tir ) {
                refract_ray.e = inter_Pt;
                refract_ray.d = refract_direction;      
            } else {
                R = 1;
            }
            
            Color3 refract_Color = map_color(refract_ray, reflectTime, caustic_flag);
            // specular direction

            Vector3 newray_direction = r.d - 2*dot(material_para.normal, r.d)*material_para.normal;
            newray_direction = normalize(newray_direction); 
            Ray newray(inter_Pt, newray_direction);
            Color3 reflect_Color = map_color(newray, reflectTime, caustic_flag);
            return R*reflect_Color + (1-R)*refract_Color;

        }

    }else {

        return scene->background_color;
    }
    return scene->background_color;
}


/**
 * Performs a raytrace on the given pixel on the current scene.
 * The pixel is relative to the bottom-left corner of the image.
 * @param scene The scene to trace.
 * @param x The x-coordinate of the pixel to trace.
 * @param y The y-coordinate of the pixel to trace.
 * @param width The width of the screen in pixels.
 * @param height The height of the screen in pixels.
 * @return The color of that pixel in the final image.
 */
Color3 Raytracer::trace_pixel(const Scene* scene,
                  size_t x,
                  size_t y,
                  size_t width,
                  size_t height)
{
    assert(x < width);
    assert(y < height);

    real_t dx = real_t(1)/width;
    real_t dy = real_t(1)/height;

    Color3 res = Color3::Black();
    unsigned int iter;

    for (iter = 0; iter < num_samples; iter++)
    {
        // pick a point within the pixel boundaries to fire our
        // ray through.
        real_t i = real_t(2)*(real_t(x)+random())*dx - real_t(1);
        real_t j = real_t(2)*(real_t(y)+random())*dy - real_t(1);
        Ray r = Ray(scene->camera.get_position(), Ray::get_pixel_dir(i, j));

        // for directed illumination and specular
        res += 0.6*recursive_raytracing(r, 0);
	// caustic effect
        res += 60*map_color(r, 0, true);
	// indirect effect
        res += 150*map_color(r, 0, false);


    }
    return res*(real_t(1)/num_samples);
}

Vector3 Raytracer::create_montecarol(Vector3 pt, real_t radius) {

    
    real_t x = random_gaussian();
    real_t y = random_gaussian();
    real_t z = random_gaussian();
    
    real_t x_new = x / sqrt(x*x + y*y + z*z);
    real_t y_new = y / sqrt(x*x + y*y + z*z);
    real_t z_new = z / sqrt(x*x + y*y + z*z);
    
    Vector3 v(x_new, y_new, z_new);
    v = v*radius + pt;

    return v;
}

Vector3 Raytracer::create_montecarol_vector() {

    
    real_t x = random_gaussian();
    real_t y = random_gaussian();
    real_t z = random_gaussian();
    
    real_t x_new = x / sqrt(x*x + y*y + z*z);
    real_t y_new = y / sqrt(x*x + y*y + z*z);
    real_t z_new = z / sqrt(x*x + y*y + z*z);
    
    Vector3 v(x_new, y_new, z_new);

    return v;
}

Color3 Raytracer::calDiffuseColor(Vector3 pt, Material_Para material_para) {

    Color3 diffuse_Color(0.0, 0.0, 0.0);
    

    // for every light
    for(size_t i=0; i<scene->num_lights(); i++) {

        Color3 diffuse_Color_per(0.0, 0.0, 0.0);

        // create soft shadow(use monte carol)
    
        if ( lights[i].radius == 0) {  // which mean it is a point light src.
            
                Vector3 lights_position = lights[i].position;
                Vector3 d = normalize(lights_position - pt);
                real_t lights_length = distance(lights_position, pt);
                Ray r(pt, d);
                bool block = false;   
            
                // to check is there any object blocking the light.
                for(size_t j=0; j<scene->num_geometries(); j++) {
                    // dummy s
                    Solution_info s;
                    if (geometries[j]->checkIntersection(r, s, t_max)==true) {
               
                        real_t length_tmp = distance(pt + d*s.t, pt);
                        if ( length_tmp<lights_length) {
                            block = true;
                            continue;  
                        }      
                        
                    }
                }

                real_t attenuation = 1;
                real_t insert_angle = 0;

                if (block == false) {

                    attenuation = 1.0/ (lights[i].attenuation.constant + 
                                        lights[i].attenuation.linear*lights_length + 
                                        lights[i].attenuation.quadratic*pow(lights_length,2)); 
                    insert_angle = dot(d, material_para.normal);
                    if (insert_angle<0) {
                        insert_angle = 0;
                    }

                    diffuse_Color_per = lights[i].color*attenuation*material_para.diffuse*insert_angle;
                }
            
        }else {

            int monte_times = 0;
            // generate random lights position.     
            for (size_t k=0; k<MONTE_CAROL_TIMES; k++) {
                
                // generate random lights position. 
                Vector3 lights_position = create_montecarol(lights[i].position, lights[i].radius);
                Vector3 d = normalize(lights_position - pt);
                real_t lights_length = distance(lights_position, pt);
                Ray r(pt, d);
                bool block = false;   
                // to check is there any object blocking the light.
                for(size_t j=0; j<scene->num_geometries(); j++) {
                    // dummy s
                    Solution_info s;
                    if (geometries[j]->checkIntersection(r, s, t_max)==true) {
                        
                        real_t length_tmp = distance(pt + d*s.t, pt);
                        if ( length_tmp<lights_length) {
                            block = true;
                            continue;  
                        }      
                    }
                }

                real_t attenuation = 1;
                real_t insert_angle = 0;

                if (block == false) {

                    monte_times++;

                    attenuation = 1.0/ (lights[i].attenuation.constant + 
                    lights[i].attenuation.linear*lights_length + 
                    lights[i].attenuation.quadratic*pow(lights_length,2)); 

                    insert_angle = dot(d, material_para.normal);
                    if (insert_angle<0) {
                        insert_angle = 0;
                    }
                    //caculate the diffuse light
                    diffuse_Color_per = diffuse_Color_per + lights[i].color*attenuation*material_para.diffuse*insert_angle;
                }
            }
            //sum all the monte carol lights and average all of them.
            diffuse_Color_per = diffuse_Color_per*(real_t(1.0)/MONTE_CAROL_TIMES);
        }

        diffuse_Color = diffuse_Color + diffuse_Color_per;

    }

    return diffuse_Color;


}


Color3 Raytracer::recursive_raytracing(Ray r, size_t reflectTime) 
{   
    size_t const recursion_limit = 5;
    if (reflectTime > recursion_limit) {
        return Color3::Black();
    }
    reflectTime++;

    // loop search for the smallest t and its index. 
    bool hit_flag = false;
    Solution_info s;
    Solution_info s_min;
    s_min.t = t_max;
    real_t R; //frensal 

    int geometry_index = 0;    
    for (size_t i=0; i<scene->num_geometries(); i++) {

        if (geometries[i]->checkIntersection(r, s, t_max)==true) {
            hit_flag = true;        
            if(s.t < s_min.t) {
                s_min.t = s.t;
                s_min.beta = s.beta;
                s_min.gamma = s.gamma;
                s_min.index = s.index;
                geometry_index = i;
            }
        }
    }


    // Caculate the color

    Color3 diffuse_Color(0.0, 0.0, 0.0);
    Color3 ambient_Color(0.0, 0.0, 0.0);
    Color3 direct_Color(0.0, 0.0, 0.0);
    Color3 specular_Color(0.0, 0.0, 0.0);
    Color3 refract_Color(0.0, 0.0, 0.0);
    Color3 final_Color(0.0, 0.0, 0.0);

    if (hit_flag == true) {

        //Get the intesect point
        Vector3 inter_Pt = r.e + s_min.t*r.d;
        //Vector3 normal_Pt = ;
        // Get the geometry property
        Material_Para material_para = geometries[geometry_index]->getMaterial(r, s_min);
 
        if (material_para.refractive_index == 0 ) {


            // direct illumination
            diffuse_Color = calDiffuseColor(inter_Pt, material_para);    
            ambient_Color = material_para.ambient * scene->ambient_light;
            direct_Color = diffuse_Color + ambient_Color;   


            // specular
            Color3 reflection_Color(0.0, 0.0, 0.0);
            if (dot(r.d, material_para.normal)>0) {
//            if (dot(r.d, material_para.normal)<0) {  use for cylinder case

                specular_Color =  Color3::Black();
            }else { 
                Vector3 newray_direction = r.d - 2*dot(material_para.normal, r.d)*material_para.normal;
                newray_direction = normalize(newray_direction); 
                Ray newray(inter_Pt, newray_direction);
                reflection_Color = recursive_raytracing(newray, reflectTime);
                specular_Color = reflection_Color * material_para.specular;
            }

            //sum all color parts.
            final_Color = material_para.texture*( direct_Color + specular_Color );
            return final_Color;
            //return ambient_Color;

        // not opaque    
        }else {

            Vector3 refract_direction;

            // check whether the full internal reflection happen or not.
            bool tir = caculate_Refracted_Ray(R, material_para, r, refract_direction) ;
            if ( tir ) {
            
            Ray refract_ray(inter_Pt, refract_direction);
                refract_Color = recursive_raytracing(refract_ray, reflectTime);
            } else {
                R = 1;
            }
            
            // specular
            Color3 reflection_Color(0.0, 0.0, 0.0);
            Vector3 newray_direction = r.d - 2*dot(material_para.normal, r.d)*material_para.normal;
            newray_direction = normalize(newray_direction); 
            Ray newray(inter_Pt, newray_direction);
            reflection_Color = recursive_raytracing(newray, reflectTime);
            specular_Color = reflection_Color;

            // sum all the color part.
            final_Color = R*specular_Color + (1-R)*refract_Color;
            return final_Color;
        }



    //  if there is no intersection, return with background color.
    }else {

        return scene->background_color;
    }
}

bool Raytracer::caculate_Refracted_Ray(real_t &R, Material_Para material_para, Ray r, Vector3 &newray) {

    real_t n1;
    real_t n2;
    real_t cos_angle = dot(r.d, material_para.normal);
    real_t cos_R;
    real_t R0;

    //enter
    if(cos_angle<0) {
        n1 = scene->refractive_index;
        n2 = material_para.refractive_index;
        cos_angle = -cos_angle;

        Vector3 bsin_theta = (r.d + material_para.normal*cos_angle) * (n1 / n2);
        real_t q = 1 - pow(n1/n2, 2)*(1-pow(cos_angle, 2));
        if ( q < 0) {
            return false;
        }
        Vector3 ncos_theta = material_para.normal * sqrt(q);

        newray = bsin_theta - ncos_theta;
        
        if (n1 < n2) {
            cos_R = cos_angle;
        }else {
            cos_R = sqrt(q);
        }

        R0 = pow((n2-1)/(n2+1), 2);

    //leave
    }else {
        n1 = material_para.refractive_index;
        n2 = scene->refractive_index;
        material_para.normal = -material_para.normal;
        
        Vector3 bsin_theta = (r.d + material_para.normal*cos_angle) * (n1 / n2);
        real_t q = 1 - pow(n1/n2, 2)*(1-pow(cos_angle, 2));
        if ( q < 0) {
            return false;
        }
        Vector3 ncos_theta = material_para.normal * sqrt(q);

        newray = bsin_theta - ncos_theta;
    
        if (n1 < n2) {
            cos_R = cos_angle;
        }else {
            cos_R = sqrt(q);
        }

        R0 = pow((n2-1)/(n2+1), 2);

    }

    R = R0 + (1-R0)*pow((1-cos_R), 5);
    assert(R>=0 && R<=1);
    return true;
    
    
}

/**
 * Raytraces some portion of the scene. Should raytrace for about
 * max_time duration and then return, even if the raytrace is not copmlete.
 * The results should be placed in the given buffer.
 * @param buffer The buffer into which to place the color data. It is
 *  32-bit RGBA (4 bytes per pixel), in row-major order.
 * @param max_time, If non-null, the maximum suggested time this
 *  function raytrace before returning, in seconds. If null, the raytrace
 *  should run to completion.
 * @return true if the raytrace is complete, false if there is more
 *  work to be done.
 */
bool Raytracer::raytrace(unsigned char* buffer, real_t* max_time)
{
    // TODO Add any modifications to this algorithm, if needed.


    static const size_t PRINT_INTERVAL = 64;

    // the time in milliseconds that we should stop
    unsigned int end_time = 0;
    bool is_done;

    if (max_time)
    {
        // convert duration to milliseconds
        unsigned int duration = (unsigned int) (*max_time * 1000);
        end_time = SDL_GetTicks() + duration;
    }

    // until time is up, run the raytrace. we render an entire group of
    // rows at once for simplicity and efficiency.
    for (; !max_time || end_time > SDL_GetTicks(); current_row += STEP_SIZE)
    {
        // we're done if we finish the last row
        is_done = current_row >= height;
        // break if we finish
        if (is_done) break;

        int loop_upper = std::min(current_row + STEP_SIZE, height);
        // This tells OpenMP that this loop can be parallelized.
#pragma omp parallel for
        for (int c_row = current_row; c_row < loop_upper; c_row++)
        {
            /*
             * This defines a critical region of code that should be
             * executed sequentially.
             */
#pragma omp critical
            {
                if (c_row % PRINT_INTERVAL == 0)
                    printf("Raytracing (Row %d)\n", c_row);
            }

            for (size_t x = 0; x< width; x++)
            {
                // trace a pixel
                Color3 color = trace_pixel(scene, x, c_row, width, height);
                // write the result to the buffer, always use 1.0 as the alpha
                color.to_array(&buffer[4 * (c_row * width + x)]);
            }
        }
    }

    if (is_done) printf("Done raytracing!\n");

    return is_done;
}

} /* _462 */
