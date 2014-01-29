#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <cassert>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <assimp/scene.h>

namespace viewer
{

namespace drawer { class Drawer; }

namespace objects
{

using namespace std;

struct Bound
{
    struct Min
    {
        float   x;
        float   y;
        float   z;
    } min;

    struct Max
    {
        float   x;
        float   y;
        float   z;
    } max;
}; // struct Bound

class Mesh
{
    friend class drawer::Drawer;
    struct Local
    {
        vector<uint32_t>    indice;
        vector<float>       vert;
        vector<float>       normal;
        vector<float>       uv;
        glm::vec4           diffuse;
        glm::vec4           ambient;
        glm::vec4           specular;
        glm::vec4           emissive;
        float               shininess;
        uint32_t            maxShininess;

        string              texture;
    } local;

    struct GL
    {
        GLuint  indice;
        GLuint  vert;
        GLuint  normal;
        GLuint  uv;

        GLuint  texID;
    } gl;

    public:
        Mesh(void);
        ~Mesh(void);

        void load(aiMesh *mesh, const aiScene *scene, Bound::Min &min, Bound::Max &max);
        void setup(unordered_map<string, GLuint> textureID);
        void draw(void);
}; // class Mesh

inline
Mesh::Mesh(void)
:local()
,gl()
{
}

inline
Mesh::~Mesh(void)
{
    glDeleteBuffers(1, &gl.indice);
    glDeleteBuffers(1, &gl.vert);
    glDeleteBuffers(1, &gl.normal);
    glDeleteBuffers(1, &gl.uv);
}

inline
void Mesh::load(aiMesh *mesh, const aiScene *scene, Bound::Min &min, Bound::Max &max)
{
    uint32_t    faces   = mesh->mNumFaces;
    uint32_t    indices = faces * 3;
    uint32_t    verts   = mesh->mNumVertices;

    local.indice.resize(indices);
    local.vert.resize(verts * 3);
    local.normal.resize(verts * 3);
    local.uv.resize(verts * 2);

    for(uint32_t f = 0; f < faces; ++ f)
    {
        const aiFace &face = mesh->mFaces[f];
        assert(face.mNumIndices == 3);
        local.indice[f * 3 + 0] = face.mIndices[0];
        local.indice[f * 3 + 1] = face.mIndices[1];
        local.indice[f * 3 + 2] = face.mIndices[2];
    }

    for(uint32_t v = 0; v < verts; ++ v)
    {
        if(mesh->HasPositions())
        {
            local.vert[v * 3 + 0] = mesh->mVertices[v].x;
            local.vert[v * 3 + 1] = mesh->mVertices[v].y;
            local.vert[v * 3 + 2] = mesh->mVertices[v].z;

            min.x = ::min(min.x, mesh->mVertices[v].x);
            min.y = ::min(min.y, mesh->mVertices[v].y);
            min.z = ::min(min.z, mesh->mVertices[v].z);

            max.x = ::max(max.x, mesh->mVertices[v].x);
            max.y = ::max(max.y, mesh->mVertices[v].y);
            max.z = ::max(max.z, mesh->mVertices[v].z);
        }

        if(mesh->HasNormals())
        {
            local.normal[v * 3 + 0] = mesh->mNormals[v].x;
            local.normal[v * 3 + 1] = mesh->mNormals[v].y;
            local.normal[v * 3 + 2] = mesh->mNormals[v].z;
        }

        if(mesh->HasTextureCoords(0))
        {
            local.uv[v * 2 + 0] = mesh->mTextureCoords[0][v].x;
            local.uv[v * 2 + 1] = mesh->mTextureCoords[0][v].y;
        }
    }

    aiMaterial *mtl = scene->mMaterials[mesh->mMaterialIndex];
    aiString path;
    if(mtl->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
        local.texture = path.data;

    aiColor4D color;
    if(aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS)
        local.diffuse = glm::vec4(color.r, color.g, color.b, color.a);

    if(aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &color) == AI_SUCCESS)
        local.ambient = glm::vec4(color.r, color.g, color.b, color.a);

    if(aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &color) == AI_SUCCESS)
        local.specular = glm::vec4(color.r, color.g, color.b, color.a);

    if(aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &color) == AI_SUCCESS)
        local.emissive = glm::vec4(color.r, color.g, color.b, color.a);

    aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &local.shininess, &local.maxShininess);
}

inline
void Mesh::setup(unordered_map<string, GLuint> textureID)
{
    glGenBuffers(1, &gl.indice);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.indice);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, local.indice.size() * sizeof(uint32_t), &local.indice[0], GL_STATIC_DRAW);

    glGenBuffers(1, &gl.vert);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vert);
    glBufferData(GL_ARRAY_BUFFER, local.vert.size() * sizeof(float), &local.vert[0], GL_STATIC_DRAW);

    glGenBuffers(1, &gl.normal);
    glBindBuffer(GL_ARRAY_BUFFER, gl.normal);
    glBufferData(GL_ARRAY_BUFFER, local.normal.size() * sizeof(float), &local.normal[0], GL_STATIC_DRAW);

    glGenBuffers(1, &gl.uv);
    glBindBuffer(GL_ARRAY_BUFFER, gl.uv);
    glBufferData(GL_ARRAY_BUFFER, local.uv.size() * sizeof(float), &local.uv[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(!local.texture.empty())
        gl.texID = textureID.at(local.texture);
}

inline
void Mesh::draw(void)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.indice);

    glBindBuffer(GL_ARRAY_BUFFER, gl.vert);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, gl.uv);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, gl.normal);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindTexture(GL_TEXTURE_2D, gl.texID);

    glDrawElements(GL_TRIANGLES, local.indice.size(), GL_UNSIGNED_INT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace objects

} // namespace viewer

#endif // __OBJECTS_H__
