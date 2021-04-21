#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3)
{}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
	double tempMin = std::numeric_limits<double>::max(); //just initialize tempMin to something big to start off so that it will def change later.
	Hit tempHit; //temp hit that we will use to compare later to see if it is closer than our closest_int intersection/hit
	Object* currObj;
	Hit closest_int = { 0,0,0 }; //initialize closest_int
	for (unsigned int i = 0; i < objects.size();i++) { //gotta run through our vector of objects to see what we intersect with first
		currObj = objects[i]; //the obj we are currently at
		tempHit = currObj->Intersection(ray, -1); //set hit using the intersection function to see if there is a hit between the ray and the object we are currently at
		if (tempHit.object && tempHit.dist < tempMin && tempHit.dist > small_t) { //dont forget to make sure tempHit.dist >= small_t
			tempMin = tempHit.dist; //if hit is true, the distance from that hit is less than our tempMin meaning it is closer than previous closest intersection
			closest_int = tempHit; //and it is >=small_t then we need to update our closest_int and tempMin
		}
	}
	return closest_int;
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
	vec3 endPoint = camera.position; //set endPoint of our ray to the camera's position
	vec3 direction = (camera.World_Position(pixel_index) - endPoint).normalized(); //the direction is from the camera's World position toward the pixel_index param
    Ray ray(endPoint,direction); //create ray with our two prev variables
    vec3 color=Cast_Ray(ray,1);
    camera.Set_Pixel(pixel_index,Pixel_Color(color));
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy();

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    vec3 color;
	Hit closest_int = Closest_Intersection(ray);
	if (closest_int.object != nullptr && recursion_depth <= recursion_depth_limit) { //if there is an intersection that means that our closest_int variable is not null
		vec3 intPoint = ray.Point(closest_int.dist); //set our intersection point to where the ray and object intersect
		vec3 normVect = closest_int.object->Normal(intPoint, closest_int.part); //set normVect to the vector normal to the surface of object
		color = closest_int.object->material_shader->Shade_Surface(ray, intPoint, normVect, recursion_depth);
	}
	else { //there is no intersection so set color to background_shader
		//background_shader is flat shader, any vectors will do for arguments.
		color = background_shader->Shade_Surface(ray, ray.direction, ray.direction, recursion_depth);
	}
    return color;
}

void Render_World::Initialize_Hierarchy()
{
    TODO; // Fill in hierarchy.entries; there should be one entry for
    // each part of each object.

    hierarchy.Reorder_Entries();
    hierarchy.Build_Tree();
}
