#include "driver_state.h"
#include <cstring>
#include <climits>
#include <algorithm>
#include <cfloat>

driver_state::driver_state()
{
}

driver_state::~driver_state()
{
    delete [] image_color;
    delete [] image_depth;
}

// This function should allocate and initialize the arrays that store color and
// depth.  This is not done during the constructor since the width and height
// are not known when this class is constructed.
void initialize_render(driver_state& state, int width, int height)
{
    state.image_width=width;
    state.image_height=height;
    state.image_color = new pixel[width*height];
    state.image_depth = new float[width*height];
	//Last time I could just set initially to a large value but because float variables can hold huge values I just included cfloat library so I could use FLT_MAX
	for (int i = 0; i < width*height; i++)
	{
		state.image_color[i] = make_pixel(0, 0, 0);
		state.image_depth[i] = FLT_MAX;
	}
}

// This function will be called to render the data that has been stored in this class.
// Valid values of type are:
//   render_type::triangle - Each group of three vertices corresponds to a triangle.
//   render_type::indexed -  Each group of three indices in index_data corresponds
//                           to a triangle.  These numbers are indices into vertex_data.
//   render_type::fan -      The vertices are to be interpreted as a triangle fan.
//   render_type::strip -    The vertices are to be interpreted as a triangle strip.
void render(driver_state& state, render_type type)
{
	//I should declare this in triangle case but gives me errors about jump to case label, will come back to that once I figure out the issue
	switch (type) {
		case render_type::triangle: {
			int triangles;
			int currVertex = 0;

			data_geometry * dataGeoArr = new data_geometry[3];
			data_vertex dataVertex;
			//This gives us the number of trangles
			triangles = state.num_vertices / 3;
			//for each triangle we have to call rasterize triangle but we need to fill dataGeoArr for each individual triangle
			for (int i = 0; i < triangles; i++) {
				//the data for each data_geometry in the array points to the first float of each vertex. 
				//Increment by state.floats_per_vertex to get to the next set of data for the next vertex
				for (int j = 0; j < 3; j++) {
					dataGeoArr[j].data = state.vertex_data + currVertex;
					currVertex += state.floats_per_vertex;
				}
				//now call vertex shader for our data_vertex using the array of data_geometry and the states uniform data
				for (int k = 0; k < 3; k++) {
					dataVertex.data = dataGeoArr[k].data;
					state.vertex_shader(dataVertex, dataGeoArr[k], state.uniform_data);
				}
				//finally call rasterize triangle on our dataGeoArr. Dont like how this looks 
				//but need to cast it to this for rasterize_triangle to work
				rasterize_triangle(state, (const data_geometry **)(&dataGeoArr));
			}
			break;
		}

		case render_type::indexed: {
			int triangles;
			int currVertex = 0;

			data_geometry * dataGeoArr = new data_geometry[3];
			data_vertex dataVertex;
			triangles = state.num_vertices / 3;
			for (int i = 0; i < triangles; i++)
			{
				for (int j = 0; j < 3; j++) {
					dataGeoArr[j].data = state.vertex_data + state.index_data[currVertex] * state.floats_per_vertex;
					currVertex += 3;
				}
				for (int k = 0; k < 3; k++) {
					dataVertex.data = dataGeoArr[k].data;
					state.vertex_shader(dataVertex, dataGeoArr[k], state.uniform_data);
				}
				rasterize_triangle(state, (const data_geometry **)(&dataGeoArr));
			}
			break;
		}

		case render_type::fan: {
			//TODO
			break;
		}

		case render_type::strip: {
			//TODO
			break;
		}

		default: {
			break;
		}
	}
}


// This function clips a triangle (defined by the three vertices in the "in" array).
// It will be called recursively, once for each clipping face (face=0, 1, ..., 5) to
// clip against each of the clipping faces in turn.  When face=6, clip_triangle should
// simply pass the call on to rasterize_triangle.
void clip_triangle(driver_state& state, const data_geometry* in[3],int face)
{
    if(face==6)
    {
        rasterize_triangle(state, in);
        return;
    }
    //std::cout<<"TODO: implement clipping. (The current code passes the triangle through without clipping them.)"<<std::endl;
    clip_triangle(state,in,face+1);
}

