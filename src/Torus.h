//
// Created by Hubert Klonowski on 12/11/2024.
//

#ifndef TORUS_H
#define TORUS_H
#include <stb_image.h>
#include <iostream>
#include <string>
#include <glad/glad.h>

#include "Mesh.h"

class Torus {

public:
    Torus(std::string texturePath, float majorRadius, float minorRadius, int numSegments, int numRings) {
        this->texturePath = texturePath;
        this->majorRadius = majorRadius;
        this->minorRadius = minorRadius;
        this->numSegments = numSegments;
        this->numRings = numRings;
    };

  // Modified generateTorusMesh to load and apply texture
    Mesh generateTorusMesh() {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // Loop through each segment and ring to generate vertices
        for (int i = 0; i <= numSegments; ++i) {
            float theta = i * 2.0f * M_PI / numSegments;
            float cosTheta = cos(theta);
            float sinTheta = sin(theta);

            for (int j = 0; j <= numRings; ++j) {
                float phi = j * 2.0f * M_PI / numRings;
                float cosPhi = cos(phi);
                float sinPhi = sin(phi);

                // Calculate vertex position
                Vertex vertex;
                vertex.position = glm::vec3(
                    (majorRadius + minorRadius * cosPhi) * cosTheta,
                    minorRadius * sinPhi,
                    (majorRadius + minorRadius * cosPhi) * sinTheta
                );

                // Calculate vertex normal
                glm::vec3 center = glm::vec3(majorRadius * cosTheta, 0.0f, majorRadius * sinTheta);
                vertex.normal = glm::normalize(vertex.position - center);

                // Texture coordinates (optional, can be used for texturing)
                vertex.texCoords = glm::vec2((float)i / numSegments, (float)j / numRings);

                vertices.push_back(vertex);
            }
        }

        // Generate indices for triangle strip
        for (int i = 0; i < numSegments; ++i) {
            for (int j = 0; j < numRings; ++j) {
                int first = (i * (numRings + 1)) + j;
                int second = first + numRings + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        // Load the texture (e.g., "stone.jpg")
        unsigned int textureID = loadTexture(texturePath);

        // Create a Texture object for the loaded texture
        Texture torusTexture;
        torusTexture.id = textureID;
        torusTexture.type = "texture_diffuse";
        torusTexture.path = "stone.jpg";  // Path to the texture file

        // Return the Mesh with the loaded texture
        return Mesh(vertices, indices, {torusTexture});
    }

    void setSegmentsNumber(int s) {
        this->numSegments = s;
    }

    void setRingsNumber(int s) {
        this->numRings = s;
    }

    int getSegmentsNumber() {
        return this->numSegments;
    }

    int getRingsNumber() {
        return this->numRings;
    }

    Mesh generateGeometryShaderMesh() {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // Create a single vertex at origin for the geometry shader
        Vertex vertex;
        vertex.position = glm::vec3(0.0f, 0.0f, 0.0f);
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertex.texCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
        indices.push_back(0);

        // Load the texture
        unsigned int textureID = loadTexture(texturePath);
        Texture torusTexture;
        torusTexture.id = textureID;
        torusTexture.type = "texture_diffuse";
        torusTexture.path = texturePath;

        return Mesh(vertices, indices, {torusTexture});
    }

  private:
    float majorRadius;
    float minorRadius;
    int numSegments;
    int numRings;

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

#endif //TORUS_H
