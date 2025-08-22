#pragma once

#include "Syngine/Syngine.hpp"
#include "SampleCallbacks.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/CubemapFramebuffer.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/ShadowMapper.hpp"
#include "Syngine/modules/Skybox.hpp"
#include "Syngine/world/entity/BT_EntityConvexHull.hpp"
#include "Syngine/world/entity/BT_EntityTriangleMesh.hpp"
#include <vector>

using namespace syng;

class SampleLightRenderer : ShaderRenderable {
public:
    Shader shader;
    std::vector<MeshInstance*> lightBulbs;

    SampleLightRenderer() {
        shader.read("shaders/lightBulbVertex.glsl", "shaders/lightBulbFrag.glsl");
    }

    void render(Shader& shader, Screenbuffer screen = {}) override {
        for (MeshInstance* bulb : lightBulbs) {
            bulb->render(this->shader, screen);
        }
    }
};

class SampleGame {
public:
    GameWindow *window;
    Camera *camera;
    Scene *scene;

    ShadowMapper *shadowMapper;
    Framebuffer *framebuffer, *framebuffer_VHS;
    CubemapFramebuffer *cubemapFramebuffer;
    Skybox *skybox;

    float lX = 0.0f, lY = 2.0f, lZ = 0.0f;
    float gamma = 1.1f;
    float yaw = 0, pitch = 0;
    float hdrExposure = 0.036f;
    float hdrSkyBoost = 5.0f;
    float roughnessConstrant = 1.0f;
    bool mouseCaptured = true;
    bool firstMouse;

    SampleMouseEventHandler *mouseEventHandler;
    SampleKeyHandler *keyHandler;

    BT_World* overWorld;

    Model *appleModel, *sceneModel;
    ModelInstance *sceneModelInstance;
    MeshInstance *appleHMeshInstance, *appleMeshInstance;

    Shader batchShader, screenShader, skyboxShader;

    //BT_EntityConvexHull* appleEntity;
    //BT_EntityTriangleMesh* sceneEntity;

    int launch();
private:
    void createImGUI();
    void createWindow(GameWindow *window);

    void renderImGUI();
    void renderETC();

    void cleanup();
};