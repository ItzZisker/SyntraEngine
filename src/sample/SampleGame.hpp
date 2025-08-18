#pragma once

#include "Syngine.hpp"
#include "SampleCallbacks.hpp"
#include "engine/RenderTable.hpp"
#include "modules/CubemapFramebuffer.hpp"
#include "modules/Framebuffer.hpp"
#include "modules/Model.hpp"
#include "modules/ModelInstance.hpp"
#include "modules/Scene.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "modules/ShadowMapper.hpp"
#include "modules/Skybox.hpp"
#include "world/entity/BT_EntityConvexHull.hpp"
#include "world/entity/BT_EntityTriangleMesh.hpp"
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