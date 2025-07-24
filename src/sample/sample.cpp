#include <Syngine.hpp>
#include <modules/Camera.hpp>
#include <modules/Model.hpp>
#include <modules/ShadowMapper.hpp>

#include <world/entity/BT_EntityConvexHull.hpp>
#include <world/entity/BT_EntityTriangleMesh.hpp>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

#include "SDL3/SDL_events.h"
#include "engine/RenderTable.hpp"
#include "modules/CubemapFramebuffer.hpp"
#include "modules/Framebuffer.hpp"
#include "modules/Mesh.hpp"
#include "modules/MeshInstance.hpp"
#include "modules/ModelInstance.hpp"
#include "modules/Scene.hpp"
#include "modules/Shader.hpp"
#include "modules/Skybox.hpp"
#include "utils/GameUtils.hpp"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"

using namespace syng;

float moveAccel = 2.0f;

GameWindow *window;
Scene *scene;

ShadowMapper *shadowMapper;
Framebuffer *framebuffer;
CubemapFramebuffer *cubemapFramebuffer;
Skybox *skybox;

const float sensitivity = 0.1f;

float lX = 0.0f;
float gamma = 1.1f;
float yaw = 0, pitch;
double lastX, lastY;
bool firstMouse, mouseCaptured = true;

BT_World* overWorld;

Model* appleModel;
Model* sceneModel;

ModelInstance* sceneModelInstance;
MeshInstance* appleHMeshInstance;
MeshInstance* appleMeshInstance;

BT_EntityConvexHull* appleEntity;
BT_EntityTriangleMesh* sceneEntity;

Camera* camera;

void sdl_process_mouse(const SDL_Event& event) {
	if (!mouseCaptured || event.type != SDL_EVENT_MOUSE_MOTION) {
		return;
	}
    float x = static_cast<float>(event.motion.x);
    float y = static_cast<float>(event.motion.y);
    float xrel = static_cast<float>(event.motion.xrel);
    float yrel = static_cast<float>(event.motion.yrel);

    xrel *= sensitivity;
    yrel *= sensitivity;
    yaw += xrel;
    pitch += yrel;
    yaw = fmod(yaw, 360.0f);
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    camera->setDirection(GameUtils::directionOf(yaw, -pitch));
}

