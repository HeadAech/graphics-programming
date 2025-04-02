//
// Created by Hubert Klonowski on 08/12/2024.
//

#ifndef UTIL_H
#define UTIL_H
#include <cmath>

static class Util {
public:

    static double round_up(double value, int decimal_places) {
        const double multiplier = std::pow(10.0, decimal_places);
        return std::ceil(value * multiplier) / multiplier;
    }

    static std::string format(float value, int precision) {
        std::stringstream ss;
        ss << std::fixed  << std::setprecision(precision) << value;
        return ss.str();
    }


    static glm::vec3 getEulerAnglesFromDirection(glm::vec3 direction) {
        direction = glm::normalize(-direction);
        float yaw = atan2(direction.x, direction.z); // Yaw
        float pitch = atan2(direction.y, sqrt(direction.x * direction.x + direction.z * direction.z));
        float roll = 0.0f;

        return {glm::degrees(pitch), glm::degrees(yaw), glm::degrees(roll)}; // Pitch, Yaw, Roll
    }


    static glm::vec3 getDirectionFromEulerAngles(float pitch, float yaw, float roll = 0.0f) {
        pitch = glm::radians(pitch);
        yaw = glm::radians(yaw);
        roll = glm::radians(roll);
        glm::mat4 rotationYaw = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationPitch = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotationRoll = glm::rotate(glm::mat4(1.0f), roll, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec4 forward(0.0f, 0.0f, -1.0f, 1.0f);

        // Combine the rotations: yaw * pitch * roll (order matters)
        glm::mat4 rotationMatrix = rotationYaw * rotationPitch * rotationRoll;

        // Apply the combined rotation to the default forward vector
        glm::vec4 rotated = rotationMatrix * forward;

        // Return the normalized direction vector
        return glm::normalize(-glm::vec3(rotated));
    }

    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t) {
        return a + t * (b - a);
    }
};

#endif //UTIL_H
