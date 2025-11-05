#include "SampleGame.hpp"

#include "SampleCallbacks.hpp"
#include "Syngine/engine/Concurrency.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/BatchRenderer.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/ShadowMapper.hpp"
#include "Syngine/serialization/DataSerializer.hpp"
#include "Syngine/world/Coordination.hpp"

#include "glm/fwd.hpp" 

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include <filesystem>
#include <string>
#include <vector>

#define SCR_WIDTH  1280
#define SCR_HEIGHT 720

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
 *   - [*] Serialize/Deserialize Game Data (SynPack format "assets.spk")
 *   - [*] Web Support (Emscripten)
 *   - [*] Gamma correction (*) -> Basic HDR (*) -> Normal Mapping (*) -> Parallax Mapping (*)
 *   - [-] Physics-Based Rendering -> lacks environment maps, but could be implemented easily if needed ( )
 *   - [ ] Physics-Based PointLights & SpotLights
 *   - [ ] Use Block-Compression method (S3 BCn) for raw image data compression/decompression at runtime
 *   - [ ] Use Google Crashpad to catch segmentation errors and debug memory dumps to fix them ASAP if happened on client's PC
 *   - [ ] GLTF/FBX Animations! VERY VERY IMPORTANT
 *   - [ ] Global Asset Manager: Read/Write Shaders ( ), Read/Write Materials (Textures + Metadata + PBR) ( ), Read/Write Models ( ), Read/Write Meshes ( ) <bind/release meshes in model>
 *   - [-] Batching: Reduce GPU State Changes by once binding to materials for each mesh (*) -> Batched VAO Model Instances (BVMI, One Draw Call) ( ) -> BVMI + Atlased Textures ( )
 *   - [ ] UI Rendering: Text Rendering ( ) -> Mesh2D "Quads, static buttons, images etc." (-) -> Batched Mesh2D, Text, etc (defined by U.V. template) ( )
 *   - [-] Shadow Mapping: Directional Shadows (*) -> Point Shadows ( ) -> Cascaded Shadow Mapping ( )
 *   - [ ] Multi Shader Support for Scene and inherited renderable objects
 *   - [ ] Bright Parts Renderer -> Bloom ( ), Sun Rays "sometimes called God Rays" ( )
 *   - [ ] Advanced HDR: auto exposure adjustment by average luminance
 *   - [ ] SSAO, HBAO "With help of compute shaders"
 *   - [ ] Unfolded one-pass spherical Point Shadow Maps
 *   - [ ] Clustered-Forward Rendering supporting both Blinn-Phong & Physics-Based Rendering
 *   - [ ] Volumetric Fog
 *   - [ ] One Draw call Particles
 *   - [-] https://github.com/kcat/openal-soft for 3D Audio -> Implement Gaming Audio System in Syngine: audio/AudioBuffer, audio/AudioSource, audio/AudioUtil, audio/WAV/OGG/MP3 etc. ( )
 *   - [-] Make an "install" task in CMake for publishing Syngine + Bullet + etc. dependent headers + shared libs.
 *   - [-] Game Modeling + Design (Low Poly? High Constrast colors?)
 *   - [-] Game UI (VHS Style Menus? RmlUI? idk)
 *   - [ ] Networking (via ASIO & protobuf) + ANSI Server
 *   - [ ] Produce (Demo via itch.io, Paid on Steam)
 *   - [ ] < Android Support (Fully based off C++ using Android NDK) >
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
    // Model* helmetModel = new Model();

    // ModelIO::AssimpReader reader = {"assets/models/Sponza/glTF/sponza.gltf"};
    // reader.loadPBRTextures = true;
    // sceneModel->readAssimp(reader);
    // DataSerializer serializer(1024 * 1024 * 512);
    // sceneModel->serialize(ModelIO::PackedWriter(&serializer));
    // serializer.serialize(std::filesystem::current_path() / "sponza.spk");

    DataDeserializer sponzaPacked(std::filesystem::current_path() / "sponza.spk");
    sceneModel->readPacked(ModelIO::PackedReader(&sponzaPacked));
    sceneModel->uploadVertices(syng::CacheApproach::Interleaved, true);
    sceneModel->uploadTextures();

    // ModelIO::AssimpReader reader = {"assets/models/Avocado/glTF/Avocado.gltf"};
    // reader.loadPBRTextures = true;
    // helmetModel->readAssimp(reader);
    // helmetModel->uploadVertices(syng::CacheApproach::Interleaved, true);
    // helmetModel->uploadTextures();