// Rasterize the triangle defined by the three vertices in the "in" array.  This
// function is responsible for rasterization, interpolation of data to
// fragments, calling the fragment shader, and z-buffering.
//I HAVE MADE THIS MISTAKE WAY TOO MANY TIMES!!! IF TRYING TO GET VARIABLES FROM const data_gemoetry* in USE (*in). POINTERS ARE ANNOYING BUT 
//YOU NEED TO REMEMBER WHAT IS POINTING TO WHAT
void rasterize_triangle(driver_state& state, const data_geometry* in[3])
{
	//represents x and y pixel coordinates for the vertices of the triangle. ex. Vertex A is at x[0], y[0]
	int x[3];
	int y[3];

	//This array is for z-buffering, hence the name and depth will be used as a checker variable 
	float z[3];
	float depth = 0;

	//boundaries so we can make it more efficient 
	int xMin;
	int yMin;
	int xMax;
	int yMax;
	//These are the weights in the slides. we technically only need to find two of the bary coords
	//and then just do 1-alpha-beta or whichever two you find to get the last one
	//Work with floats since that is the data type that everything else is
	float k0[3];
	float k1[3];
	float k2[3];
	//Need total area of the triangle to get bary coords
	float wholeTriArea;
	//I know now that this represents the bary coords for screen cpase so I should rename respectively later for final submission
	float baryCoords[3];

	// Find the pixel coords and put them into x[] and y[]

	for (int iter = 0; iter < 3; iter++) {
		x[iter] = (int)(state.image_width / 2.0 * (*in)[iter].gl_Position[0] / (*in)[iter].gl_Position[3] + (state.image_width / 2.0 - .5));
		y[iter] = (int)(state.image_height / 2.0 * (*in)[iter].gl_Position[1] / (*in)[iter].gl_Position[3] + (state.image_height / 2.0 - .5));
	}
	//All of these calculations are taken from class notes. If you need help then look at those or a tutorial on triangle rasterization
	wholeTriArea = .5 * ((x[1] * y[2] - x[2] * y[1]) - (x[0] * y[2] - x[2] * y[0]) + (x[0] * y[1] - x[1] * y[0]));
	//Just for reference, Vertex A is x[0], y[0]; Vertex B is x[1], y[1]; and Vertex C is x[2], y[2]
	k0[0] = x[1] * y[2] - x[2] * y[1];
	k1[0] = y[1] - y[2];
	k2[0] = x[2] - x[1];

	k0[1] = x[2] * y[0] - x[0] * y[2];
	k1[1] = y[2] - y[0];
	k2[1] = x[0] - x[2];

	k0[2] = x[0] * y[1] - x[1] * y[0];
	k1[2] = y[0] - y[1];
	k2[2] = x[1] - x[0];

	//this will be the default of the boundaries do not change
	xMin = state.image_width - 1;
	yMin = state.image_height - 1;

	//take the min between the image dimensions or the vertices x's and y's 
	for (int minIt = 0; minIt < 3; minIt++) {
		xMin = std::min(xMin, x[minIt]);
		yMin = std::min(yMin, y[minIt]);
	}

	// The minimum pixel coord we can have is (0, 0) so if either xMin or 
	// yMin are negative we set them to 0.
	xMin = std::max(xMin, 0);
	yMin = std::max(yMin, 0);

	//Will need to be updated so initialize to 0 first which is below all other points that will be compared
	xMax = 0;
	yMax = 0;

	//now take the max between all the vertices x's and y's
	for (int maxIt = 0; maxIt < 3; maxIt++) {
		xMax = std::max(xMax, x[maxIt]);
		yMax = std::max(yMax, y[maxIt]);
	}

	// The maximum pixel coord we can have is the images width and height so if either 
	// xMin or yMax are greater then we set them.
	xMax = std::min(xMax, state.image_width - 1);
	yMax = std::min(yMax, state.image_height - 1);

	//now we have our boundaries for which we will check our vertices. I will only be going over the steps of what I am doing, if you need help with the 
	//math please ask the professor or follow your notes we took all of these in class so you should have them or look at a turtorial online
	for (int i = yMin + 1; i < yMax + 1; i++) {
		for (int j = xMin + 1; j < xMax + 1; j++) {
			// Calculate bary coords
			for (int vert = 0; vert < 3; vert++) {
				baryCoords[vert] = .5 * (k0[vert] + (k1[vert] * j) + (k2[vert] * i)) / wholeTriArea;
			}

			//calcute the z-coords
			for (unsigned zIt = 0; zIt < 3; zIt++) {
				z[zIt] = (*in)[zIt].gl_Position[2] / (*in)[zIt].gl_Position[3];
			}

			//I dont know why yet but for some reason if I just do depth += without using temp I fail test 9 for z-buffering
			//So for some reason doing it this way is correct for z-buffering
			float temp = 0;

			for (unsigned updateDepth = 0; updateDepth < 3; updateDepth++) {
				temp += z[updateDepth] * baryCoords[updateDepth];
			}
			
			depth = temp;

			bool baryCheck = true;
			//checking barycentric coords to make sure they are non negative. If they are negative then dont set the image color
			//optimized to only check greater than 0, see slides for why we can do this
			if (baryCoords[0] < 0 || baryCoords[1] < 0 || baryCoords[2] < 0)
			{
				baryCheck = false;
			}
			//If all the bary coords are valid and depth check is valid then set the image_color and image_depth 
			if (baryCheck && depth < state.image_depth[j + i * state.image_width]) {
				//this represents our bary coords for the world space
				float baryWorld[3];
				//variables we will use for vertex_shader to correctly color the image_color
				data_output out;
				data_fragment frag;
				
				frag.data = new float[MAX_FLOATS_PER_VERTEX];
				
				//Interpolation time! have to go through all the floats so we can interpolate based on a type, call fragment shader on our newly filled
				//fragment data into our data output and finally set our image_color and image_depth
				for (int floatIt = 0; floatIt < state.floats_per_vertex; floatIt++) {
					//Depending on what interpolation type the state has we will do different things
					//side note because I create these variables in the cases then you have to wrap it in curly braces
					//otherwise it will give you errors about jump case labels
					switch (state.interp_rules[floatIt]) {
					//flat interpolation, just set all data = to the data of the first vertex
					case interp_type::flat: {
						frag.data[floatIt] = (*in)[0].data[floatIt];
						break;
					}
					//smooth interpolation, need to convert screen space bary coords to world space ones and then interpolate them
					case interp_type::smooth: {
						float k = 0;
						float smoothInterpolatedData = 0;
						//find k so that we can...
						for (unsigned convertIt = 0; convertIt < 3; convertIt++) {
							k += baryCoords[convertIt] / (*in)[convertIt].gl_Position[3];
						}
						//use it here to convert from screen space to world space
						for (unsigned fillIt = 0; fillIt < 3; fillIt++) {
							baryWorld[fillIt] = baryCoords[fillIt] / ((*in)[fillIt].gl_Position[3] * k);
						}
						//Now that we have world bary coords we can interpolate
						for (unsigned interpolateIt = 0; interpolateIt < 3; interpolateIt++)
						{
							smoothInterpolatedData += baryWorld[interpolateIt] * (*in)[interpolateIt].data[floatIt];
						}
						frag.data[floatIt] = smoothInterpolatedData;
						break;
					}
					//no perspective interpolation, since no perspective then no need to convert from screen space to world space
					case interp_type::noperspective: {
						float npInterpolatedData = 0;
						//so all we need to do is interpolate data
						for (unsigned interpolateIt = 0; interpolateIt < 3; interpolateIt++)
						{
							npInterpolatedData += baryCoords[interpolateIt] * (*in)[interpolateIt].data[floatIt];
						}
						//and fill into frag.data
						frag.data[floatIt] = npInterpolatedData;
						break;
					}
					//not a valid interpolation type, idk if it should be flat if nothing is specified so I will leave empty for now
					default:
						break;
					}
				}
				//now that frag.data is filled we can call fragment_shader to make out.output_color the correct color
				state.fragment_shader(frag, out, state.uniform_data);
				//dont forget to delete after use so we dont have memory leaks
				delete[] frag.data;
				//Now out.output_color has a value [0,1] so need to multiply by 255 for each index of RGB
				state.image_color[j + i * state.image_width] = make_pixel(out.output_color[0] * 255, out.output_color[1] * 255, out.output_color[2] * 255);
				//Update the image_depth to be equal to the updated depth
				state.image_depth[j + i * state.image_width] = depth;
			}
			
		}
	}
}

