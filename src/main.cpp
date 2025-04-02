// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"
#include <stdio.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <complex>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>  // Initialize with gladLoadGL()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

#include "Camera.h"
#include "Model.h"
#include "Shader.h"

#include <iostream>
#include <list>
#include <thread>

#include "Animator.h"
#include "Input.h"
#include "Node.h"
#include "Plane.h"
#include "Robot.h"
#include "Skybox.h"
#include "Torus.h"
#include "Util.h"
#include "MeshInstance/MeshInstance.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool init();
void init_imgui();

void handle_input(GLFWwindow *window);
void update();
void render();
void renderEntityAndChildren(Node* entity);
void setUpLights(Model& pointLightModel, Model& spotLightModel, Model& dirLightModel);
void renderLights();
void setupShaders();

void imgui_begin();
void imgui_render();
void imgui_end();

void end_frame();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void calculateAndDisplayFPS() ;
constexpr int32_t WINDOW_WIDTH  = 1100;
constexpr int32_t WINDOW_HEIGHT = 700;

GLFWwindow* window = nullptr;

// Change these to lower GL version like 4.5 if GL 4.6 can't be initialized on your machine
const     char*   glsl_version     = "#version 410";
constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 1;

// Camera
Camera camera(glm::vec3(0.0f, 16.0f, 44.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing

float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Models
std::vector<Model> models;

Node* root;

bool cursorLocked = false;

bool wireframe = false;

glm::vec3 rootRotation = glm::vec3(0.0f);

std::vector<InstanceManager*> instances;

Input Input;

std::vector<Node*> pointLights;
std::vector<Node*> spotLights;

std::vector<glm::vec3> lightColors = {
    {0.4f, 0.4f, 0.4f},
    {0, 0, 1.0f},
    {1.0f, 0, 0}
};

float FPS;

Shader* advancedShader;
Shader* regularShader;
Shader* emissionShader;
Shader* skyboxShader;
Shader* reflectiveShader;
Shader* refractiveShader;
Shader* testShader;
glm::vec3 ior = {1.52f, 1.50f, 1.48f};
float chromaticAbberationStrength = 0.02;

Node* flashlightNode;

Skybox* skybox;

Animator* animator;

bool controllingRobot = false;
Node* cameraHandleForRobot;

Robot* robot;
Node* headNode;
Node* torsoNode;


GLuint framebuffer, colorTexture, depthTexture;
int main(int, char**)
{
    if (!init())
    {
        spdlog::error("Failed to initialize project!");
        return EXIT_FAILURE;
    }
    spdlog::info("Initialized project.");

    init_imgui();
    spdlog::info("Initialized ImGui.");

    stbi_set_flip_vertically_on_load(true);


    regularShader = new Shader("res/shaders/basic.vert", "res/shaders/blinnphong/shader.frag");
    advancedShader = new Shader("res/shaders/blinnphong/shader.vert", "res/shaders/blinnphong/shader.frag");
    emissionShader = new Shader("res/shaders/emission/shader.vert", "res/shaders/emission/shader.frag");
    skyboxShader = new Shader("res/shaders/skybox/shader.vert", "res/shaders/skybox/shader.frag");
    reflectiveShader = new Shader("res/shaders/reflective/shader.vert", "res/shaders/reflective/shader.frag");
    refractiveShader = new Shader("res/shaders/refractive/shader.vert", "res/shaders/refractive/shader.frag");
    testShader = new Shader("res/shaders/test.vert", "res/shaders/test.frag");

    camera.setPitch(-20.0f);

    Input.addKey(GLFW_KEY_Q);
    Input.addKey(GLFW_KEY_W);
    Input.addKey(GLFW_KEY_S);
    Input.addKey(GLFW_KEY_D);
    Input.addKey(GLFW_KEY_A);
    Input.addKey(GLFW_KEY_E);
    Input.addKey(GLFW_KEY_R);
    Input.addKey(GLFW_KEY_Z);
    Input.addKey(GLFW_KEY_C);
    Input.addKey(GLFW_KEY_X);
    Input.addKey(GLFW_KEY_ESCAPE);
    Input.addKey(GLFW_KEY_LEFT_SHIFT);


    Model houseBody("res/models/house/body/housebody.obj");
    Model houseRoof("res/models/house/roof/roof.obj");

    auto* houseInstances = new InstanceManager(houseBody);
    auto* houseRoofInstances = new InstanceManager(houseRoof);


    root = new Node();
    root->setLabel("Root");
    root->setStationary(true);
    root->wireframe = &wireframe;


    // ---------_SPAWN HOUSES_-----------


    int width = 2;
    int height = 2;
    int separation = 10;

    Model ground("res/models/ground/ground.obj");
    // Plane plane("res/textures/stone.jpg", width * separation, height * separation);
    // Model ground(plane.generatePlaneMesh());

    // std::unique_ptr<Node> groundNode = std::make_unique<Node>(&ground);
    // groundNode->setLabel("Ground");

    Node* groundNode = new Node(&ground);
    groundNode->setLabel("Ground");
    groundNode->setStationary(true);
    groundNode->transform.setScale({200, 1, 200 });

    root->addChild(groundNode);

    Model backpackreflective("res/models/cube/cube.obj");
    Node* backpackreflectiveNode = new Node(&backpackreflective);
    backpackreflectiveNode->setMaterial(REFLECTIVE);

    backpackreflectiveNode->setLabel("Backpack reflective");
    backpackreflectiveNode->setStationary(true);
    backpackreflectiveNode->transform.setLocalPosition({0, 10, 0});
    backpackreflectiveNode->transform.setScale({5, 5, 5 });

    root->addChild(backpackreflectiveNode);

    Model backpackRefractive("res/models/cube/cube.obj");
    Node* backpackRefractiveNode = new Node(&backpackRefractive);
    backpackRefractiveNode->setMaterial(REFRACTIVE);
    backpackRefractiveNode->setLabel("Backpack refractive");
    backpackRefractiveNode->setStationary(true);
    backpackRefractiveNode->transform.setScale({5, 5, 5});
    backpackRefractiveNode->transform.setLocalPosition({30, 10, 0});
    root->addChild(backpackRefractiveNode);

    //lights
    Model bulb("res/models/sun/sun.obj");
    Model flashlight("res/models/flashlight/flashlight.obj");
    Model arrow("res/models/arrow/arrow.obj");

    setupShaders();

    setUpLights(bulb, flashlight, arrow);

    int id = 0;
    int rowNum = 0;

    std::unique_ptr<Node> housesNode = std::make_unique<Node>();
    housesNode->setLabel("--Houses--");
    housesNode->setStationary(true);
    housesNode->transform.setLocalPosition({0, 2, 0});

    root->addChild(std::move(housesNode));

    Node* housesNodeP = root->getLastChild();

    for ( int x = width/2 * -1; x < width/2; x++)
    {
        std::unique_ptr<Node> row = std::make_unique<Node>();
        row->setLabel("Rows " + std::to_string(rowNum));


        housesNodeP->addChild(std::move(row));
        Node* lastRowNode = housesNodeP->getLastChild();
        for ( int y = height/2 * -1; y < height/2; y++) {


            // Node* n = new Node();
            std::unique_ptr<Node> n = std::make_unique<Node>();
            n->setStationary(true);
            n->setLabel("House " + std::to_string(id));
            glm::vec3 pos = n->transform.getLocalPosition();
            pos.x += separation * x;
            pos.z += separation * y;
            n->transform.setScale({2, 2, 2});
            n->transform.setLocalPosition(pos);
            n->transform.computeModelMatrix();

            std::unique_ptr<Node> body = std::make_unique<Node>();
            // Node* body = new Node();
            body->setStationary(true);
            body->setLabel("Body");
            body->transform.computeModelMatrix(n->transform.getModelMatrix());

            std::unique_ptr<Node> roof = std::make_unique<Node>();
            // Node* roof = new Node();
            roof->setStationary(true);
            roof->setLabel("Roof");

            glm::vec3 roofPos = roof->transform.getLocalPosition();
            roofPos.y += 2;
            roof->transform.setLocalPosition(roofPos);
            roof->transform.computeModelMatrix(n->transform.getModelMatrix());

            auto* hBodyI = new Instance(houseBody, id);
            auto* hRoofI = new Instance(houseRoof, id);
            id++;

            hBodyI->modelMatrix = body->transform.getModelMatrix();
            hRoofI->modelMatrix = roof->transform.getModelMatrix();

            body->instance = hBodyI;
            roof->instance = hRoofI;

            // root->addChild(n);
            lastRowNode->addChild(std::move(n));
            hBodyI->parentManager = houseInstances;
            hRoofI->parentManager = houseRoofInstances;

            houseInstances->addMatrix(hBodyI->modelMatrix);
            houseRoofInstances->addMatrix(hRoofI->modelMatrix);
            Node* lastEntity = lastRowNode->getLastChild();

            // lastEntity->addChild(body);
            // lastEntity->addChild(roof);
            lastEntity->addChild(std::move(body));
            lastEntity->addChild(std::move(roof));

        }

        rowNum++;
    }

    // -----------------------------

    instances.push_back(houseInstances);
    instances.push_back(houseRoofInstances);

    houseInstances->instantiate();
    houseRoofInstances->instantiate();

    root->updateSelfAndChild(deltaTime);


    GLint maxVertices;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxVertices);
    std::cout << "Max vertices allowed by hardware: " << maxVertices << std::endl;

    GLint maxComponents;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &maxComponents);
    std::cout << "Max components allowed by hardware: " << maxComponents << std::endl;

    std::vector<std::string> skyboxFaces = {
        "res/textures/skybox/right.jpg",
        "res/textures/skybox/left.jpg",
        "res/textures/skybox/top.jpg",
        "res/textures/skybox/bottom.jpg",
        "res/textures/skybox/front.jpg",
        "res/textures/skybox/back.jpg"
    };

    skybox = new Skybox(skyboxShader, skyboxFaces);

    reflectiveShader->use();
    reflectiveShader->setInt("skybox", 0);

    std::vector<std::string> robotBones = {
        "Head",
        "Torso",
        "LeftArm",
        "RightArm",
        "LeftForearm",
        "RightForearm",
        "LeftLeg",
        "RightLeg",
        "LeftThigh",
        "RightThigh",
        "Screen",
        "Visor"

    };

    Model head("res/models/robot/head.obj");
    headNode = new Node(&head);
    headNode->setLabel("Head");

    Model visor("res/models/robot/visor.obj");
    Node* visorNode = new Node(&visor);
    visorNode->setLabel("Visor");
    visorNode->setMaterial(REFLECTIVE);

    Model torso("res/models/robot/torso.obj");
    torsoNode = new Node(&torso);
    torsoNode->transform.setLocalPosition({0, 9, 30});
    torsoNode->setLabel("Torso");

    Model screen("res/models/robot/screen.obj");
    Node* screenNode = new Node(&screen);
    screenNode->setLabel("Screen");
    screenNode->setMaterial(REFRACTIVE);

    Model leftArm("res/models/robot/left_arm.obj");
    Node* leftArmNode = new Node(&leftArm);
    leftArmNode->setLabel("LeftArm");

    Model rightArm("res/models/robot/right_arm.obj");
    Node* rightArmNode = new Node(&rightArm);
    rightArmNode->setLabel("RightArm");

    Model leftLeg("res/models/robot/left_leg.obj");
    Node* leftLegNode = new Node(&leftLeg);
    leftLegNode->setLabel("LeftLeg");

    Model rightLeg("res/models/robot/right_leg.obj");
    Node* rightLegNode = new Node(&rightLeg);
    rightLegNode->setLabel("RightLeg");

    Model leftForearm("res/models/robot/left_forearm.obj");
    Node* leftForearmNode = new Node(&leftForearm);
    leftForearmNode->setLabel("LeftForearm");

    Model rightForearm("res/models/robot/right_forearm.obj");
    Node* rightForearmNode = new Node(&rightForearm);
    rightForearmNode->setLabel("RightForearm");

    Model leftThigh("res/models/robot/left_thigh.obj");
    Node* leftThighNode = new Node(&leftThigh);
    leftThighNode->setLabel("LeftThigh");

    Model rightThigh("res/models/robot/right_thigh.obj");
    Node* rightThighNode = new Node(&rightThigh);
    rightThighNode->setLabel("RightThigh");

    cameraHandleForRobot = new Node();
    cameraHandleForRobot->setLabel("CameraHandleForRobot");
    cameraHandleForRobot->transform.setLocalPosition({0, 10, -17});

    root->addChild(torsoNode);
    torsoNode->addChild(headNode);
    torsoNode->addChild(leftArmNode);
    torsoNode->addChild(rightArmNode);
    torsoNode->addChild(leftThighNode);
    torsoNode->addChild(rightThighNode);
    torsoNode->addChild(screenNode);
    //for camera to follow the robot
    torsoNode->addChild(cameraHandleForRobot);

    headNode->addChild(visorNode);

    leftArmNode->addChild(leftForearmNode);
    rightArmNode->addChild(rightForearmNode);

    leftThighNode->addChild(leftLegNode);
    rightThighNode->addChild(rightLegNode);


    robot = new Robot();

    animator = new Animator();

    for (auto bone: robotBones) {
        animator->PrepareAnimations("robot/" + bone);
    }

    //3D
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create color attachment
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_WIDTH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    // Create depth attachment
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_WIDTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    // Check framebuffer
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    MeshInstance* meshInstance = new MeshInstance();
    meshInstance->LoadModel("res/models/cube/cube.obj");
    meshInstance->SetShader(testShader);


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        calculateAndDisplayFPS();


        Input.processInput(window);
        // Process I/O operations here
        handle_input(window);

        // Update game objects' state here
        update();

        // OpenGL rendering code here
        render();
        houseInstances->Draw(advancedShader);
        houseRoofInstances->Draw(advancedShader);

        // regularShader->use();
        // glm::mat4 modelMatrix = glm::mat4(1.0f);
        // modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 40, 0));
        // modelMatrix = glm::scale(modelMatrix, glm::vec3(4, 1, 1));
        // regularShader->setMat4("model", modelMatrix);
        meshInstance->Render();
        // cubeInstances->Draw(instanceShader);


        // Draw ImGui
        imgui_begin();
        imgui_render(); // edit this function to add your own ImGui controls
        imgui_end(); // this call effectively renders ImGui

        // End frame and swap buffers (double buffering)
        end_frame();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    delete advancedShader;
    delete regularShader;
    delete emissionShader;

    return 0;
}

