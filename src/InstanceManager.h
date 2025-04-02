//
// Created by Hubert Klonowski on 07/12/2024.
//

#ifndef INSTANCEMANAGER_H
#define INSTANCEMANAGER_H
#include "Node.h"

class InstanceManager {


public:
    std::vector<glm::mat4> modelMatrices;
    Model& model;
    bool isDirty = false;
    unsigned int buffer;


    InstanceManager(Model& model): model(model) {
    }

    void addMatrix(glm::mat4 m) {
        modelMatrices.push_back(m);
    }

    void setModel(Model& model) {
        this->model = model;
    }

    void updateModelMatrix(int id, glm::mat4 m) {
        modelMatrices[id] = m;
    }

    void instantiate() {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

        for (unsigned int i = 0; i < model.meshes.size(); i++)
        {
            unsigned int VAO = model.meshes[i].VAO;
            glBindVertexArray(VAO);
            // set attribute pointers for matrix (4 times vec4)
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);

            glBindVertexArray(0);
        }

        std::cout << "Instantiated instance " << std::endl;
    }

    void updateBuffer() {
        if (!isDirty) return;

        glBindBuffer(GL_ARRAY_BUFFER, buffer); // Ensure the buffer is bound
        glBufferSubData(GL_ARRAY_BUFFER, 0, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0]);

        isDirty = false; // Reset the flag
    }

    void Draw(Shader* shader) {
        shader->use();
        // shader->setInt("texture_diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.textureLoaded[0].id);

        updateBuffer();

        // std::cout << "drawing for " << modelMatrices.size() << std::endl;
        for (unsigned int i = 0; i < model.meshes.size(); i++)
        {
            glBindVertexArray(model.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(model.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, modelMatrices.size());
            glBindVertexArray(0);
        }
    }

};

#endif //INSTANCEMANAGER_H