#ifdef __EMSCRIPTEN__
    batchShader.read("assets/shaders/ES/batchVertex.glsl", "assets/shaders/ES/batchFrag.glsl");
    screenShader.read("assets/shaders/ES/screenVertex.glsl", "assets/shaders/ES/screenFrag.glsl");
#else
    batchShader.read("assets/shaders/PBRbatchVertex.glsl", "assets/shaders/PBRbatchFrag.glsl");
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
    // ModelInstance* helmetInstance = new ModelInstance(helmetModel);

    materialBatch = new MaterialBatchRenderer(scene);
    materialBatch->add(sceneModelInstance);
    //materialBatch->add(helmetInstance);
    materialBatch->sort(DEFAULT_BATCH_SORT);
    scene->getBatchRenderTable()->add("materialBatch", materialBatch);

    // helmetInstance->setScale(glm::vec3(10.0f));

    DirLight dayLight = {
        {-0.86f, -1.0f, -0.97f},
        {0.3f, 0.3f, 0.15f},
        {0.35f, 0.35f, 0.14f},
        {0.6f, 0.6f, 0.25f}
    };
#ifndef __EMSCRIPTEN__
    dayLight.ambient *= 2.5f * 300.0f;
    dayLight.diffuse *= 3.5f * 500.0f;
    dayLight.specular *= 6.0f * 600.0f;
#endif
    scene->setDirectionalLight(dayLight);
    scene->reloadShaders();

    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
    glm::vec3 lightPos = -lightDir * 50.0f;
#ifdef __EMSCRIPTEN__
    depthShader.read("assets/shaders/ES/depthVertex.glsl", "assets/shaders/ES/depthFrag.glsl");
#else
    depthShader.read("assets/shaders/PBRdepthVertex.glsl", "assets/shaders/PBRdepthFrag.glsl");
#endif
    shadowMapper = new ShadowMapper(depthShader, 4096, glm::vec3(0.0f), lightDir, lightPos);
    shadowMapper->strength = 1.0f;
    shadowMapper->biasMax = 0.0;
    shadowMapper->biasMin = 0.0005;
    shadowMapper->create();
    scene->withShadows(shadowMapper);

    keyHandler = new SampleKeyHandler(this);
    mouseEventHandler = new SampleMouseEventHandler(this);

    window->addEventHandler(scene);
    window->addEventHandler(mouseEventHandler);

    framebuffer = new Framebuffer(scene);
    framebuffer->setTCBFiltering(GL_LINEAR);
#ifndef __EMSCRIPTEN__
    framebuffer->setTCBFormat(GL_RGBA16F);
    framebuffer->setHDR({0.036f});
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
    ImGui::SliderFloat("HDR Exposure", &hdrExposure, 0.0f, 1.0f, "%.3f");
    ImGui::Checkbox("Mouse Captured", &mouseCaptured);
    ImGui::SliderFloat("Bias min", &shadowMapper->biasMin, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Bias max", &shadowMapper->biasMax, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("IBL Radiance Lambertian Factor", &IBLRadianceLambertianFactor, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("IBL Radiance GGX Factor", &IBLRadianceGGXFactor, 0.0f, 1.0f, "%.3f");

    ImGui::End();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void SampleGame::renderETC() {
    scene->setGamma(gamma);
    scene->getBatchShader().use();
    scene->getBatchShader().setFloat("roughnessConstrant", roughnessConstrant);
#ifndef __EMSCRIPTEN__
    skybox->hdrBoost = glm::vec3(hdrSkyBoost);
    framebuffer->setHDR({hdrExposure});
    scene->setPBR_IBLRadianceLambertianFactor(IBLRadianceLambertianFactor);
    scene->setPBR_IBLRadianceGGXFactor(IBLRadianceGGXFactor);
#endif
}

void SampleGame::cleanup() {
#ifndef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
#endif
}

int main() {
    return SampleGame().launch();
}