void sdl_process_keys(SDL_Window* sdlWindow) {
    const bool* state = SDL_GetKeyboardState(NULL);
    const float cameraSpeed = 1.5f * window->getLastFrameTime();

    glm::vec3 horizontalDirection(0.0f);

    horizontalDirection.x = cos(glm::radians(yaw));
    horizontalDirection.z = sin(glm::radians(yaw));

    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 cameraUp = camera->getUp();

    glm::vec3 moving(0.0f);

    if (state[SDL_SCANCODE_UP]) {
        glm::vec3 dir(1.0f, 0.0f, 0.0f);
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        glm::vec3 dir(-1.0f, 0.0f, 0.0f);
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (state[SDL_SCANCODE_LEFT]) {
        glm::vec3 dir(0.0f, 0.0f, -1.0f);
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        glm::vec3 dir(0.0f, 0.0f, 1.0f);
        dir *= (window->getLastFrameTime() * moveAccel);
        moving += dir;
    }

    btRigidBody* body = appleEntity->getBody();

    body->activate();
    if (body->getLinearVelocity().length2() <= 3 && moving.length() > 0.0f) {
        moving *= (3.0f / moving.length());
    }
    body->applyCentralImpulse(GameUtils::toBulletVector(moving));

    if (state[SDL_SCANCODE_W])
        cameraPos += cameraSpeed * horizontalDirection;
    if (state[SDL_SCANCODE_S])
        cameraPos -= cameraSpeed * horizontalDirection;
    if (state[SDL_SCANCODE_A])
        cameraPos -= glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (state[SDL_SCANCODE_D])
        cameraPos += glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (state[SDL_SCANCODE_SPACE])
        cameraPos += cameraSpeed * cameraUp;
    if (state[SDL_SCANCODE_LSHIFT])
        cameraPos -= cameraSpeed * cameraUp;

    camera->setPosition(cameraPos);

    if (state[SDL_SCANCODE_P])
        overWorld->paused = false;

    static bool escapePressedLastFrame = false;
    if (state[SDL_SCANCODE_ESCAPE]) {
        if (!escapePressedLastFrame) {
            mouseCaptured = !mouseCaptured;
            SDL_SetWindowRelativeMouseMode(sdlWindow, mouseCaptured);
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
 *   === SEIZURE PROGRAM (Lethal-like Coop Video Game) ===
 *
 *   - [*] Window Resize Viewport bugfix
 *   - [*] Make shaders variables replaceable (AKA Configurable)
 *   - [*] Make shaders reloadable
 *   - [*] Make Spot/Point lights dynamic and configurable
 *   - [*] Move To SDL3 & Delete GLFW3
 *   - [*] Move headers to source
 *   - [*] Fix SDL3 Window I/O ImGui Bug
 *   - [*] Move classes/global functions to "syng" namespace
 *   - [*] Rename Bullet-dependent Classes Starting with "BT_" and PhysX with "PX_"
 *   - [*] Opacity Support + Blending Objects (Supports both Skybox & Objects behind)
 *   - [*] Rebuild Bullet linked all in one libBullet3.dll (Impossible, linked them statically, much more cleaner)
 *   - [ ] Deferred Shading
 *   - [ ] Merge Point Shadow Mapping into "development" branch
 *   - [-] Shadow Mapping: Directional Shadows (*) -> Point Shadows (*) -> Cascaded Shadow Mapping ( )
 *   - [ ] SSAO (+ < Game Menu Option >)
 *   - [ ] Room to Room Lighting System (Affects lights only on visible neighboring faces using id Tech 4 method or Minecraft's Lighting System)
 *   - [ ] Anti-Aliasing
 *   - [-] Flame Particles ( ) | Gamma correction (*) -> HDR ( ) -> Bloom ( ) -> Normal Mapping ( ) -> PBR Textures ( )
 *   - [ ] < Make format parser for mesh nodes name (Using gltf's custom properties + assimp) (ECH_: Entity Convex Hull, ETM_: Entity Triangle Mesh, PF_: FlameParticle, [B]LP_: [Bloom]PointLight, [B]LS_: [Bloom]SpotLight, R_: Renderable mesh) >
 *   - [ ] < Serialize/Deserialize Game Data >
 *   - [ ] < Review https://github.com/kcat/openal-soft for 3D Audio >
 *   - [ ] < Game Modeling + Design (Low Poly? High Constrast colors?) >
 *   - [ ] < Game UI (VHS Style Menus? idk) >
 *   - [ ] < Networking (via Facebook Wangle) + ANSI Server >
 *   - [ ] < Produce (Demo via itch.io, Paid on Steam) >
 */ 
void init(GameWindow *window) {
    overWorld = new BT_World(0, "overworld");
    camera = new Camera(overWorld, glm::vec3(5.0f, 0.0f, 5.0f), yaw, pitch);

    appleModel = new Model("models/apple2/apple.obj");
    appleModel->filterMesh("Apple");
    appleModel->loadModel(Interleaved);

    appleHMeshInstance = new MeshInstance(appleModel->meshes["Hitbox"]);
    appleHMeshInstance->setScale(glm::vec3(1.0f, 3.0f, 1.0f));

    appleMeshInstance = new MeshInstance(appleModel->meshes["Apple"]);
    appleMeshInstance->setScale(glm::vec3(1.0f, 3.0f, 1.0f));

    appleEntity = new BT_EntityConvexHull(overWorld, 0.2f, *appleHMeshInstance);
    appleEntity->setPosition(glm::vec3(0, 10, 0));
    appleEntity->bind("Apple", appleMeshInstance);
    appleEntity->load(false);

    sceneModel = new Model("models/wall/wall.obj");
    sceneModel->loadModel(Sequential);
    sceneModelInstance = new ModelInstance(sceneModel);

    sceneEntity = new BT_EntityTriangleMesh(overWorld, *sceneModelInstance);
    sceneEntity->load();

    scene = new Scene(camera, window);
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
    scene->getBatchRenderTable()->add("sceneModel", sceneModelInstance);
    //scene->getBatchRenderTable()->add("appleModel", appleMeshInstance);

    scene->getBatchShader().use();
    scene->getBatchShader().setVec3f("spotLights[0].position", camera->getPosition());
    scene->getBatchShader().setVec3f("spotLights[0].direction", camera->getDirection());
    window->addEventHandler(scene);

    cubemapFramebuffer = new CubemapFramebuffer(scene);
    cubemapFramebuffer->getRefractionRenderTable()->add("apple", appleMeshInstance);
    cubemapFramebuffer->create(true);

    shadowMapper->getDepthRenderTable()->add(cubemapFramebuffer->getRefractionRenderTable());

    framebuffer = new Framebuffer(scene);
    framebuffer->getRenderTable()->add("scene", scene);
    framebuffer->getRenderTable()->add("reflectives", cubemapFramebuffer);
    framebuffer->create(800, 600, true);

    window->getWindowRenderTable()->add("overWorld", overWorld);
    window->getWindowRenderTable()->add("appleEntity", appleEntity);
    window->getWindowRenderTable()->add("sceneEntity", sceneEntity);
    window->getWindowRenderTable()->add("framebuffer", framebuffer);

    SDL_GL_SetSwapInterval(0);
    SDL_SetWindowRelativeMouseMode(window->getSDLWindowPtr(), true);
}

void render_Inputs(GameWindow *window) {
    sdl_process_keys(window->getSDLWindowPtr());
    window->forEachFrameEvents([](const SDL_Event event){sdl_process_mouse(event);});
}

void render_ImGui() {
    window->forEachFrameEvents([](const SDL_Event event){ImGui_ImplSDL3_ProcessEvent(&event);});
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.0f", (window->getLastFrameTime() == 0) ? 999.0f : 1.0f / window->getLastFrameTime());

    if (ImGui::Button("Reset")) {
        window->getWindowRenderTable()->wipe("appleEntity");

        appleEntity = new BT_EntityConvexHull(overWorld, 0.2f, appleModel->meshes["Hitbox"]);
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
    window->attrib(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
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