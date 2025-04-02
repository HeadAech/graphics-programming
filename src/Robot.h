//
// Created by Hubert Klonowski on 17/01/2025.
//

#ifndef ROBOT_H
#define ROBOT_H
#include "Node.h"

class Robot {
public:

    Robot() {}

    float speed = 35;
    float rotationSpeed = 80;
    float slowRate;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    void Update(float delta) {

        velocity += acceleration * delta;
        velocity *= slowRate;

    }
};

#endif //ROBOT_H
