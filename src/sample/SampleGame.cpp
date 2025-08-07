#include "SampleGame.hpp"

#include "SampleCallbacks.hpp"
#include "engine/RenderTable.hpp"
#include "utils/GameUtils.hpp"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "modules/Framebuffer.hpp"
#include "modules/Mesh.hpp"
#include "modules/Scene.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <ostream>


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
 *   - [-] Flame Particles ( ) | Gamma correction (*) -> HDR ( ) -> Bloom ( ) -> Normal Mapping (*) -> Parallax Mapping (*) -> PBR Textures ( )
 *   - [ ] Test/Load Sample GLTF Models by Standard
 *   - [ ] < Make format parser for mesh nodes name (Using gltf's custom properties + assimp) (ECH_: Entity Convex Hull, ETM_: Entity Triangle Mesh, PF_: FlameParticle, [B]LP_: [Bloom]PointLight, [B]LS_: [Bloom]SpotLight, R_: Renderable mesh) >
 *   - [ ] < Serialize/Deserialize Game Data >
 *   - [ ] < Review https://github.com/kcat/openal-soft for 3D Audio >
 *   - [ ] < Game Modeling + Design (Low Poly? High Constrast colors?) >
 *   - [ ] < Game UI (VHS Style Menus? idk) >
 *   - [ ] < Networking (via Facebook Wangle) + ANSI Server >
 *   - [ ] < Produce (Demo via itch.io, Paid on Steam) >
 */ 

int SampleGame::launch() {
    window = new GameWindow("Sample", {1024, 768});
    window->attrib(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    window->addInitTask([&](GameWindow *window){ 
        createImGUI();
        createWindow(window);
    });
    window->addRenderTask([&](GameWindow *window){
        renderImGUI();
        renderETC();
    });
    int exitCode = window->initLoop();
    cleanup();
    return exitCode;
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
    camera = new Camera(overWorld, glm::vec3(5.0f, 0.0f, 5.0f), yaw, pitch);

    // appleModel = new Model("models/apple2/apple.obj");
    // appleModel->filterMesh("Apple");
    // appleModel->loadModel(Interleaved);

    // std::cout << "A\n";
    // appleHMeshInstance = new MeshInstance(appleModel->meshes["Hitbox"]);
    // appleHMeshInstance->setScale(glm::vec3(0.5f, 1.0f, 0.5f));

    // std::cout << "B\n";
    // appleMeshInstance = new MeshInstance(appleModel->meshes["Apple"]);
    // appleMeshInstance->setScale(glm::vec3(0.5f, 1.0f, 0.5f));

    // Crashing at "F" I have no clue whatsoever

    // std::cout << "C\n";
    // appleEntity = new BT_EntityConvexHull(overWorld, 0.2f, appleHMeshInstance);
    // std::cout << "D\n";
    // appleEntity->setPosition(glm::vec3(0, 10, 0));
    // std::cout << "E\n";
    // appleEntity->bind("Apple", appleMeshInstance);
    // std::cout << "F\n";
    // appleEntity->load(false);
    // std::cout << "G\n";

    std::cout << "scene\n";
    sceneModel = new Model("models/wall/2g/wall.gltf");
    std::cout << "H\n";
    sceneModel->loadModel(Interleaved);
    std::cout << "scene done\n";

    std::string dir = std::filesystem::current_path().string();
    for (auto& pair : sceneModel->meshes) {
        Mesh* mesh = pair.second;
        for (auto& tex : mesh->textures) {
            if (GameUtils::str_contains(tex.path, "beige_wall_001")) {
                sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/beige_wall_001_nor_gl_1k.jpg", dir, "texture_normal"));
                sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/beige_wall_001_disp_1k.png", dir, "texture_height"));
            }
            if (GameUtils::str_contains(tex.path, "laminate_floor_03")) {
                sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/laminate_floor_03_nor_gl_1k.png", dir, "texture_normal"));
                sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/laminate_floor_03_disp_1k.png", dir, "texture_height"));
            }
            if (GameUtils::str_contains(tex.path, "paper_0033")) {
                sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/paper_0033_normal_opengl_1k.png", dir, "texture_normal"));
                //sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/paper_0033_height_1k.png", dir, "texture_height"));
            }
            if (GameUtils::str_contains(tex.path, "wood_table_001")) {
                sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/wood_table_001_nor_gl_1k.png", dir, "texture_normal"));
                //sceneModel->pushTexture(pair.first, TextureFromFile("models/wall/2g/wood_table_001_disp_1k.png", dir, "texture_height"));
            }
        }
    }

    //sceneModel->meshes["Plane"]->textures.push_back(bricks2disp);

    sceneModelInstance = new ModelInstance(sceneModel);
    sceneModelInstance->getMeshInstances()->forEach([](const std::string& key, MeshInstance* meshInstance){
        std::cout << "IA: " << key << std::endl;
    });
    std::cout << "3\n";
    //sceneModelInstance->getMeshInstances()->get("Cube")->getMesh()->material.opacity = 0.5f;
    //sceneModelInstance->getMeshInstances()->sort(RT_SORT_OPACITY);

    //sceneEntity = new BT_EntityTriangleMesh(overWorld, sceneModelInstance);
    //sceneEntity->load();
    std::cout << "4\n";

    scene = new Scene(camera, window);
    scene->setZBufferLayout(0.1f, 100.0f);
    // shadowMapper = new ShadowMapper(2048);
    // shadowMapper->strength = 1.0f;
    // shadowMapper->pcfRadius = 2;
    // shadowMapper->create();
    // scene->withShadows(shadowMapper);
    std::cout << "5\n";

    skybox = new Skybox(scene, {
        "models/skybox/lightblue/right.png",
        "models/skybox/lightblue/left.png",
        "models/skybox/lightblue/top.png",
        "models/skybox/lightblue/bottom.png",
        "models/skybox/lightblue/front.png",
        "models/skybox/lightblue/back.png"
    });
    skybox->hdrBoost = glm::vec3(hdrSkyBoost);
    skybox->load();
    scene->getBatchRenderTable()->add("skybox", skybox);
    scene->getBatchRenderTable()->add("sceneModel", sceneModelInstance);
    //scene->getBatchRenderTable()->add("appleModel", appleMeshInstance);
    // DirLight daylight = {
    //     {-0.86f, -1.0f, -0.97f},
    //     {0.5f, 0.5f, 0.5f},
    //     {0.75f, 0.75f, 0.5f},
    //     {0.85f, 0.85f, 0.6f}
    // };
    // daylight.ambient *= 25.0f;
    // daylight.diffuse *= 35.0f;
    // daylight.specular *= 60.0f;
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

    //cubemapFramebuffer = new CubemapFramebuffer(scene);
    //cubemapFramebuffer->getRefractionRenderTable()->add("apple", appleMeshInstance);
    //cubemapFramebuffer->create(true);

    //shadowMapper->getDepthRenderTable()->add(cubemapFramebuffer->getRefractionRenderTable());

    //framebuffer_VHS = new Framebuffer(scene, {"shaders/vhsVertex.glsl", "shaders/vhs2Frag.glsl"});
    framebuffer_VHS = new Framebuffer(scene);
    framebuffer_VHS->addRenderTask([&](Framebuffer* buffer){
        Shader outputShader = buffer->getOutputShader();
        outputShader.use();
        outputShader.setFloat("time", SDL_GetTicks() / 1000.0f);
    });
    framebuffer_VHS->setTCBFormat(GL_RGBA16F);
    framebuffer_VHS->setHDR({0.036f});
    framebuffer_VHS->setAntiAliasing(AA_FXAAx1);
    framebuffer_VHS->getRenderTable()->add("scene", scene);
    framebuffer_VHS->create(1024, 768, true);

    framebuffer = new Framebuffer(scene);
    //framebuffer->getRenderTable()->add("reflectives", cubemapFramebuffer);
    framebuffer->setAntiAliasing(AA_OFF);
    framebuffer->addRenderTask([&](Framebuffer* buffer){
        framebuffer_VHS->render(*buffer);
    });
    framebuffer->create(1024, 768, true);

    window->getWindowRenderTable()->add("overWorld", overWorld);
    //window->getWindowRenderTable()->add("appleEntity", appleEntity);
    //window->getWindowRenderTable()->add("sceneEntity", sceneEntity);
    window->getWindowRenderTable()->add("framebuffer", framebuffer_VHS);
    window->getWindowRenderTable()->add("keyHandler", keyHandler);

    SDL_GL_SetSwapInterval(0);
    SDL_SetWindowRelativeMouseMode(window->getSDLWindowPtr(), true);

    std::cout << "init done\n";
}

void SampleGame::renderImGUI() {
    window->forEachFrameEvents([](const SDL_Event event){ImGui_ImplSDL3_ProcessEvent(&event);});

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.0f", (window->getLastFrameTime() == 0) ? 999.0f : 1.0f / window->getLastFrameTime());

    // if (ImGui::Button("Reset")) {
    //     window->getWindowRenderTable()->wipe("appleEntity");

    //     appleEntity = new BT_EntityConvexHull(overWorld, 0.2f, appleHMeshInstance);
    //     appleEntity->load();

    //     window->getWindowRenderTable()->add("appleEntity", appleEntity);
    // }
    // ImGui::SliderFloat("IOR R", &appleMeshInstance->getMesh()->material.ior.x, 1.0f, 2.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    // ImGui::SliderFloat("IOR G", &appleMeshInstance->getMesh()->material.ior.y, 1.0f, 2.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    // ImGui::SliderFloat("IOR B", &appleMeshInstance->getMesh()->material.ior.z, 1.0f, 2.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
    // ImGui::SliderFloat("F0", &appleMeshInstance->getMesh()->material.F0, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    // ImGui::SliderFloat("Opacity (Scene)", &sceneModelInstance->getMeshInstances()->get("Cube")->getMesh()->material.opacity, 0.0f, 1.0f, "%.3f");
    //ImGui::SliderFloat("Opacity", &appleMeshInstance->getMesh()->material.opacity, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Gamma", &gamma, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    //ImGui::SliderFloat("FOV (Reflectives)", &cubemapFramebuffer->fieldOfView, 80.0f, 100.0f, "%.3f");
    // ImGui::SliderFloat("Light X", &lX, -20.0f, 20.0f, "%.3f");
    // ImGui::SliderFloat("Light Y", &lY, -20.0f, 20.0f, "%.3f");
    // ImGui::SliderFloat("Light Z", &lZ, -20.0f, 20.0f, "%.3f");
    ImGui::SliderFloat("HDR Boost (Skybox)", &hdrSkyBoost, 0.0f, 100.0f, "%.3f");
    ImGui::SliderFloat("HDR Exposure", &hdrExposure, 0.0f, 0.1f, "%.3f");
    ImGui::SliderFloat("Roughness Constrant", &roughnessConstrant, 0.1f, 10.0f, "%.3f");
    // ImGui::SliderFloat("Shadow Bias Min", &shadowMapper->biasMin, 0.001f, 1.0f, "%.3f");
    // ImGui::SliderFloat("Shadow Bias Max", &shadowMapper->biasMax, 0.001f, 1.0f, "%.3f");
    // ImGui::SliderFloat("Shadow PCF Scale", &shadowMapper->pcfScale, 0.1f, 10.0f, "%.3f");
    // ImGui::SliderInt("Shadow PCF Radius", (int*) &shadowMapper->pcfRadius, 1, 10);
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
    framebuffer_VHS->setHDR({hdrExposure});
    PointLight pl = {camera->getPosition()};
    pl.ambient = {0.05f, 0.05f, 0.05f};
    pl.diffuse = {0.8f, 0.8f, 0.5f};
    pl.specular = {1.0f, 1.0f, 0.6f};
    pl.boost(18.0f);
    scene->setPointLight(0, pl);
    //static float dT = (float) window->getLastFrameTime();
    //dT += window->getLastFrameTime();
    //sceneModelInstance->getMeshInstances()->get("Cube.001")->setDirection({sin(dT), 0.0f, cos(dT)});
}

void SampleGame::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

int main() {
    SampleGame *game = new SampleGame();
    return game->launch();
}