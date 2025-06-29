#include <Syngine/Syngine.hpp>
#include <Syngine/modules/Camera.hpp>
#include <Syngine/modules/Model.hpp>

#include <Syngine/world/entity/EntityConvexHull.hpp>
#include <Syngine/world/entity/EntityTriangleMeshCompound.hpp>

#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

#include "GLFW/glfw3.h"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/CubemapFramebuffer.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/Skybox.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

float moveAccel = 2.0f;

GameWindow *window;
Scene *scene;

Framebuffer *framebuffer;
CubemapFramebuffer *cubemapFramebuffer;
Skybox *skybox;

float gamma = 1.1f;
float yaw = 0, pitch;
double lastX, lastY;
bool firstMouse, mouseCaptured = true;

World* overWorld;

Model* appleModel;
Model* sceneModel;

ModelInstance* sceneModelInstance;
MeshInstance* appleMeshInstance;

EntityConvexHull* appleEntity;
EntityTriangleMeshCompound* sceneEntity;

Camera* camera;

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
    const float cameraSpeed = 1.5f * window->getLastFrameTime(); // adjust accordingly
    glm::vec3 horizontalDirection = glm::vec3(0.0f);

    horizontalDirection.x = cos(glm::radians(yaw));
    horizontalDirection.z = sin(glm::radians(yaw));

    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 cameraUp = camera->getUp();

    glm::vec3 moving(0.0f);

    if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS) {
        glm::vec3 dir(1.0f, 0.0f, 0.0f);
        dir.y = 0;
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
        glm::vec3 dir(-1.0f, 0.0f, 0.0f);
        dir.y = 0;
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
        glm::vec3 dir(0.0f, 0.0f, -1.0f);
        dir.y = 0;
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        glm::vec3 dir(0.0f, 0.0f, 1.0f);
        dir.y = 0;
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }
    
    btRigidBody* body = appleEntity->getBody();

    body->activate();
    if (body->getLinearVelocity().length2() <= 3) {
        moving *= (3 / moving.length());
    }
    body->applyCentralImpulse(GameUtils::toBulletVector(moving));

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

