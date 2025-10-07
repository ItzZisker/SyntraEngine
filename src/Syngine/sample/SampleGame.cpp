#include "SampleGame.hpp"

#include "SampleCallbacks.hpp"
#include "Syngine/engine/Concurrency.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/BatchRenderer.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/world/Coordination.hpp"

#include "glm/fwd.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include <filesystem>
#include <string>
#include <vector>

#define SCR_WIDTH  1360
#define SCR_HEIGHT 1024

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
 *   - [*] GLVertex, GLVertexElement, GLObjects, cleaner vertex read/write to GPU
 *   - [-] Scene2D ( ), Mesh2D (*)
 *   - [ ] Deferred Shading
 *   - [ ] Unfolded one-pass spherical Point Shadow Maps
 *   - [-] Shadow Mapping: Directional Shadows (*) -> Point Shadows (*) -> Cascaded Shadow Mapping ( )
 *   - [ ] SSAO (+ < Game Menu Option >)
 *   - [ ] Anti-Aliasing
 *   - [-] Flame Particles ( ) | Gamma correction (*) -> HDR (*) -> Bloom ( ) -> Normal Mapping (*) -> Parallax Mapping (*) -> PBR Textures ( )
 *   - [ ] Test/Load Sample GLTF Models by Standard
 *   - [ ] < Make format parser for mesh nodes name (Using gltf's custom properties + assimp) (ECH_: Entity Convex Hull, ETM_: Entity Triangle Mesh, PF_: FlameParticle, [B]LP_: [Bloom]PointLight, [B]LS_: [Bloom]SpotLight, R_: Renderable mesh) >
 *   - [ ] < Room to Room Lighting System > (Filter lights for specific meshes in a room, so meshes behind the walls won't get lit, Only usable for static pointlights)
 *   - [ ] < Serialize/Deserialize Game Data >
 *   - [ ] < Review https://github.com/kcat/openal-soft for 3D Audio >
 *   - [ ] < Game Modeling + Design (Low Poly? High Constrast colors?) >
 *   - [ ] < Game UI (VHS Style Menus? idk) >
 *   - [ ] < Networking (via ASIO) + ANSI Server >
 *   - [ ] < Produce (Demo via itch.io, Paid on Steam) >
 */

int SampleGame::launch() {
    window = new GameWindow("Sample", {SCR_WIDTH, SCR_HEIGHT});
    window->attrib(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    window->addInitTask([&](GameWindow *window){ 
        createImGUI();
        createWindow(window);
    });
    window->addRenderTask([&](GameWindow *window){
        renderImGUI();
        renderETC();
    });
    window->addCleanupTask([&](GameWindow *window){
        cleanup();
    });
    return window->initLoop();
}

void SampleGame::createImGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window->getSDLWindowPtr(), window->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");
}

void SampleGame::createWindow(GameWindow *window) {
    overWorld = new BT_World(0, "overworld");
    camera = new Camera(glm::vec3(5.0f, 0.0f, 5.0f), yaw, pitch);

    Model* sceneModel = new Model();

    sceneModel->readAssimp({"models/Sponza/glTF/Sponza.gltf"});
    sceneModel->uploadVertices(CacheApproach::Interleaved);

    batchShader.read("shaders/batchVertex.glsl", "shaders/batchFrag.glsl");
    screenShader.read("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");

    Scene_T props = {
        .width = SCR_WIDTH,
        .height = SCR_HEIGHT,
        .zNear = 0.1f,
        .zFar = 100.0f
    };
    scene = new Scene(camera, batchShader, screenShader, props);

    skyboxShader.read("shaders/skyboxVertex.glsl", "shaders/skyboxFrag.glsl");
    skybox = new Skybox(scene, skyboxShader);
    skybox->hdrBoost = glm::vec3(hdrSkyBoost);
    skybox->load({
        "models/skybox/lightblue/right.png",
        "models/skybox/lightblue/left.png",
        "models/skybox/lightblue/top.png",
        "models/skybox/lightblue/bottom.png",
        "models/skybox/lightblue/front.png",
        "models/skybox/lightblue/back.png"
    });
    scene->getBatchRenderTable()->add("skybox", skybox);

    sceneModelInstance = new ModelInstance(sceneModel);

    modelBatch = new ModelBatchRenderer(scene);
    modelBatch->add("sceneModel", sceneModelInstance);
    scene->getBatchRenderTable()->add("modelBatch", modelBatch);

    DirLight nightlight = {
        {-0.86f, -1.0f, -0.97f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.75f},
        {0.6f, 0.6f, 0.85f}
    };
    nightlight.ambient *= 25.0f * (hdrSkyBoost / 100.0f);
    nightlight.diffuse *= 35.0f * (hdrSkyBoost / 100.0f);
    nightlight.specular *= 60.0f * (hdrSkyBoost / 100.0f);
    scene->setDirectionalLight(nightlight);
    scene->setPointLights({{{lX, lY, lZ}}});
    scene->reloadShaders();
    keyHandler = new SampleKeyHandler(this);
    mouseEventHandler = new SampleMouseEventHandler(this);

    window->addEventHandler(scene);
    window->addEventHandler(mouseEventHandler);

    framebuffer = new Framebuffer(scene);
    framebuffer->setTCBFormat(GL_RGBA16F);
    framebuffer->setTCBFiltering(GL_LINEAR);
    framebuffer->setHDR({0.036f});
    framebuffer->setAntiAliasing(AA_FXAAx4);
    framebuffer->getRenderTable()->add("scene", scene);
    framebuffer->create(SCR_WIDTH, SCR_HEIGHT, true);

    window->getWindowRenderTable()->add("overWorld", overWorld);
    window->getWindowRenderTable()->add("framebuffer", framebuffer);
    window->getWindowRenderTable()->add("keyHandler", keyHandler);

    SDL_GL_SetSwapInterval(0);
    SDL_SetWindowRelativeMouseMode(window->getSDLWindowPtr(), true);
}

void SampleGame::renderImGUI() {
    window->forEachFrameEvents([](const SDL_Event event){ImGui_ImplSDL3_ProcessEvent(&event);});

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.0f", (window->getLastFrameTime() == 0) ? 999.0f : 1.0f / window->getLastFrameTime());

    ImGui::SliderFloat("Gamma", &gamma, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Light X", &lX, -20.0f, 20.0f, "%.3f");
    ImGui::SliderFloat("Light Y", &lY, -20.0f, 20.0f, "%.3f");
    ImGui::SliderFloat("Light Z", &lZ, -20.0f, 20.0f, "%.3f");
    ImGui::SliderFloat("Scene X", &pX, -20.0f, 20.0f, "%.3f");
    ImGui::SliderFloat("Scene Y", &pY, -20.0f, 20.0f, "%.3f");
    ImGui::SliderFloat("Scene Z", &pZ, -20.0f, 20.0f, "%.3f");
    ImGui::SliderFloat("HDR Boost (Skybox)", &hdrSkyBoost, 0.0f, 100.0f, "%.3f");
    ImGui::SliderFloat("HDR Exposure", &hdrExposure, 0.0f, 0.1f, "%.3f");
    ImGui::Checkbox("Mouse Captured", &mouseCaptured);

    ImGui::End();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SampleGame::renderETC() {
    scene->setGamma(gamma);
    skybox->hdrBoost = glm::vec3(hdrSkyBoost);
    scene->getBatchShader().use();
    scene->getBatchShader().setFloat("roughnessConstrant", roughnessConstrant);
    framebuffer->setHDR({hdrExposure});
    PointLight pl = {{lX, lY, lZ}};
    pl.ambient = {0.05f, 0.05f, 0.05f};
    pl.diffuse = {0.8f, 0.8f, 0.5f};
    pl.specular = {1.0f, 1.0f, 0.6f};
    pl.boost(25.0f);
    scene->setPointLight(0, pl);
    sceneModelInstance->setPosition({pX, pY, pZ});
}

void SampleGame::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

int main() {
    Concurrency::initMainThread();
    SampleGame *game = new SampleGame();
    return game->launch();
}