#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

//Psuedo: Achieves better shading by finding normals at each point of the surface 
vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
	vec3 color; //our final color, the sum of all components
	/*vec3 ambColor; //I tried using three components and then adding at the end but for some reason I could not get it to work
	vec3 diffColor; //Will try to come back to since I think it looks more readable to have these setup and then just add them up at the end
	vec3 specColor; */
	vec3 lightVec; //Looking at the image this will be the vector from surface to light source
	vec3 reflectVec; //Looking at the image this is the reflected vector bouncing off the surface
	vec3 shadow;
	//Ray shadowRay; tried to initialize them out here but was not working as intended. I feel it is better to just initalize once and overwrite instead of creating one everytime
	//Hit shadowIntersect; we run the loop but could not figure out how to make it work

	Light* currLight; //our light pointer which we will use to traverse through all lights in the world
	color += world.ambient_color*world.ambient_intensity*color_ambient; //this is the ambient aspect of the phong shading. So in the slide this is Ra*La
	if (world.enable_shadows){
		for (unsigned int i = 0; i < world.lights.size(); i++) { //This is why its expensive, have to go through everypoint on the surface to compute the shading
			currLight = world.lights[i];//For each light
			lightVec = currLight->position - intersection_point; //set the light vector
			Ray shadowRay(intersection_point, lightVec);//compute shadow ray
			Hit shadowIntersect = world.Closest_Intersection(shadowRay);
			if (shadowIntersect.dist > 0) {//this check is for the bug test! (21) sneaky sneaky behind the light
				shadow = shadowRay.Point(shadowIntersect.dist) - intersection_point;
			}
			if (!shadowIntersect.object || shadow.magnitude() > lightVec.magnitude()) {//shadow ray didnt hit an object so add light diffuse and specular components
				color += color_diffuse * currLight->Emitted_Light(lightVec)*(std::max(dot(normal, lightVec.normalized()), 0.0)); // follow the slides for this computation
				reflectVec = -lightVec + 2 * (dot(lightVec, normal))*normal; //set the reflection vector
				color += color_specular * currLight->Emitted_Light(lightVec)*std::pow(std::max(dot(-ray.direction, reflectVec.normalized()), 0.0), specular_power); //just follow the slides for this
			}
		}
	}
	else { //set pixel color to background
		for (unsigned int j = 0; j < world.lights.size(); j++){
			currLight = world.lights[j];
			lightVec = currLight->position - intersection_point; //set the light vector
			color += color_diffuse * currLight->Emitted_Light(lightVec)*(std::max(dot(normal, lightVec.normalized()), 0.0)); // follow the slides for this computation
			reflectVec = -lightVec + 2 * (dot(lightVec, normal))*normal; //set the reflection vector
			color += color_specular * currLight->Emitted_Light(lightVec)*std::pow(std::max(dot(-ray.direction, reflectVec.normalized()), 0.0), specular_power); //just follow the slides for this
		}
	}
	//color = ambColor + diffColor + specColor; //finally you add them all together to get the final color
    return color;
}