void init_ImGUI() {
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window->getGLFWWindowPtr(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

/* TODO:
 *   - Window Resize Viewport was bugged
 *   - Make Spot/Point lights dynamic and configurable
 *   - Make shaders variables replaceable (AKA Configurable)
 *   - Make shaders reloadable
 *   - Flame Particles ( ) | Gamma correction (*) -> HDR ( ) -> Bloom ( ) -> Normal Mapping ( ) -> PBR ( )
 *   - Make format parser for mesh nodes name <prefix>_<name> (ECH_: Entity Convex Hull, ETM_: Entity Triangle Mesh, PF_: FlameParticle, [B]LP_: [Bloom]PointLight, [B]LS_: [Bloom]SpotLight, R_: Renderable mesh)
 *   - Review https://github.com/kcat/openal-soft for 3D Audio
 *   - < Game Modeling + Design >
 *   - < Game UI >
 *   - < Networking (via Facebook Wangle) >
 *   - < Produce >
 */ 
void init(GameWindow *window) {
    overWorld = new World(0, "overworld");
    camera = new Camera(overWorld, glm::vec3(5.0f, 0.0f, 5.0f), yaw, pitch);

    appleModel = new Model("models/apple2/apple.obj");
    appleModel->filterMesh("Apple");
    appleModel->loadModel(Interleaved);
    appleMeshInstance = new MeshInstance(appleModel->meshes["Apple"]);

    appleEntity = new EntityConvexHull(overWorld, 0.2f, appleModel->meshes["Hitbox"]);
    appleEntity->setPosition(glm::vec3(0, 2, 0));
    appleEntity->bind("Apple", appleMeshInstance);
    appleEntity->load(false);

    sceneModel = new Model("models/wall/wall.obj");
    sceneModel->loadModel(Sequential);
    sceneModelInstance = new ModelInstance(sceneModel);

    sceneEntity = new EntityTriangleMeshCompound(overWorld, sceneModel);
    sceneEntity->load();

    scene = new Scene(camera, window);
    scene->installCallbacks(window);
    scene->getBatchRenderTable()->add("sceneModel", sceneModelInstance);
    scene->getBatchShader().use();
    scene->getBatchShader().setVec3f("dirLight.direction", -0.2f, -1.0f, -0.3f);
    scene->getBatchShader().setVec3f("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    scene->getBatchShader().setVec3f("dirLight.diffuse", 0.1f, 0.1f, 0.1f);
    scene->getBatchShader().setVec3f("dirLight.specular", 0.2f, 0.2f, 0.2f);

    skybox = new Skybox(scene, std::vector<std::string> {
        "models/skybox/lightblue/right.png",
        "models/skybox/lightblue/left.png",
        "models/skybox/lightblue/top.png",
        "models/skybox/lightblue/bot.png",
        "models/skybox/lightblue/front.png",
        "models/skybox/lightblue/back.png"
    });
    skybox->load();
    scene->getBatchRenderTable()->add("skybox", skybox);
    scene->getBatchShader().use();
    scene->getBatchShader().setVec3f("spotLights[0].position", camera->getPosition());
    scene->getBatchShader().setVec3f("spotLights[0].direction", camera->getDirection());

    cubemapFramebuffer = new CubemapFramebuffer(scene);
    cubemapFramebuffer->getRefractionRenderTable()->add("apple", appleMeshInstance);
    cubemapFramebuffer->create(true);

    framebuffer = new Framebuffer(scene);
    framebuffer->getRenderTable()->add("scene", scene);
    framebuffer->getRenderTable()->add("reflectives", cubemapFramebuffer);
    framebuffer->create(window, true);

    window->getWindowRenderTable()->add("overWorld", overWorld);
    window->getWindowRenderTable()->add("appleEntity", appleEntity);
    window->getWindowRenderTable()->add("sceneEntity", sceneEntity);
    window->getWindowRenderTable()->add("framebuffer", framebuffer);

    glfwSwapInterval(0);
    glfwSetInputMode(window->getGLFWWindowPtr(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void render_Inputs(GameWindow *window) {
    double xpos, ypos;

    glfw_process_keys(window->getGLFWWindowPtr());
    glfwGetCursorPos(window->getGLFWWindowPtr(), &xpos, &ypos);
    glfw_process_mouse(window->getGLFWWindowPtr(), xpos, ypos);
}

void render_ImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.0f", (window->getLastFrameTime() == 0) ? 999.0f : 1.0f / window->getLastFrameTime());

    if (ImGui::Button("Reset")) {
        window->getWindowRenderTable()->wipe("appleEntity");

        appleEntity = new EntityConvexHull(overWorld, 0.2f, appleModel->meshes["Hitbox"]);
        appleEntity->load();

        window->getWindowRenderTable()->add("appleEntity", appleEntity);
    }
    ImGui::SliderFloat("IOR", &appleMeshInstance->getMesh()->material.ior, 1.0f, 2.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Gamma", &gamma, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::Checkbox("Mouse Captured", &mouseCaptured);
    ImGui::End();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void onExit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
}

int main() {
    window = new GameWindow("Sample", 800, 600);
    window->withHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    window->addInitTask([](GameWindow *window){ 
        init_ImGUI();
        init(window);
    });
    window->addRenderTask([](GameWindow *window){ 
        scene->getScreenShader().use();
        scene->getScreenShader().setFloat("gamma", gamma);
        scene->getBatchShader().use();
        scene->getBatchShader().setVec3f("spotLights[1].position", camera->getPosition());
        scene->getBatchShader().setVec3f("spotLights[1].direction", camera->getDirection());
        render_Inputs(window);
        render_ImGui();
    });

    int exitCode = window->initLoop();
    onExit();
    return exitCode;
}