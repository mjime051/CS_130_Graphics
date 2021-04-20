#ifndef OBJ_H
#define OBJ_H

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <iostream>

class obj
{
public:
    // Faces maintain indices to the data for efficiency, rather
    // than duplicating the data. Provided the indices, you can call
    // get_vertex and get_normal in obj:: to get the data.
    struct face
    {
        std::string mat;
        std::string group;
        std::vector<int> vind; // vertex indices
        std::vector<int> nind; // normal indices
        std::vector<int> tex;  // texture indices
    };

    struct material
    {
        material();

        float ka[3], ks[3], kd[3];
        float ns;
        int illum;
        std::string map_ka, map_kd, map_ks;
    };

    obj();
    ~obj();

    bool load(const std::string& filename);

    inline size_t get_vertex_count() const;
    inline size_t get_normal_count() const;
    inline size_t get_texture_indices_count() const;

    // returns the vertex with vertex index i
    inline const float* get_vertex(size_t i) const;

    // returns the normal with normal index i
    inline const float* get_normal(size_t i) const;

    // returns the texture with texture index i
    inline const float* get_texture_indices(size_t i) const;

    inline size_t get_face_count() const;
    inline const face& get_face(size_t i) const;
    inline const material& get_material(
        const std::string& name, unsigned int index=0
        ) const;
    inline const std::map<std::string, material>& get_materials(unsigned int index=0) const;

    bool valid_material(unsigned int index)
    { return index<mats_list.size();}
    
    bool add_material(const std::string& filename)
    {return load_material(filename,mats_list.size());}

private:
    bool load_material(const std::string& filename,unsigned int index=0);

    std::vector<float> verts;
    std::vector<float> norms;
    std::vector<float> tex;
    std::vector<face> faces;
    std::vector<std::map<std::string, material> > mats_list;
};

size_t obj::get_face_count() const
{
    return faces.size();
}

const obj::face& obj::get_face(size_t i) const
{
    assert(i < faces.size());
    return faces[i];
}

size_t obj::get_vertex_count() const
{
    return verts.size() / 3;
}

const float* obj::get_vertex(size_t i) const
{
    return &verts[3*i];
}

size_t obj::get_normal_count() const
{
    return norms.size() / 3;
}

const float* obj::get_normal(size_t i) const
{
    return &norms[3*i];
}

size_t obj::get_texture_indices_count() const
{
    return tex.size() / 2;
}

const float* obj::get_texture_indices(size_t i) const
{
    return &tex[2*i];
}

const obj::material& obj::get_material(
    const std::string& name, unsigned int index
    ) const
{
    assert(index<mats_list.size());
    const std::map<std::string, material>& mats=mats_list[index];
    std::map<std::string, material>::const_iterator i =
        mats.find(name);
    assert(i != mats.end());
    return i->second;
}

const std::map<std::string, obj::material>& obj::get_materials(unsigned int index) const
{
    assert(index<mats_list.size());
    return mats_list[index];
}

#endif
