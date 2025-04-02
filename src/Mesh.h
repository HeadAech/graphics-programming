//
// Created by Hubert Klonowski on 12/11/2024.
//

#ifndef MESH_H
#define MESH_H
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Shader.h"
#include "imgui_impl/imgui_impl_opengl3_loader.h"

#define MAX_BONE_INFLUENCE 4


enum Material {
    STANDARD,
    REFLECTIVE,
    REFRACTIVE,
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    //tangent
    glm::vec3 tangent;
    // bitangent
    glm::vec3 biTangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
    public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Material material = STANDARD;

    bool instanced = false;

    unsigned int VAO, VBO, EBO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader *shader, unsigned int skyboxTexture);

    void setupMesh();

    unsigned int getVAO();
};



#endif //MESH_H
