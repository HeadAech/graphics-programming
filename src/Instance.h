//
// Created by Hubert Klonowski on 07/12/2024.
//

#ifndef INSTANCE_H
#define INSTANCE_H
#include "InstanceManager.h"
#include "Model.h"

class Instance {
public:
    int id = 0;
    Model& model;
    glm::mat4 modelMatrix;
    unsigned int buffer;

    InstanceManager* parentManager = nullptr;

    Instance(Model& model, int id): model(model) {
        this->id = id;
    }


    void Draw(Shader& shader) {
        shader.use();
        shader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.textureLoaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for (unsigned int i = 0; i < model.meshes.size(); i++)
        {
            glBindVertexArray(model.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(model.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, 2);
            glBindVertexArray(0);
        }
    }

};

#endif //INSTANCE_H
