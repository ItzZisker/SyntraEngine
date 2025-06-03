#include <KEngine/KEngine.hpp>
#include <KEngine/modules/Camera.hpp>
#include <KEngine/modules/Model.hpp>

#include <KEngine/world/entity/EntityConvexHull.hpp>
#include <KEngine/world/entity/EntityTriangleMesh.hpp>

#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

float moveAccel = 2.0f;

GameWindow window("Sample", 800, 600);

glm::mat4 projection = glm::mat4(1.0f);
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

float yaw = 0, pitch;
double lastX, lastY;
bool firstMouse, mouseCaptured = true;

World* overWorld;
Model* appleModel, *sceneModel;

EntityConvexHull* appleEntity;
EntityTriangleMesh* sceneEntity;

Camera* camera;

void applyVelocity(glm::vec3 targetVelocity) {
    btRigidBody* body = appleEntity->getBody();

    body->activate();
    if (body->getLinearVelocity().length2() <= 3) {
        targetVelocity *= (3 / targetVelocity.length());
    }
    body->applyCentralImpulse(GameUtils::toBulletVector(targetVelocity));
}

void glfw_process_mouse(GLFWwindow *glfwWindow, double xpos, double ypos) {
    if (!mouseCaptured) {
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    yaw = fmod(yaw, 360.0f);
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    camera->setDirection(GameUtils::directionOf(yaw, pitch));
}

void glfw_process_keys(GLFWwindow *glfwWindow) {
    const float cameraSpeed = 1.5f * window.getLastFrameTime(); // adjust accordingly
    glm::vec3 horizontalDirection = glm::vec3(0.0f);

    horizontalDirection.x = cos(glm::radians(yaw));
    horizontalDirection.z = sin(glm::radians(yaw));

    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 cameraUp = camera->getUp();

    glm::vec3 moving(0.0f);

    if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS) {
        glm::vec3 dir(1.0f, 0.0f, 0.0f);
        dir.y = 0;
        dir *= (window.getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
        glm::vec3 dir(-1.0f, 0.0f, 0.0f);
        dir.y = 0;
        dir *= (window.getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
        glm::vec3 dir(0.0f, 0.0f, -1.0f);
        dir.y = 0;
        dir *= (window.getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        glm::vec3 dir(0.0f, 0.0f, 1.0f);
        dir.y = 0;
        dir *= (window.getLastFrameTime() * moveAccel);
        moving += dir;
    }

    applyVelocity(moving);

    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * horizontalDirection;
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * horizontalDirection;
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;

    camera->setPosition(cameraPos);

    if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS)
        lightPos += glm::vec3(1.0f, 0.0f, 0.0f) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
        lightPos -= glm::vec3(1.0f, 0.0f, 0.0f) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
        lightPos -= glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
        lightPos += glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed;

    if (glfwGetKey(glfwWindow, GLFW_KEY_P) == GLFW_PRESS)
        overWorld->paused = false;

    static bool escapePressedLastFrame = false;

    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (!escapePressedLastFrame) {
            mouseCaptured = !mouseCaptured;
            glfwSetInputMode(glfwWindow, GLFW_CURSOR,
                             mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
        escapePressedLastFrame = true;
    } else {
        escapePressedLastFrame = false;
    }
}

void glfw_framebuffer_resize_callback(GLFWwindow *glfwWindow, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(65.0f), (float)width / (float)height, 0.1f, 100.0f);

    Shader batchShader = window.getBatchShader();
    batchShader.use();
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
}

void init(GameWindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window->getGLFWWindowPtr(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    overWorld = new World(0, "overworld");
    camera = new Camera(overWorld, glm::vec3(5.0f, 0.0f, 5.0f), yaw, pitch);

    appleModel = new Model("models/apple/apple.obj");
    appleModel->loadModel();

    sceneModel = new Model("models/scene/test.obj");
    sceneModel->loadModel();

    appleEntity = new EntityConvexHull(overWorld, 0.2f, appleModel);
    sceneEntity = new EntityTriangleMesh(overWorld, 0.0f, sceneModel);

    appleEntity->load(false);    
    sceneEntity->load();

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);

    int height = window->getWindowHeight();
    int width = window->getWindowWidth();

    projection = glm::perspective(glm::radians(65.0f), (float)width / (float)height, 0.1f, 100.0f);

    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window->getGLFWWindowPtr(), glfw_framebuffer_resize_callback);
    glfwSetInputMode(window->getGLFWWindowPtr(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    window->getRenderTable()->add("overWorld", overWorld);
    window->getRenderTable()->add("camera", camera);
    window->getRenderTable()->add("apple", appleEntity);
    window->getRenderTable()->add("scene", sceneEntity);

    Shader batchShader = window->getBatchShader();

    batchShader.use();
    batchShader.setFloat("shininess", 32.0f);

    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);

    batchShader.setVec3f("dirLight.direction", -0.2f, -1.0f, -0.3f);
    batchShader.setVec3f("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    batchShader.setVec3f("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    batchShader.setVec3f("dirLight.specular", 0.5f, 0.5f, 0.5f);

    batchShader.setVec3f("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    batchShader.setVec3f("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    batchShader.setVec3f("spotLight.specular", 1.0f, 1.0f, 1.0f);
    batchShader.setFloat("spotLight.constant", 1.0f);
    batchShader.setFloat("spotLight.linear", 0.09f);
    batchShader.setFloat("spotLight.quadratic", 0.032f);
    batchShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    batchShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

void render_Inputs(GameWindow *window) {
    glfw_process_keys(window->getGLFWWindowPtr());
    double xpos, ypos;
    glfwGetCursorPos(window->getGLFWWindowPtr(), &xpos, &ypos);
    glfw_process_mouse(window->getGLFWWindowPtr(), xpos, ypos);
}

void render_ImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug");

    if (ImGui::Button("Reset")) {
        if (appleEntity) {
            window.getRenderTable()->remove("apple");
            delete appleEntity;
        }

        appleEntity = new EntityConvexHull(overWorld, 0.2f, appleModel);
        appleEntity->load();

        window.getRenderTable()->add("apple", appleEntity);
    }
    ImGui::Checkbox("Mouse Captured", &mouseCaptured);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
    window.withHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    window.addInitTask([](GameWindow *window){ init(window); });
    window.addRenderTask([](GameWindow *window){ 
        render_Inputs(window);
        render_ImGui();
    });

    int exitCode = window.initWindow();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return exitCode;
}