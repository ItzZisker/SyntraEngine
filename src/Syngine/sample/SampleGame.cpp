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
#include "Syngine/modules/ShadowMapper.hpp"
#include "Syngine/world/Coordination.hpp"

#include "glm/fwd.hpp" 

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include <filesystem>
#include <string>
#include <vector>

#define SCR_WIDTH  1024
#define SCR_HEIGHT 768

/* TODO:
 *   === SEIZURE PROGRAM (Lethal-like COOP Video Game from scratch) ===
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
 *   - [*] Rebuild Bullet linked all in one libBullet3.dll (Impossible, linked them statically, much cleaner)
 *   - [*] Test/Load Sample GLTF Models by Standard
 *   - [*] GLVertex, GLVertexElement, GLObjects, cleaner vertex read/write to GPU
 *   - [*] Model mesh-tree traversal, each mesh has its own local transform (*) -> MeshInstances followed by ModelInstances (*)
 *   - [*] Anti-Aliasing: MSAA (*) -> FXAA (*)
 *   - [ ] Global Asset Manager: Read/Write Shaders ( ), Read/Write Materials (Textures + Metadata + PBR) ( ), Read/Write Models ( ), Read/Write Meshes ( ) <bind/release meshes in model>
 *   - [ ] Batching: Reduce GPU State Changes by once binding to materials for each mesh (*) -> Batched VAO Model Instances (BVMI) ( ) -> BVMI + Atlased Textures ( )
 *   - [ ] UI Rendering: Text Rendering ( ) -> Mesh2D "Quads, static buttons, images etc." (-) -> Batched Mesh2D, Text, etc (defined by U.V. template) ( )
 *   - [-] Shadow Mapping: Directional Shadows (*) -> Point Shadows (*) -> Cascaded Shadow Mapping ( )
 *   - [-] One Draw call Particles ( ) | Gamma correction (*) -> HDR (*) -> Bloom ( ) -> Normal Mapping (*) -> Parallax Mapping (*)
 *   - [ ] Make an "install" task in CMake for publishing Syngine + Bullet + etc. dependent headers + shared libs.
 *   - [ ] Multi Shader Support for Scene and inherited renderable objects
 *   - [ ] Deferred Rendererer as an object in the scene rendering tree
 *   - [ ] Unfolded one-pass spherical Point Shadow Maps
 *   - [ ] SSAO (+ < Game Menu Option >)
 *   - [ ] Physics-Based Rendering
 *   - [*] Web Support (Emscripten)
 *   - [ ] Android Support (Fully based off C++ using Android NDK)
 *   - [ ] < Make format parser for special nodes name (Using gltf's custom properties + assimp) ([B]LP_: [Bloom]PointLight, [B]LS_: [Bloom]SpotLight, R_: Renderable mesh) >
 *   - [ ] < Room to Room Lighting System > (Filter lights for specific meshes in a room, so meshes behind the walls won't get lit, Only usable for static pointlights)
 *   - [*] < Serialize/Deserialize Game Data > (SynPack format "assets.spk")
 *   - [-] < Review https://github.com/kcat/openal-soft for 3D Audio > -> Implement Gaming Audio System in Syngine ( )
 *   - [-] < Game Modeling + Design (Low Poly? High Constrast colors?) >
 *   - [-] < Game UI (VHS Style Menus? idk) >
 *   - [ ] < Networking (via ASIO & protobuf) + ANSI Server >
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
#ifndef __EMSCRIPTEN__
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window->getSDLWindowPtr(), window->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
}

void SampleGame::createWindow(GameWindow *window) {
    overWorld = new BT_World(0, "overworld");
    camera = new Camera(glm::vec3(5.0f, 0.0f, 5.0f), yaw, pitch);

    Model* sceneModel = new Model();

    sceneModel->readAssimp({"assets/models/Sponza/glTF/Sponza.gltf"});
    sceneModel->uploadVertices(CacheApproach::Interleaved);

#ifdef __EMSCRIPTEN__
    batchShader.read("assets/shaders/ES/batchVertex.glsl", "assets/shaders/ES/batchFrag.glsl");
    screenShader.read("assets/shaders/ES/screenVertex.glsl", "assets/shaders/ES/screenFrag.glsl");
#else
    batchShader.read("assets/shaders/batchVertex.glsl", "assets/shaders/batchFrag.glsl");
    screenShader.read("assets/shaders/screenVertex.glsl", "assets/shaders/screenFrag.glsl");
#endif

    Scene_T props = {
        .width = SCR_WIDTH,
        .height = SCR_HEIGHT,
        .zNear = 0.1f,
        .zFar = 100.0f
    };
    scene = new Scene(camera, batchShader, screenShader, props);

#ifdef __EMSCRIPTEN__
    skyboxShader.read("assets/shaders/ES/skyboxVertex.glsl", "assets/shaders/ES/skyboxFrag.glsl");
#else
    skyboxShader.read("assets/shaders/skyboxVertex.glsl", "assets/shaders/skyboxFrag.glsl");
#endif
    skybox = new Skybox(scene, skyboxShader);
    skybox->hdrBoost = glm::vec3(hdrSkyBoost);
    skybox->load({
        "assets/skybox/daylight/right.bmp",
        "assets/skybox/daylight/left.bmp",
        "assets/skybox/daylight/top.bmp",
        "assets/skybox/daylight/bottom.bmp",
        "assets/skybox/daylight/front.bmp",
        "assets/skybox/daylight/back.bmp"
    });
    scene->getBatchRenderTable()->add("skybox", skybox);

    sceneModelInstance = new ModelInstance(sceneModel);

    materialBatch = new MaterialBatchRenderer(scene);
    materialBatch->add(sceneModelInstance);
    materialBatch->sort(DEFAULT_BATCH_SORT);
    scene->getBatchRenderTable()->add("materialBatch", materialBatch);

    DirLight dayLight = {
        {-0.86f, -1.0f, -0.97f},
        {0.3f, 0.3f, 0.2f},
        {0.5f, 0.5f, 0.3f},
        {0.6f, 0.6f, 0.45f}
    };
#ifndef __EMSCRIPTEN__
    // dayLight.ambient *= 25.0f * (hdrSkyBoost / 20.0f);
    // dayLight.diffuse *= 35.0f * (hdrSkyBoost / 20.0f);
    // dayLight.specular *= 60.0f * (hdrSkyBoost / 20.0f);
#endif
    scene->setDirectionalLight(dayLight);
    scene->reloadShaders();

    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
    glm::vec3 lightPos = -lightDir * 50.0f;
#ifdef __EMSCRIPTEN__
    depthShader.read("assets/shaders/ES/depthVertex.glsl", "assets/shaders/ES/depthFrag.glsl");
#else
    depthShader.read("assets/shaders/depthVertex.glsl", "assets/shaders/depthFrag.glsl");
#endif
    shadowMapper = new ShadowMapper(depthShader, 4096, glm::vec3(0.0f), lightDir, lightPos);
    shadowMapper->strength *= 2;
    shadowMapper->biasMax *= 0.18f;
    shadowMapper->biasMin *= 0.05f;
    shadowMapper->create();
    scene->withShadows(shadowMapper);

    keyHandler = new SampleKeyHandler(this);
    mouseEventHandler = new SampleMouseEventHandler(this);

    window->addEventHandler(scene);
    window->addEventHandler(mouseEventHandler);

    framebuffer = new Framebuffer(scene);
    framebuffer->setTCBFiltering(GL_LINEAR);
#ifndef __EMSCRIPTEN__
    // framebuffer->setTCBFormat(GL_RGBA16F);
    // framebuffer->setHDR({0.036f});
#endif
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
#ifndef __EMSCRIPTEN__
    window->forEachFrameEvents([](const SDL_Event event){ImGui_ImplSDL3_ProcessEvent(&event);});

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    frameCount++;
    if (frameCount >= maxFrameCount) frameCount = 0;
    framerate[frameCount] = (window->getLastFrameTime() == 0) ? 999.0f : 1.0f / window->getLastFrameTime();

    float FPS = 0;
    for (int i = 0; i < maxFrameCount; i++) FPS += framerate[i];
    FPS /= maxFrameCount;

    ImGui::NewFrame();
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.0f", FPS);

    ImGui::SliderFloat("Gamma", &gamma, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("HDR Boost (Skybox)", &hdrSkyBoost, 0.0f, 100.0f, "%.3f");
    ImGui::SliderFloat("HDR Exposure", &hdrExposure, 0.0f, 0.1f, "%.3f");
    ImGui::Checkbox("Mouse Captured", &mouseCaptured);

    ImGui::End();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void SampleGame::renderETC() {
    scene->setGamma(gamma);
    scene->getBatchShader().use();
    scene->getBatchShader().setFloat("roughnessConstrant", roughnessConstrant);
    //skybox->hdrBoost = glm::vec3(hdrSkyBoost);
    //framebuffer->setHDR({hdrExposure});
}

void SampleGame::cleanup() {
#ifndef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
#endif
}

int main() {
    Concurrency::initMainThread();
    SampleGame *game = new SampleGame();
    return game->launch();
}