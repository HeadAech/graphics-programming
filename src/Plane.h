#ifndef PLANE_H
#define PLANE_H

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <glad/glad.h>
#include "Mesh.h"
#include <stb_image.h>

class Plane {
public:
    Plane(std::string texturePath, float width, float depth) {
        this->texturePath = texturePath;
        this->width = width;
        this->depth = depth;
    }

    Mesh generatePlaneMesh() {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float halfWidth = width / 2.0f;
        float halfDepth = depth / 2.0f;

        // Define vertices
        vertices = {
            {{-halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // Bottom-left
            {{ halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // Bottom-right
            {{ halfWidth, 0.0f,  halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Top-right
            {{-halfWidth, 0.0f,  halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}  // Top-left
        };

        // Define indices
        indices = {
            0, 1, 2, // First triangle
            2, 3, 0  // Second triangle
        };

        // Load texture
        unsigned int textureID = loadTexture(texturePath);

        // Create texture object
        Texture planeTexture;
        planeTexture.id = textureID;
        planeTexture.type = "texture_diffuse";
        planeTexture.path = texturePath;

        // Return the mesh
        return Mesh(vertices, indices, {planeTexture});
    }

private:
    float width;
    float depth;
    std::string texturePath;

    unsigned int loadTexture(const std::string& path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrChannels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        } else {
            std::cout << "Failed to load texture: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
};

#endif // PLANE_H