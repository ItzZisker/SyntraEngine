#include <Syngine/Syngine.hpp>
#include <Syngine/modules/Camera.hpp>
#include <Syngine/modules/Model.hpp>
#include <Syngine/modules/ShadowMapper.hpp>

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
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"

float moveAccel = 2.0f;

SDL_Event event;

GameWindow *window;
Scene *scene;

ShadowMapper *shadowMapper;
Framebuffer *framebuffer;
CubemapFramebuffer *cubemapFramebuffer;
Skybox *skybox;

float lX = 0.0f;
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

void sdl_process_mouse(const SDL_Event& event) {
	if (!mouseCaptured) {
		return;
	}

    float x, y, xrel, yrel;

    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        float x = static_cast<float>(event.motion.x);
        float y = static_cast<float>(event.motion.y);
        float xrel = static_cast<float>(event.motion.xrel);
        float yrel = static_cast<float>(event.motion.yrel);

        const float sensitivity = 1.1f;
        xrel *= sensitivity;
        yrel *= sensitivity;
        yaw += xrel;
        pitch += yrel;
        yaw = fmod(yaw, 360.0f);
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
        camera->setDirection(GameUtils::directionOf(yaw, -pitch));
    }
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

    ImGui_ImplSDL3_InitForOpenGL(window->getSDLWindowPtr(), window->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");
}

/* TODO:
 *   - [*] Window Resize Viewport was bugged
 *   - [*] Make shaders variables replaceable (AKA Configurable)
 *   - [*] Make shaders reloadable
 *   - [*] Make Spot/Point lights dynamic and configurable
*    - [-] Opacity Support + Blending Objects (Only Supports Disappearing Objects or showing objects behind, Skybox & Blending is not supported)
 *   - [-] Shadow Mapping: Directional Shadows (*) -> Point Shadows ( ) -> Cascaded Shadow Mapping ( )
 *   - [ ] Anti-Aliasing
 *   - [-] Flame Particles ( ) | Gamma correction (*) -> HDR ( ) -> Bloom ( ) -> Normal Mapping ( ) -> PBR Textures ( )
 *   - [ ] Make format parser for mesh nodes name <prefix>_<name> (ECH_: Entity Convex Hull, ETM_: Entity Triangle Mesh, PF_: FlameParticle, [B]LP_: [Bloom]PointLight, [B]LS_: [Bloom]SpotLight, R_: Renderable mesh)
 *   - [ ] Review https://github.com/kcat/openal-soft for 3D Audio
 *   - [ ] < Game Modeling + Design >
 *   - [ ] < Game UI >
 *   - [ ] < Networking (via Facebook Wangle) >
 *   - [ ] < Produce >
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
    scene->withCallbacks(window);
    scene->getBatchRenderTable()->add("sceneModel", sceneModelInstance);
    scene->getBatchRenderTable()->add("appleModel", appleMeshInstance);

    shadowMapper = new ShadowMapper(2048);
    shadowMapper->create();
    scene->withShadows(shadowMapper);

    skybox = new Skybox(scene, {
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
    //framebuffer->getRenderTable()->add("reflectives", cubemapFramebuffer);
    framebuffer->create(800, 600, true);

    window->getWindowRenderTable()->add("overWorld", overWorld);
    window->getWindowRenderTable()->add("appleEntity", appleEntity);
    window->getWindowRenderTable()->add("sceneEntity", sceneEntity);
    window->getWindowRenderTable()->add("framebuffer", framebuffer);

    SDL_GL_SetSwapInterval(0);
    SDL_SetWindowRelativeMouseMode(window->getSDLWindowPtr(), true);
}

void render_Inputs(GameWindow *window) {
    float xpos, ypos;
    //glfw_process_keys(window->getGLFWWindowPtr());
    //glfwGetCursorPos(window->getGLFWWindowPtr(), &xpos, &ypos);
    //glfw_process_mouse(window->getGLFWWindowPtr(), xpos, ypos);
    SDL_PollEvent(&event);
    sdl_process_mouse(event);
}

void render_ImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.0f", (window->getLastFrameTime() == 0) ? 999.0f : 1.0f / window->getLastFrameTime());

    if (ImGui::Button("Reset")) {
        window->getWindowRenderTable()->wipe("appleEntity");

        appleEntity = new EntityConvexHull(overWorld, 0.2f, appleModel->meshes["Hitbox"]);
        appleEntity->load();

        window->getWindowRenderTable()->add("appleEntity", appleEntity);
    }
    ImGui::SliderFloat("IOR R", &appleMeshInstance->getMesh()->material.ior.x, 1.0f, 2.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("IOR G", &appleMeshInstance->getMesh()->material.ior.y, 1.0f, 2.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("IOR B", &appleMeshInstance->getMesh()->material.ior.z, 1.0f, 2.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("F0", &appleMeshInstance->getMesh()->material.F0, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Opacity (Scene)", &sceneModelInstance->meshInstances.find("Cube")->second.getMesh()->material.opacity, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Opacity", &appleMeshInstance->getMesh()->material.opacity, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Gamma", &gamma, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Light X", &lX, 0.1f, 20.0f, "%.3f");
    ImGui::SliderFloat("Shadow Bias Min", &shadowMapper->biasMin, 0.001f, 1.0f, "%.3f");
    ImGui::SliderFloat("Shadow Bias Max", &shadowMapper->biasMax, 0.001f, 1.0f, "%.3f");
    ImGui::Checkbox("Mouse Captured", &mouseCaptured);
    ImGui::End();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void onExit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext();
}

int main() {
    window = new GameWindow("Sample", 800, 600);
    //window->withHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    window->withHint(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    window->addInitTask([](GameWindow *window){ 
        init_ImGUI();
        init(window);
    });
    window->addRenderTask([](GameWindow *window){ 
        scene->getScreenShader().use();
        scene->getScreenShader().setFloat("gamma", gamma);
        scene->getBatchShader().use();
        scene->getBatchShader().setVec3f("pointLights[0].position", lX, 2.0f, 0.0f);
        scene->getBatchShader().setVec3f("spotLights[1].position", camera->getPosition());
        scene->getBatchShader().setVec3f("spotLights[1].direction", camera->getDirection());
        render_Inputs(window);
        render_ImGui();
    });

    int exitCode = window->initLoop();
    onExit();
    return 0;
}