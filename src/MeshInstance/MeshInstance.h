//
// Created by Hubert Klonowski on 14/03/2025.
//

#ifndef MESHINSTANCE_H
#define MESHINSTANCE_H
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "assimp/scene.h"
#include "glad/glad.h"
#include "imgui_impl/imgui_impl_opengl3_loader.h"
#include "Shader.h"
#include "assimp/scene.h"
#include "stb_image.h"

#define MAX_BONE_INFLUENCE 4


struct Vertex1 {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    int m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture1 {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh1 {
    std::vector<Vertex1> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture1> textures;

    unsigned int VAO, VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex1), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex1), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex1), (void*)offsetof(Vertex1, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex1), (void*)offsetof(Vertex1, texCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex1), (void*)offsetof(Vertex1, tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex1), (void*)offsetof(Vertex1, bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex1), (void*)offsetof(Vertex1, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex1), (void*)offsetof(Vertex1, m_Weights));
        glBindVertexArray(0);
    }
    unsigned int getVAO() {
        return this->VAO;
    }

public:
    Mesh1(std::vector<Vertex1> vertices, std::vector<unsigned int> indices, std::vector<Texture1> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }
    void Render(Shader* shader) {
        unsigned int skyboxTexture = NULL;
        // std::cout << "Mesh Draw - isTorus: " << isTorus
        //           << ", vertices: " << vertices.size()
        //           << ", indices: " << indices.size() << std::endl;

        // bind appropriate textures
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        // if(skyboxTexture) {
        //     glActiveTexture(GL_TEXTURE0);
        //     glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        // }
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = textures[i].type;
            if(name == "texture_diffuse") {
                number = std::to_string(diffuseNr++);
                // shader.setInt("material.diffuse", textures[i].id);
            }
            else if(name == "texture_specular") {
                number = std::to_string(specularNr++); // transfer unsigned int to string
                // shader.setInt("material.specular", textures[i].id);
            }
            else if(name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to string
            else if(name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to string

            // now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader->ID, (name + number).c_str()), i);

            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);

        }

        // draw mesh
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }
};

class MeshInstance {

Shader* shader = nullptr;

    std::string directory;
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh1 processMesh(aiMesh* mesh, const aiScene* scene);

    std::vector<Texture1> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

    unsigned int textureFromFile(const char* path, const std::string& directory);

public:

    MeshInstance();

    void Render();

    void LoadModel(const std::string& path);

    void AddMesh(Mesh1 mesh);
    std::vector<Texture1> textureLoaded;
    std::vector<Mesh1> meshes;

    void SetShader(Shader* shader);
};





#endif //MESHINSTANCE_H
