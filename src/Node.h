//
// Created by Hubert Klonowski on 12/11/2024.
//

#ifndef NODE_H
#define NODE_H
#include <list>
#include <memory>
#include <random>
#include <glm/ext/matrix_transform.hpp>
#include <vector>

#include "Instance.h"
#include "Light.h"
#include "Transform.h"



class Node {

    bool stationary = true;
    std::string label = "Node";

public:

    std::vector<std::unique_ptr<Node>> children;
    // std::vector<Node*> children;
    Node* parent;


    Transform transform;

    Model* model = nullptr;
    Instance* instance = nullptr;

    bool visible = true;

    bool* wireframe = nullptr;

    Light* light = nullptr;

    Material material = STANDARD;

    std::vector<std::string> materialMap = {
        "Standard",
        "Reflective",
        "Refractive"
    };


    Node(Model* model = nullptr, const Transform& transform = Transform())
    : parent(nullptr), transform(transform) {
        if (model != nullptr) {
            this->model = model;
        }
        children = std::vector<std::unique_ptr<Node>>();
        // children = std::vector<Node*>();
        setRandomRotationSpeed();
    }

    void toggleVisibility() {
        visible = !visible;
    }

    void setVisible(bool v) {
        visible = v;
    }

    void setLight(Light* l) {
        light = l;
    }

    bool isVisible() const {
        return visible;
    }

    void setModel(Model* model) {
        this->model = model;
    }

    void setInstance(Instance* instance) {
        this->instance = instance;
    }

    void setLabel(std::string l) {
        this->label = l;
    }

    char* getLabel() {
        return this->label.data();
    }

    void setStationary(bool value) {
        stationary = value;
    }

    bool isStationary() {
        return stationary;
    }

    bool isInstanced() {
        return instance;
    }


    // std::vector<std::unique_ptr<Node>> getChildren() {
    //     return this->children;
    // }

    Node *getChild(int index) const {
        if (index < 0 || index >= children.size()) return nullptr;
        return this->children[index].get();
    }

    Node *getLastChild() const {
        return children.back().get();
    }


    template<typename... TArgs>
    void addChild(const TArgs&... args) {
        children.push_back(std::make_unique<Node>(args...));
        children.back()->parent = this;
        children.back()->wireframe = wireframe;
    }

    void addChild(Node* node) {
        children.push_back(std::unique_ptr<Node>(node));
        children.back()->parent = this;
        children.back()->wireframe = wireframe;
    }

    void addChild(Model* model) {
        children.push_back(std::make_unique<Node>(model));
        children.back()->parent = this;
        children.back()->wireframe = wireframe;
    }

    void addChild(std::unique_ptr<Node> node) {
        node->parent = this;
        children.push_back(std::move(node));
        children.back()->wireframe = wireframe;
    }


    void setParent(Node* newParent) {
        if (parent) {
            // find and remove current node from original parent's position
            auto& siblings = parent->children;
            auto it = std::find_if(siblings.begin(), siblings.end(),
                [this](const std::unique_ptr<Node>& child) {
                    return child.get() == this;
                });

            if (it != siblings.end()) {
                // change ownership to new parent
                std::unique_ptr<Node> movingNode = std::move(*it);
                siblings.erase(it);

                // Add the node to the new parent's children
                newParent->addChild(std::move(movingNode));
            }
        } else {
            newParent->addChild(std::unique_ptr<Node>(this));
        }
        this->parent = newParent;
    }

    void removeChild(Node* node) {
        for (int i = 0; i < children.size(); i++) {
            if (children[i]->label == node->label) {
                children.erase(children.begin() + i);
            }
        }
    }


    void updateSelfAndChild(float& deltaTime) {
        Update(deltaTime);
        if (transform.isDirty()) {
            forceUpdateSelfAndChild(deltaTime);
            return;
        }

        for (const std::unique_ptr<Node>& child : children)
        {
            child->updateSelfAndChild(deltaTime);
        }
    }

    void forceUpdateSelfAndChild(float& deltaTime) {
        if (parent) {
            Update(deltaTime);
            transform.computeModelMatrix(parent->transform.getModelMatrix());
            if (instance) {
                instance->modelMatrix = transform.getModelMatrix();
                instance->parentManager->updateModelMatrix(instance->id, transform.getModelMatrix());
                instance->parentManager->isDirty = true;
            }
        }
        else
            transform.computeModelMatrix();

        for (const std::unique_ptr<Node>& child : children)
        {
            child->forceUpdateSelfAndChild(deltaTime);
        }

    }

    void Draw(Shader* shader, unsigned int& cubemapTexture) {
        if (!visible) return;

        if (*wireframe == true) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (model) {

            model->Draw(shader, cubemapTexture);
        }
    }

    Node* find(std::string l) {
        if (getLabel() == l) return this;  // return the current node if it matches the label

        // Recursively search in children
        for (auto&& child : children) {
            Node* result = child->find(l);  // Recursively search in the child
            if (result != nullptr) {
                return result;  // If found, return the result
            }
        }

        return nullptr;
    }

    float rotationSpeed = 0;

    void setRandomRotationSpeed() {

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(2, 5);

        rotationSpeed = dis(gen);
        // std::cout << "Rotation speed set to: " << rotationSpeed << std::endl;
    }

    void setRandomRotation() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-5, 5);

        transform.setEulerRotation({dis(gen), 0.f, -dis(gen)});
    }

    void Update(float deltaTime) {
        // std::cout << "Updating node" << std::endl;
        if (light) {
            // if (light->type == POINT) {
            light->setForTypeVec3("position", transform.getGlobalPosition());
            // }
        }
        if (stationary) return;

        glm::vec3 rotation = transform.getEulerRotation();
        rotation.y += 10.0f * deltaTime * rotationSpeed;
        if (rotation.y > 360) rotation.y = 0;
        transform.setEulerRotation(rotation);

    }

    void setMaterial(Material material) {
        this->material = material;
    }

};

#endif //NODE_H
