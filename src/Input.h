//
// Created by Hubert Klonowski on 07/12/2024.
//

#ifndef INPUT_H
#define INPUT_H
#include <map>

#include "GLFW/glfw3.h"

class Input {

        std::map<int, bool> currentKeys;
        std::map<int, bool> previousKeys;

public:

    // Check if a key is currently pressed
    bool isKeyPressed(int key) {
        return currentKeys[key];
    }

    // Check if a key was just pressed (transition from RELEASED to PRESSED)
    bool isKeyJustPressed(int key) {
        return currentKeys[key] && !previousKeys[key];
    }

    // Check if a key was just released (transition from PRESSED to RELEASED)
    bool isKeyJustReleased(int key) {
        return !currentKeys[key] && previousKeys[key];
    }

    // Check if a key is currently released
    bool isKeyReleased(int key) {
        return !currentKeys[key];
    }

    // Process key states (update current and previous states)
    void processInput(GLFWwindow* window) {
        for (auto& [key, state] : currentKeys) {
            // Update the previous state
            previousKeys[key] = state;

            // Get the current state from GLFW
            currentKeys[key] = glfwGetKey(window, key) == GLFW_PRESS;
        }
    }

    // Add a key to track its state
    void addKey(int key) {
        currentKeys[key] = false;
        previousKeys[key] = false;
    }
};

#endif //INPUT_H
