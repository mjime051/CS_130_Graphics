#include "reflective_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Reflective_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
	vec3 color;
	color = (1 - reflectivity) * shader->Shade_Surface(ray, intersection_point, normal, recursion_depth);//oh hey we are gonna recurse! 
	//as long as our current recursion depth < recursion depth limit then continue recursing!
	if (recursion_depth < world.recursion_depth_limit) { 
		vec3 view = ray.endpoint - intersection_point; //Compute view ray. r(t) = e + tu
		vec3 reflect = (2 * dot(view, normal) * normal - view).normalized(); //Now that we have view we find the reflected ray using 2(N.view)N - view. Make sure its normalized!
		Ray reflectedRay(intersection_point, reflect);//make it into a ray so we can use it in Cast_Ray()
		color += reflectivity * world.Cast_Ray(reflectedRay, recursion_depth + 1);//This is where we recurse because Cast_Ray() call Shade_Surface()
	}
	//This will break the recursion once we reach our recursion depth limit
	return color;
}
