#include "SampleGame.hpp"

#include "SampleCallbacks.hpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

int SampleGame::launch() {
    window = new GameWindow("Sample", 800, 600);
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

    keyHandler = new SampleKeyHandler(this);
    mouseEventHandler = new SampleMouseEventHandler(this);

    window->addEventHandler(scene);
    window->addEventHandler(mouseEventHandler);

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

void SampleGame::renderETC() {
    scene->getScreenShader().use();
    scene->getScreenShader().setFloat("gamma", gamma);
    scene->getBatchShader().use();
    scene->getBatchShader().setVec3f("pointLights[0].position", lX, 2.0f, 0.0f);
    scene->getBatchShader().setVec3f("spotLights[1].position", camera->getPosition());
    scene->getBatchShader().setVec3f("spotLights[1].direction", camera->getDirection());
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