//
// Created by Hubert Klonowski on 07/12/2024.
//

#ifndef LIGHT_H
#define LIGHT_H
#include <glm/vec3.hpp>

enum Type {
    DIRECTIONAL, SPOTLIGHT, POINT
};

class Light {
public:
    int id = 0;
    bool active = true;
    glm::vec3 direction;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float cutOff = glm::cos(glm::radians(12.92));
    float outerCutOff = glm::cos(glm::radians(17.29));

    float intensity = 1.0f;

    Type type = Type::DIRECTIONAL;

    Shader* shader;
    Shader* regularShader;

public:
    Light(Type type, Shader* shader, Shader* regularShader, int id) {
        this->shader = shader;
        this->regularShader = regularShader;
        this->type = type;
        this->id = id;
    }
    //TODO UPDATE DLA REGULAR SHADERA
    void setForTypeVec3(std::string property, glm::vec3 value) const {
        shader->use();
        switch (type) {
            case DIRECTIONAL:
                shader->setVec3("dirLight." + property, value);
                break;
            case SPOTLIGHT:
                shader->setVec3("spotLights[" + std::to_string(id) + "]."+ property, value);
                break;
            case POINT:
                shader->setVec3("pointLights[" + std::to_string(id) + "]."+ property, value);
                break;
        }
        regularShader->use();
        switch (type) {
            case DIRECTIONAL:
                regularShader->setVec3("dirLight." + property, value);
                break;
            case SPOTLIGHT:
                regularShader->setVec3("spotLights[" + std::to_string(id) + "]."+ property, value);
                break;
            case POINT:
                regularShader->setVec3("pointLights[" + std::to_string(id) + "]."+ property, value);
                break;
        }
    }

    void setForTypeFloat(std::string property, float value) const {
        shader->use();
        switch (type) {
            case DIRECTIONAL:
                shader->setFloat("dirLight." + property, value);
                break;
            case SPOTLIGHT:
                shader->setFloat("spotLights[" + std::to_string(id) + "]."+ property, value);
                break;
            case POINT:
                shader->setFloat("pointLights[" + std::to_string(id) + "]."+ property, value);
                break;
        }
        regularShader->use();
        switch (type) {
            case DIRECTIONAL:
                regularShader->setFloat("dirLight." + property, value);
                break;
            case SPOTLIGHT:
                regularShader->setFloat("spotLights[" + std::to_string(id) + "]."+ property, value);
                break;
            case POINT:
                regularShader->setFloat("pointLights[" + std::to_string(id) + "]."+ property, value);
                break;
        }
    }

    void setForTypeBool(std::string property, bool value) const {
        shader->use();
        switch (type) {
            case DIRECTIONAL:
                shader->setBool("dirLight." + property, value);
                break;
            case SPOTLIGHT:
                shader->setBool("spotLights[" + std::to_string(id) + "]."+ property, value);
                break;
            case POINT:
                shader->setBool("pointLights[" + std::to_string(id) + "]."+ property, value);
                break;
        }
        regularShader->use();
        switch (type) {
            case DIRECTIONAL:
                regularShader->setBool("dirLight." + property, value);
                break;
            case SPOTLIGHT:
                regularShader->setBool("spotLights[" + std::to_string(id) + "]."+ property, value);
                break;
            case POINT:
                regularShader->setBool("pointLights[" + std::to_string(id) + "]."+ property, value);
                break;
        }
    }

    void setActive(bool v) {
        active = v;
        setForTypeBool("isOn", v);
    }

    void setIntensity(float v) {
        this->intensity = v;
        setForTypeFloat("intensity", v);
    }

    void setDirection(glm::vec3 direction) {
        this->direction = direction;
        setForTypeVec3("direction", direction);

    }

    void setDiffuse(const glm::vec3 diffuse) {
        this->diffuse = diffuse;
        setForTypeVec3("diffuse", diffuse);
    }

    void setSpecular(const glm::vec3 specular) {
        this->specular = specular;
        setForTypeVec3("specular", specular);
    }

    void setAmbient(const glm::vec3 ambient) {
        this->ambient = ambient;
        setForTypeVec3("ambient", ambient);
    }

    void setConstant(const float constant) {
        this->constant = constant;
        setForTypeFloat("constant", constant);
    }

    void setLinear(const float linear) {
        this->linear = linear;
        setForTypeFloat("linear", linear);
    }

    void setQuadratic(const float quadratic) {
        this->quadratic = quadratic;
        setForTypeFloat("quadratic", quadratic);
    }

    void setCutOff(const float cutOff) {
        this->cutOff = cutOff;
        setForTypeFloat("cutOff", cutOff);
    }

    void setType(const Type type) {
        this->type = type;
    }

    void setOuterCutOff(const float outerCutOff) {
        this->outerCutOff = outerCutOff;
        setForTypeFloat("outerCutOff", outerCutOff);
    }

    glm::vec3 getDirection() {
        return direction;
    }


    glm::vec3 getDiffuse() {
        return diffuse;
    }

    glm::vec3 getSpecular() {
        return specular;
    }

    glm::vec3 getAmbient() {
        return ambient;
    }

    float getConstant() const {
        return constant;
    }

    float getLinear() const {
        return linear;
    }

    float getQuadratic() const {
        return quadratic;
    }

    float getCutOff() const {
        return cutOff;
    }

    float getOuterCutOff() const {
        return outerCutOff;
    }

    float getIntensity() const {
        return intensity;
    }

    void setCutOffDegrees(float degrees) {
        this->cutOff = glm::cos(glm::radians(degrees));
    }

    void setOuterCutOffDegrees(float degrees) {
        this->outerCutOff = glm::cos(glm::radians(degrees));
    }

    Type getType() {
        return type;
    }




};

#endif //LIGHT_H
