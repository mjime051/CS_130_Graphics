#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{
	//We found this in Lab1 so use that formula/function
	Hit intersect;
	vec3 x = ray.endpoint - center;
	//using quadratic formula so find each part to use later
	double a = ray.direction.magnitude_squared();
	double b = 2 * dot(ray.direction, x);
	double c = x.magnitude_squared() - (radius*radius);
	double root = (b*b) - (4 * a*c);
	if (root < 0) {//the root is imaginary, dont bother cause that means there is no intersection
		intersect = {0, 0, part};
	}
	else {//the root is positive so there are roots/intersections!
		double root1 = ((-1)*b + sqrt(root)) / (2 * a);//root1: -b + sqrt
		double root2 = ((-1)*b - sqrt(root)) / (2 * a);//root2: -b - sqrt
		if (root1 < root2 && root1 >= small_t) { //root1 is first point of intersection
			intersect = {this, root1, part};
		}
		else if (root2 <= root1 && root2 >= small_t) { //root2 is first point of intersection. If they are equal then they intersect at one point so just set either root, in this case root2
			intersect = {this, root2, part};
		}
		else { //no intersect
			intersect = {0, 0, part};
		}
	}
	return intersect;
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
	vec3 normal = point - center; // look at slides if you need help visualizing
	normal = normal.normalized(); // unitize/normalize it
	return normal;
}

Box Sphere::Bounding_Box(int part) const
{
    Box box;
    TODO; // calculate bounding box
    return box;
}
