//
// Created by Hubert Klonowski on 12/11/2024.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H

class Transform {
public:
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 eulerRotation = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    glm::vec3 globalPosition = glm::vec3(0.0f);

    //Dirty flag
    bool m_isDirty = true;

    glm::mat4 getLocalModelMatrix()
    {
        const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
                    glm::radians(eulerRotation.x),
                    glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
                    glm::radians(eulerRotation.y),
                    glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
                    glm::radians(eulerRotation.z),
                    glm::vec3(0.0f, 0.0f, 1.0f));

        // Y * X * Z
        const glm::mat4 rotationMatrix = transformY * transformX * transformZ;

        // translation * rotation * scale (also know as TRS matrix)
        return glm::translate(glm::mat4(1.0f), position) *
                    rotationMatrix *
                    glm::scale(glm::mat4(1.0f), scale);
    }


    void computeModelMatrix()
    {
        modelMatrix = getLocalModelMatrix();
        m_isDirty = false;
    }

    void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
    {
        modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
        m_isDirty = false;
        globalPosition = glm::vec3(parentGlobalModelMatrix * glm::vec4(position, 1.0f));
    }


    void setLocalPosition(const glm::vec3& newPosition)
    {
        position = newPosition;
        m_isDirty = true;
    }

    void setScale(const glm::vec3& newScale) {
        scale = newScale;
        m_isDirty = true;
    }

    void setEulerRotation(const glm::vec3& newEulerRotation) {
        eulerRotation = newEulerRotation;
        m_isDirty = true;
    }

    const glm::vec3& getEulerRotation() {
        return eulerRotation;
    }


    const glm::vec3& getLocalPosition()
    {
        return position;
    }

    const glm::mat4& getModelMatrix()
    {
        return modelMatrix;
    }

    const void setModelMatrix(glm::mat4& m) {
        modelMatrix = m;
        m_isDirty = false;
    }

    bool isDirty()
    {
        return m_isDirty;
    }

    const glm::vec3& getGlobalPosition() const {
        return globalPosition;
    }

    void setGlobalPosition(glm::vec3 globalPosition) {
        this->globalPosition = globalPosition;
    }

    void MoveLocalPosition(glm::vec3 offset) {
        this->setLocalPosition(position + offset);
    }

    void Rotate(glm::vec3 offset) {
        this->setEulerRotation(eulerRotation + offset);
    }
};

#endif //TRANSFORM_H
