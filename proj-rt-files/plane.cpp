#include "plane.h"
#include "ray.h"
#include <cfloat>
#include <limits>

// Intersect with the half space defined by the plane.  The plane's normal
// points outside.  If the ray starts on the "inside" side of the plane, be sure
// to record a hit with t=0 as the first entry in hits.
Hit Plane::Intersection(const Ray& ray, int part) const
{
	Hit intersect;
	double y = dot(ray.direction, normal);
	if (y != 0) { //the dot product is not 0 so there is an intersection
		vec3 x = ray.endpoint - x1; // E - x1
		double t = (-1)*dot(x, normal) / y;
		if (t > small_t) {//this makes sure the ray intersects with a value of t > 0
			intersect = {this, t, part};
		}
		else { //if t < 0 then the ray intersects but only in the opposite direction so we ignore that and set intersect to 0
			intersect = {0, 0, part};
		}
	}
	else {//dot product is 0 so no intersect
		intersect = {0, 0, part};
	}
	return intersect;
}

vec3 Plane::Normal(const vec3& point, int part) const
{
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Plane::Bounding_Box(int part) const
{
    Box b;
	//just saw this and looked it up, can also use the numeric_limits funct to set min
    b.hi.fill(std::numeric_limits<double>::max());
    b.lo=-b.hi;
    return b;
}
