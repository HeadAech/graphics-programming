//
// Created by Hubert Klonowski on 17/01/2025.
//

#ifndef ANIMATOR_H
#define ANIMATOR_H
#include <fstream>
#include <map>
#include <string>
#include <glm/vec3.hpp>

#include "Node.h"
#include "Util.h"

struct AnimationFrame {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    float timeToLerp;
};

struct Animation {
    std::string name;
    float duration;
    bool backwards = false;

    std::vector<AnimationFrame> frames;
};

class Animator {

    std::map<std::string, Animation> animations;
    std::map<std::string, Node*> nodes;
    std::map<std::string, int> currentFrames;

public:

    Animator(){};
    ~Animator(){};

    bool playing = false;

    void RecordKeyFrame(std::string label, std::vector<glm::vec3> transforms, float time) {
        std::ofstream file("res/animations/" + label + ".txt", std::ios::app);
        if (file.is_open()) {
            for (int i = 0; i < transforms.size(); i++) {
                file << transforms[i].x << " " << transforms[i].y << " " << transforms[i].z << std::endl;
            }
            file << time << std::endl;
        }

        file.close();
    }

    void PrepareAnimations(std::string label) {
        std::ifstream file("res/animations/" + label + ".txt", std::ios::in);

        int propertyIdx = 0;
        int coordinateIdx = 0;

        std::string line;

        Animation animation;

        AnimationFrame animationFrame;

        if (file.is_open()) {
            while (std::getline(file, line)) {
                std::string x;
                std::string y;
                std::string z;
                std::string time;
                if (line.find(' ') == std::string::npos) {
                    time = line;
                } else {
                    for (int i = 0; i < line.size(); i++) {
                        if (line[i] == ' ') {
                            coordinateIdx++;
                            continue;
                        }

                        if (coordinateIdx == 0) {
                            x += line[i];
                        } else if (coordinateIdx == 1) {
                            y += line[i];
                        } else if (coordinateIdx == 2) {
                            z += line[i];
                        }
                    }
                }
                if (propertyIdx == 0) {
                    animationFrame.position = glm::vec3(std::stof(x), std::stof(y), std::stof(z));
                } else if (propertyIdx == 1) {
                    animationFrame.rotation = glm::vec3(std::stof(x), std::stof(y), std::stof(z));
                } else if (propertyIdx == 2) {
                    animationFrame.scale = glm::vec3(std::stof(x), std::stof(y), std::stof(z));
                } else if (propertyIdx == 3) {
                    animationFrame.timeToLerp =  std::stof(time);
                    propertyIdx = -1;
                    animation.frames.push_back(animationFrame);
                }

                propertyIdx++;
                coordinateIdx = 0;
            }
        }

        if (animations.contains(label)) {
            animations[label] = animation;
        } else {
            animations.insert(std::pair(label, animation));
        }
    }

    void PlayAnimation(std::string label, Node* node, bool backwards = false) {
        nodes[label] = node;
        currentFrames[label] = backwards ? animations[label].frames.size() - 1 : 0;
        animations[label].backwards = backwards;
        globalTime = 0;
    }

    void StopAnimations() {
        for (std::map<std::string, Animation>::iterator it = animations.begin(); it != animations.end(); it++) {
            currentFrames[it->first] = 0;
            Node* node = nodes[it->first];

            if (node) {
                node->transform.setLocalPosition({0,0,0});
                node->transform.setEulerRotation({0,0,0});
                node->transform.setScale({1,1,1});
            }
        }
        nodes.clear();
    }

    float globalTime = 0.0f;

    void Update(float delta) {
        globalTime += delta;

        for (std::map<std::string, Animation>::iterator it = animations.begin(); it != animations.end(); it++) {
            Animation animation = it->second;
            std::string key = it->first;
            int currentFrame = currentFrames[key];

            Node* node = nodes[key];


            if (node != NULL) {
                if (currentFrame >= static_cast<int>(animation.frames.size())) {
                    currentFrames[key] = 0;
                    currentFrame = 0;
                }
                if (currentFrame < 0 && animation.backwards) {
                    currentFrames[key] = animation.frames.size() - 1;
                    currentFrame = animation.frames.size() - 1;
                }
                if (animation.frames.size() == 0) {
                    continue;
                }


                auto animationFrame = animation.frames[currentFrame];

                float t = animationFrame.timeToLerp * delta;

                node->transform.setLocalPosition(Util::lerp(node->transform.position, animationFrame.position, t));
                node->transform.setEulerRotation(Util::lerp(node->transform.eulerRotation, animationFrame.rotation, t));
                node->transform.setScale(Util::lerp(node->transform.scale, animationFrame.scale, t));

                if (compareVectors(node->transform.position, animationFrame.position, 0.1f)
                    && compareVectors(node->transform.eulerRotation, animationFrame.rotation, 0.1f)
                    && compareVectors(node->transform.scale, animationFrame.scale, 0.1f)) {
                    node->transform.setLocalPosition(animationFrame.position);
                    node->transform.setEulerRotation(animationFrame.rotation);
                    node->transform.setScale(animationFrame.scale);
                    if (animation.backwards) {
                        currentFrames[key]--;
                    }else {
                        currentFrames[key]++;
                    }
                }
            }
        }
    }

    bool compareVectors(glm::vec3 a, glm::vec3 b, float tolerance) {
        if (std::abs(a.x - b.x) < tolerance
            && std::abs(a.y - b.y) < tolerance
            && std::abs(a.z - b.z) < tolerance) {
            return true;
        }
        return false;
    }


};

#endif //ANIMATOR_H
