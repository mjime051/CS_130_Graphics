#include "mesh.h"
#include "plane.h"
#include "ray.h"
#include <fstream>
#include <string>
#include <limits>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts).
void Mesh::Read_Obj(const char* file)
{
    std::ifstream fin(file);
    if(!fin)
    {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e;
    vec3 v;
    box.Make_Empty();
    while(fin)
    {
        getline(fin,line);

        if(sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
        {
            vertices.push_back(v);
            box.Include_Point(v);
        }

        if(sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
        }
    }
    number_parts=triangles.size();
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray& ray, int part) const
{
	double distance;
	int i = 0;
	Hit intersect = {0,0,0};

	//if part is already >= 0 then we are at a specific triangle and just checking to see if there is an intersection between the ray parameter and that triangle
	if (part >= 0) {
		//if there is an intersection then just return {this, distance, part} as the Hit
		if (Intersect_Triangle(ray, part, distance)) {
			intersect = { this, distance, part };
		}
	}
	else { //check all triangle to make sure they is no intersection between the ray and the triangle. You could just go through all the triangles from the start but that isnt efficient
		while (i < (int)triangles.size() && !Intersect_Triangle(ray, i, distance)) {
			i++;
		}//Hey there was an intersection and we have not reached i = traingles.size() so return the the hit with the part we are currently at
		if (i != (int)triangles.size()) {
			intersect = { this, distance, i };
		}
	}


	//we went through all the triangles and no intersections were found so intersect was not updated and we will return {0,0,0}
	return intersect;
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const vec3& point, int part) const
{
	assert(part >= 0);
	//I know this is a long way of doing this but it just makes more sense to me finding the individual components
	//First get the vertices of the triangle
	vec3 a = vertices.at(triangles[part][0]);
	vec3 b = vertices.at(triangles[part][1]);
	vec3 c = vertices.at(triangles[part][2]);
	//Then find any two vectors of the triangle.
	vec3 ab = a - b;
	vec3 ac = a - c;
	//now find the normal by taking the cross product of the two vectors. Dont forget to normalize it to make things easier in later equations!
	vec3 normal = cross(ab, ac).normalized();

	return normal;
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
bool Mesh::Intersect_Triangle(const Ray& ray, int tri, double& dist) const
{
	//Find the vertices again
	vec3 a = vertices.at(triangles[tri][0]);
	vec3 b = vertices.at(triangles[tri][1]);
	vec3 c = vertices.at(triangles[tri][2]);

	//create a plane using one vertex and the normal to the triangle
	Plane tri_plane(a, Normal(a, tri));
	//check if there is an intersection between the ray and the plane
	Hit tri_intersection = tri_plane.Intersection(ray, tri);
	//If there is no intersection return false
	if (!tri_intersection.object || tri_intersection.dist <= small_t) {
		return false;
	}
	//Now that we know there is an intersection between ray and plane we can try and find the intersection between the ray and the triangle
	vec3 pointPos = ray.Point(tri_intersection.dist);
	//the three vectors we will use for the triangle: vector a to b, vector a to c, and vector a to the point
	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ap = pointPos - a;
	//u is the vector of the ray
	vec3 u = ray.direction;

	//just follow the formulas for barycentric coordinates. Too long to explain math
	double gamma = dot(cross(u, ab), ap) / dot(cross(u, ab), ac);
	double beta = dot(cross(ac, u), ap) / dot(cross(ac, u), ab);
	double alpha = 1 - beta - gamma;

	//we use weight tolerance instead of just zero, read the function explanation for why we do this. Learned in classe that you do not actually need to check if it is between
	//0 and 1 so just make sure that all barycentric coordinates are positive or >= -weight_tolerance
	if (alpha >= -weight_tolerance && beta >= -weight_tolerance && gamma >= -weight_tolerance) {
		dist = tri_intersection.dist;
		return true;
	}

	return false;
}

// Compute the bounding box.  Return the bounding box of only the triangle whose
// index is part.
Box Mesh::Bounding_Box(int part) const
{
    Box b;
    TODO;
    return b;
}