void calculateAndDisplayFPS() {
    static int frameCount = 0;
    static auto lastTime = std::chrono::high_resolution_clock::now();

    frameCount++;

    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsedTime = currentTime - lastTime;

    if (elapsedTime.count() >= .5f) { // If one second has passed
        float fps = frameCount / elapsedTime.count();

        // Reset for the next calculation
        frameCount = 0;
        lastTime = currentTime;
        FPS = fps;
    }
}

bool init()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        spdlog::error("Failed to initalize GLFW!");
        return false;
    }

    // GL 4.6 + GLSL 460
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PAG 3 - Budget Osiedle", NULL, NULL);
    if (window == NULL)
    {
        spdlog::error("Failed to create GLFW Window!");
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1); // Enable VSync - fixes FPS at the refresh rate of your screen

    bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (err)
    {
        spdlog::error("Failed to initialize OpenGL loader!");
        return false;
    }

    return true;
}

void init_imgui()
{
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();

}

void setCursorLocked(const bool cursorLocked) {
    if (cursorLocked) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void handle_input(GLFWwindow *window)
{
    if (Input.isKeyJustPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (cursorLocked) {
        if (Input.isKeyPressed(GLFW_KEY_W))
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (Input.isKeyPressed(GLFW_KEY_S))
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (Input.isKeyPressed(GLFW_KEY_A))
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (Input.isKeyPressed(GLFW_KEY_D))
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (Input.isKeyJustPressed(GLFW_KEY_LEFT_SHIFT)) {
            camera.setSprintSpeed();
        }
        if (Input.isKeyJustReleased(GLFW_KEY_LEFT_SHIFT)) {
            camera.setNormalSpeed();
        }
    }

    //TODO: Robot movement
    if (controllingRobot) {
        if (Input.isKeyJustPressed(GLFW_KEY_W) || Input.isKeyJustPressed(GLFW_KEY_S)) {
            if (!animator->playing) {
                bool sPressed = Input.isKeyJustPressed(GLFW_KEY_S);
                animator->PlayAnimation("robot/Head", root->find("Head"), sPressed);
                animator->PlayAnimation("robot/LeftLeg", root->find("LeftLeg"), sPressed);
                animator->PlayAnimation("robot/LeftThigh", root->find("LeftThigh"), sPressed);
                animator->PlayAnimation("robot/RightThigh", root->find("RightThigh"), sPressed);
                animator->PlayAnimation("robot/RightLeg", root->find("RightLeg"), sPressed);
                animator->PlayAnimation("robot/RightArm", root->find("RightArm"), sPressed);
                animator->PlayAnimation("robot/LeftArm", root->find("LeftArm"), sPressed);
            }
        }

        if (Input.isKeyJustReleased(GLFW_KEY_W) || Input.isKeyJustReleased(GLFW_KEY_S)) {
            animator->StopAnimations();
        }

        if (Input.isKeyPressed(GLFW_KEY_W)) {
            float distance = robot->speed * deltaTime;
            float dx = (float) (distance * std::sin(glm::radians(torsoNode->transform.eulerRotation.y)));
            float dz = (float) (distance * std::cos(glm::radians(torsoNode->transform.eulerRotation.y)));
            torsoNode->transform.MoveLocalPosition({dx, 0, dz});
        }

        if (Input.isKeyPressed(GLFW_KEY_S)) {
            float distance = -robot->speed * deltaTime;
            float dx = (float) (distance * std::sin(glm::radians(torsoNode->transform.eulerRotation.y)));
            float dz = (float) (distance * std::cos(glm::radians(torsoNode->transform.eulerRotation.y)));
            torsoNode->transform.MoveLocalPosition({dx, 0, dz});
        }

        if (Input.isKeyPressed(GLFW_KEY_D)) {
            float rotation = -robot->rotationSpeed * deltaTime;
            torsoNode->transform.Rotate({0, rotation, 0});
        }

        if (Input.isKeyPressed(GLFW_KEY_A)) {
            float rotation = robot->rotationSpeed * deltaTime;
            torsoNode->transform.Rotate({0, rotation, 0});
        }

        if (Input.isKeyPressed(GLFW_KEY_Z)) {
            float rotation = robot->rotationSpeed * deltaTime;
            if (headNode->transform.eulerRotation.y <= 45.0f) {
                headNode->transform.Rotate({0, rotation, 0});
            }
        }

        if (Input.isKeyPressed(GLFW_KEY_C)) {
            float rotation = -robot->rotationSpeed * deltaTime;
            if (headNode->transform.eulerRotation.y >= -45.0f) {
                headNode->transform.Rotate({0, rotation, 0});
            }
        }

        if (Input.isKeyJustPressed(GLFW_KEY_X)) {
            headNode->transform.setEulerRotation({0,0,0});
        }
    }

    if (Input.isKeyJustPressed(GLFW_KEY_Q) && controllingRobot == false) {
        cursorLocked = !cursorLocked;
        setCursorLocked(cursorLocked);
    }

    if (Input.isKeyJustPressed(GLFW_KEY_R)) {
        controllingRobot = !controllingRobot;
        if (controllingRobot == true)
            cursorLocked = false;
        setCursorLocked(cursorLocked);
    }


}

void updateLights() {
    // Node* flashlightNode = root->find("Spot Light Flashlight");
    flashlightNode->light->setDirection(camera.Front);
    flashlightNode->light->setForTypeVec3("position", camera.Position);
}

void update()
{

    root->updateSelfAndChild(deltaTime);
    updateLights();

    animator->Update(deltaTime);

    if (controllingRobot) {
        camera.setPosition(Util::lerp(camera.Position, cameraHandleForRobot->transform.globalPosition, 5 * deltaTime));
        camera.LookAt(torsoNode->transform.globalPosition);
    }
}

void render()
{
    // glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // OpenGL Rendering code goes here
    // glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.GetViewMatrix(); 
    advancedShader->use();
    advancedShader->setMat4("view", view);
    advancedShader->setVec3("viewPos", camera.Position);

    emissionShader->use();
    emissionShader->setMat4("view", view);

    regularShader->use();
    regularShader->setMat4("view", view);

    reflectiveShader->use();
    reflectiveShader->setMat4("view", view);

    refractiveShader->use();
    refractiveShader->setMat4("view", view);

    testShader->use();
    testShader->setMat4("view", view);

    renderEntityAndChildren(root);

    view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix

    skybox->Draw(view);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void renderLights() {
    // advancedShader->use();
    // regularShader.use();

    Shader* shader = regularShader;
    for (int s = 0; s < 2; s++) {
        if (s == 0) shader = advancedShader;
        else shader = regularShader;

        shader->use();
        //Directional light
        Node* dirLightNode = root->find("Dir Light");
        Light* dirLight = dirLightNode->light;
        shader->setBool("dirLight.isOn", dirLight->active);
        shader->setVec3("dirLight.direction", dirLight->getDirection());
        shader->setVec3("dirLight.ambient", dirLight->getAmbient());
        shader->setVec3("dirLight.diffuse", dirLight->getDiffuse());
        shader->setVec3("dirLight.specular", dirLight->getSpecular());

        for (auto lightNode : pointLights) {
            Light* light = lightNode->light;

            std::string base = "pointLights[" + std::to_string(light->id) + "]";
            shader->setBool(base + ".isOn", light->active);
            shader->setVec3(base + ".position", lightNode->transform.getGlobalPosition());
            shader->setVec3(base + ".ambient", light->getAmbient());
            shader->setVec3(base + ".diffuse", light->getDiffuse());
            shader->setVec3(base + ".specular", light->getSpecular());
            shader->setFloat(base + ".constant", light->getConstant());
            shader->setFloat(base + ".linear", light->getLinear());
            shader->setFloat(base + ".quadratic", light->getQuadratic());
            shader->setFloat(base + ".intensity", light->getIntensity());

        }

        for (auto lightNode : spotLights) {
            Light* light = lightNode->light;

            std::string base = "spotLights[" + std::to_string(light->id) + "]";
            shader->setBool(base + ".isOn", light->active);
            shader->setVec3(base + ".position", lightNode->transform.getGlobalPosition());
            shader->setVec3(base + ".ambient", light->getAmbient());
            shader->setVec3(base + ".diffuse", light->getDiffuse());
            shader->setVec3(base + ".specular", light->getSpecular());
            shader->setFloat(base + ".constant", light->getConstant());
            shader->setFloat(base + ".linear", light->getLinear());
            shader->setFloat(base + ".quadratic", light->getQuadratic());
            shader->setFloat(base + ".intensity", light->getIntensity());
            shader->setFloat(base + ".linear", light->getLinear());
            shader->setFloat(base + ".cutOff", light->getCutOff());
            shader->setFloat(base + ".outerCutOff", light->getOuterCutOff());
            shader->setVec3(base + ".direction", light->getDirection());
        }
    }

}

void setUpLights(Model& pointLightModel, Model& spotLightModel, Model& dirLightModel) {

    // Directional light
    std::unique_ptr<Node> dirLightNode = std::make_unique<Node>(&dirLightModel);
    // Node* dirLightNode = new Node(&dirLightModel);
    dirLightNode->setLabel("Dir Light");
    dirLightNode->setStationary(true);

    glm::vec3 dirPos = dirLightNode->transform.getLocalPosition();
    dirPos.y += 40;
    dirPos.x += 20;
    dirLightNode->transform.setLocalPosition(dirPos);
    dirLightNode->transform.setEulerRotation({45, -90, 0});

    Light* dirLight = new Light(DIRECTIONAL, advancedShader, regularShader, 0);
    // dirLight->setDirection({-0.2f, -1.0f, -0.3f});
    dirLight->setDirection(Util::getDirectionFromEulerAngles(dirLightNode->transform.getEulerRotation().x, dirLightNode->transform.getEulerRotation().y, dirLightNode->transform.getEulerRotation().z));
    dirLight->setAmbient({0.05f, 0.05f, 0.05f});
    dirLight->setDiffuse({0.4f,0.4f,0.4f});
    dirLight->setSpecular({0.5f, 0.5f, 0.5f});
    dirLight->setIntensity(1.5f);
    dirLightNode->setLight(dirLight);
    root->addChild(std::move(dirLightNode));

    //Moving point light
    std::unique_ptr<Node> movingLightHandle = std::make_unique<Node>();
    // Node* movingLightHandle = new Node();
    movingLightHandle->setLabel("Moving Light Handle");
    movingLightHandle->setStationary(false);
    glm::vec3 pos = movingLightHandle->transform.getLocalPosition();
    pos.y += 10;
    movingLightHandle->transform.setLocalPosition(pos);

    root->addChild(std::move(movingLightHandle));


    std::unique_ptr<Node> movingPointLight = std::make_unique<Node>(&pointLightModel);
    // Node* movingPointLight = new Node(&pointLightModel);
    movingPointLight->setLabel("Moving Point Light");
    movingPointLight->setStationary(true);
    pos = movingPointLight->transform.getLocalPosition();
    pos.x += 15;
    movingPointLight->transform.setLocalPosition(pos);
    movingPointLight->transform.setScale({2, 2, 2});

    Light* mpl = new Light(POINT, advancedShader,regularShader, 0);
    mpl->setAmbient({0.55f, 0.55f, 0.55f});
    mpl->setDiffuse({0,0,1.0f});
    mpl->setSpecular({1.0f, 1.0f, 1.0f});
    mpl->setIntensity(10.0f);
    mpl->setForTypeVec3("position", movingPointLight->transform.getGlobalPosition());
    pointLights.push_back(movingPointLight.get());
    movingPointLight->setLight(mpl);

    Node* lastEntity = root->find("Moving Light Handle");

    lastEntity->addChild(std::move(movingPointLight));

    //Spotlights

    std::unique_ptr<Node> spotLightNode = std::make_unique<Node>();
    // Node* spotLightNode = new Node();
    spotLightNode->setLabel("Spot Light Flashlight");
    spotLightNode->setStationary(true);
    flashlightNode = spotLightNode.get();

    Light* spl = new Light(SPOTLIGHT, advancedShader,regularShader, 0);
    spl->setAmbient({0,0,0});
    spl->setDiffuse({1.0f, 0, 0});
    spl->setIntensity(20.0f);
    spl->setSpecular({1.0f, 1.0f, 1.0f});
    spl->setActive(false);

    spotLightNode->setLight(spl);
    spotLights.push_back(spotLightNode.get());

    root->addChild(std::move(spotLightNode));

    std::unique_ptr<Node> spotLightNode2 = std::make_unique<Node>(&spotLightModel);
    // Node* spotLightNode2 = new Node(&spotLightModel);
    spotLightNode2->setLabel("Spot Light 2");
    spotLightNode2->setStationary(true);
    spotLightNode2->rotationSpeed = 0.0f;
    glm::vec3 spotLightPos = spotLightNode2->transform.getLocalPosition();
    spotLightPos.y += 30;
    spotLightPos.x -= 10;
    spotLightNode2->transform.setLocalPosition(spotLightPos);

    Light* spl2 = new Light(SPOTLIGHT, advancedShader,regularShader, 1);
    spl2->setAmbient({0,0,0});
    spl2->setDiffuse({0.0f, 1.0f, 0});
    spl2->setDirection({0.0f, -1.0f, 0.0f});
    spl2->setIntensity(25.0f);
    spl2->setSpecular({1.0f, 1.0f, 1.0f});

    spotLightNode2->transform.setEulerRotation({90, 0, 0});
    spotLightNode2->setLight(spl2);

    spotLights.push_back(spotLightNode2.get());

    root->addChild(std::move(spotLightNode2));

    renderLights();
}

void setupShaders() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspectRatio = static_cast<float>((float)width / (float)height);
    glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 view = camera.GetViewMatrix();

    testShader->use();
    testShader->setMat4("projection", projection);

    advancedShader->use();
    advancedShader->setMat4("projection", projection);
    advancedShader->setFloat("material.shininess", 32.0f);

    emissionShader->use();
    emissionShader->setMat4("projection", projection);

    regularShader->use();
    regularShader->setMat4("projection", projection);
    regularShader->setFloat("material.shininess", 32.0f);

    skyboxShader->use();
    skyboxShader->setMat4("projection", projection);

    reflectiveShader->use();
    reflectiveShader->setMat4("projection", projection);

    refractiveShader->use();
    refractiveShader->setVec3("iorRGB", ior);
    refractiveShader->setFloat("aberrationStrength", chromaticAbberationStrength);
    refractiveShader->setMat4("projection", projection);

}

void renderEntityAndChildren(Node* entity) {

    if (!entity || !entity->isVisible()) return;

    if (entity->light) {
        if (entity->model != nullptr) {
            emissionShader->use();
            emissionShader->setMat4("model", entity->transform.getModelMatrix());
            emissionShader->setVec3("color", entity->light->diffuse);
        }
        entity->Draw(emissionShader, skybox->getCubemapTexture());
    }
    else if (entity->model != nullptr) {
        switch (entity->material) {
            case STANDARD:
                regularShader->use();
                regularShader->setMat4("model", entity->transform.getModelMatrix());

                entity->Draw(regularShader, skybox->getCubemapTexture());
                break;
            case REFLECTIVE:
                reflectiveShader->use();
                reflectiveShader->setMat4("model", entity->transform.getModelMatrix());
                reflectiveShader->setVec3("cameraPos", camera.Position);

                entity->Draw(reflectiveShader, skybox->getCubemapTexture());
                break;
            case REFRACTIVE:

                refractiveShader->use();
                refractiveShader->setMat4("model", entity->transform.getModelMatrix());
                refractiveShader->setVec3("cameraPos", camera.Position);
                // refractiveShader->setSampler2D("screenTexture", colorTexture);


                entity->Draw(refractiveShader, skybox->getCubemapTexture());
                break;
            default:
                regularShader->use();
                regularShader->setMat4("model", entity->transform.getModelMatrix());

                entity->Draw(regularShader, skybox->getCubemapTexture());
                break;
        }

    }

    // Render children
    for (auto& child : entity->children) {
        renderEntityAndChildren(child.get());
    }
}

void imgui_begin()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

//time to lerp for key frame
float timeForFrame = 2.0f;


void drawSceneTree(Node* node) {
    if (!node) return;

    if (ImGui::TreeNode(node, "%s : Node", node->getLabel())) {
        if (node->model && node->light == nullptr) {
            ImGui::Text("Material");
            ImGui::SameLine();

            const std::vector<std::string> items = node->materialMap;
            static int selected_material_idx = node->material;

            const std::string comboPreviewValue = items[selected_material_idx];
            if (ImGui::BeginCombo("##", comboPreviewValue.c_str())) {
                for (int i = 0; i < items.size(); i++) {
                    const bool is_selected = (selected_material_idx == i);
                    if (ImGui::Selectable(items[i].c_str(), is_selected)) {
                        selected_material_idx = i;
                        node->setMaterial((Material) selected_material_idx);
                    }

                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
        if(ImGui::TreeNode("Transform")) {
            std::string pos = "x: " + Util::format(node->transform.getLocalPosition().x, 2) + ", y: " + Util::format(node->transform.getLocalPosition().y, 2) + ", z: " + Util::format(node->transform.getLocalPosition().z, 2);
            ImGui::Text(("Position: " + pos).c_str());
            glm::vec3 positionVec = node->transform.getLocalPosition();
            ImGui::DragFloat3("##localPos",(float*) &positionVec, 0.1f);
            if (node->transform.getLocalPosition() != positionVec)
                node->transform.setLocalPosition(positionVec);

            std::string rot = "x: " + Util::format(node->transform.getEulerRotation().x, 2) + ", y: " + Util::format(node->transform.getEulerRotation().y, 2) + ", z: " + Util::format(node->transform.getEulerRotation().z, 2);
            ImGui::Text(("Rotation: " + rot).c_str());
            glm::vec3 rotationVec = node->transform.getEulerRotation();
            ImGui::DragFloat3("##localRot",(float*) &rotationVec, 1.0f);
            if (node->transform.getEulerRotation() != rotationVec) {
                node->transform.setEulerRotation(rotationVec);
                if (node->light)
                    node->light->setDirection(Util::getDirectionFromEulerAngles(rotationVec.x, rotationVec.y, rotationVec.z));
            }

            std::string scl = "x: " + Util::format(node->transform.scale.x, 2) + ", y: " + Util::format(node->transform.scale.y, 2) + ", z: " + Util::format(node->transform.scale.z, 2);
            ImGui::Text(("Scale: " + scl).c_str());
            glm::vec3 sclVec = node->transform.scale;
            ImGui::DragFloat3("##localScale",(float*) &sclVec, 0.1f);
            if (node->transform.scale != sclVec) {
                node->transform.setScale(sclVec);
            }

            if (ImGui::Button("Key Frame")) {
                std::vector<glm::vec3> transforms = {
                    node->transform.getLocalPosition(),
                    node->transform.getEulerRotation(),
                    node->transform.scale
                };
                animator->RecordKeyFrame("robot/" + static_cast<std::string>(node->getLabel()), transforms, timeForFrame);
            }


            ImGui::TreePop();

        }


        if (node->light) {
            ImGui::Text("State:");
            ImGui::SameLine();
            if (ImGui::Button(node->light->active ? "On" : "Off")) {
                node->light->setActive(!node->light->active);
            }

            ImGui::Text("Color:");
            ImGui::SameLine();
            glm::vec3 color = node->light->diffuse;
            ImGui::ColorEdit3("##color", (float*) &color);
            if (color != node->light->diffuse) {
                node->light->setDiffuse(color);
            }

            ImGui::Text("Intensity:");
            ImGui::SameLine();
            float intensity =  node->light->getIntensity();
            ImGui::DragFloat("##intensity", &intensity, 0.1f, 0.0f, 500.0f);
            if (intensity != node->light->getIntensity()) {
                node->light->setIntensity(intensity);
            }

            if (node->light->type == SPOTLIGHT || node->light->type == DIRECTIONAL) {
                ImGui::Text("Direction:");
                ImGui::SameLine();
                glm::vec3 dir = node->light->getDirection();
                ImGui::DragFloat3("##direction", (float*) &dir, 0.01f, -1, 1);
                if (dir != node->light->getDirection()) {
                    node->light->setDirection(dir);
                }


            }
        }
        if (node->children.size() > 0) {
            std::string l = "Children: " + std::to_string(node->children.size());
            if (ImGui::TreeNode(l.c_str())) {
                for (const auto& child : node->children) {
                    drawSceneTree(child.get());
                }
                ImGui::TreePop();
            }
        } else {
            ImGui::TextColored({0.6f, 0.2f, 0.8f, 1}, "---");
        }
        // for ( auto* child : node->children) {
        //     drawSceneTree(child);
        // }

        ImGui::TreePop();
    }
}

void imgui_render()
{
    if(ImGui::Begin("Inspector")) {
        ImGui::Text(("FPS: " + std::to_string(FPS)).c_str());

        if (ImGui::BeginTabBar("Tabs")) {
            if (ImGui::BeginTabItem("Scene")) {
                ImGui::Checkbox("Wireframe", &wireframe);
                drawSceneTree(root);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Animation")) {
                if (ImGui::Button("Prepare Animations")) {
                    animator->PrepareAnimations("robot/Head");
                    animator->PrepareAnimations("robot/LeftLeg");
                    animator->PrepareAnimations("robot/LeftThigh");
                    animator->PrepareAnimations("robot/RightThigh");
                    animator->PrepareAnimations("robot/RightLeg");
                    animator->PrepareAnimations("robot/LeftArm");
                    animator->PrepareAnimations("robot/LeftForearm");
                    animator->PrepareAnimations("robot/RightArm");
                    animator->PrepareAnimations("robot/RightForearm");
                }
                if (ImGui::Button("Play Animation")) {
                    // animator->PlayAnimation("robot/Head", root->find("Head"));
                    animator->PlayAnimation("robot/LeftLeg", root->find("LeftLeg"));
                    animator->PlayAnimation("robot/LeftThigh", root->find("LeftThigh"));
                    animator->PlayAnimation("robot/RightThigh", root->find("RightThigh"));
                    animator->PlayAnimation("robot/RightLeg", root->find("RightLeg"));
                    animator->PlayAnimation("robot/LeftArm", root->find("LeftArm"));
                    animator->PlayAnimation("robot/LeftForearm", root->find("LeftForearm"));
                    animator->PlayAnimation("robot/RightArm", root->find("RightArm"));
                    animator->PlayAnimation("robot/RightForearm", root->find("RightForearm"));
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop All Animations")) {
                    animator->StopAnimations();
                }

                ImGui::Text("----------");

                ImGui::DragFloat("Playback speed for frame", &timeForFrame, 0.1f);

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Shaders")) {
                ImGui::Text("Refractive Shader Parameters:");
                if (ImGui::DragFloat3("IOR", (float*) &ior, 0.01f)) {
                    refractiveShader->use();
                    refractiveShader->setVec3("iorRGB", ior);
                }
                if (ImGui::DragFloat("Chromatic Abberation Strength", &chromaticAbberationStrength, 0.01f)) {
                    refractiveShader->use();
                    refractiveShader->setFloat("aberrationStrength", chromaticAbberationStrength);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Controls")) {
                ImGui::Text("Controls:");
                ImGui::Text("Robot View - R");
                ImGui::Text("Rotate Robot's Head - Z, X (reset), C");
                ImGui::Text("Movement - W, A, S, D");
                ImGui::Text("Toggle Cursor Lock - Q");
                ImGui::Text("Fly Faster - Hold LSHIFT");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }


    }
    ImGui::End();

}

void imgui_end()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void end_frame()
{
    glfwPollEvents();
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    setupShaders();
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (cursorLocked && !controllingRobot) {

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;


        camera.ProcessMouseMovement(xoffset, yoffset);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // if (!cursorLocked) return;
    // camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
