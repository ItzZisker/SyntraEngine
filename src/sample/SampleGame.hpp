#pragma once

#include "Syngine.hpp"
#include "SampleCallbacks.hpp"
#include "modules/CubemapFramebuffer.hpp"
#include "modules/Framebuffer.hpp"
#include "modules/Model.hpp"
#include "modules/ModelInstance.hpp"
#include "modules/Scene.hpp"
#include "modules/ShadowMapper.hpp"
#include "modules/Skybox.hpp"
#include "world/entity/BT_EntityConvexHull.hpp"
#include "world/entity/BT_EntityTriangleMesh.hpp"

using namespace syng;

class SampleGame {
public:
    GameWindow *window;
    Camera *camera;
    Scene *scene;

    ShadowMapper *shadowMapper;
    Framebuffer *framebuffer;
    CubemapFramebuffer *cubemapFramebuffer;
    Skybox *skybox;

    float lX = 0.0f, lY = 2.0f, lZ = 0.0f;
    float gamma = 1.1f;
    float yaw = 0, pitch = 0;
    
    bool mouseCaptured = true;
    bool firstMouse;

    SampleMouseEventHandler *mouseEventHandler;
    SampleKeyHandler *keyHandler;

    BT_World* overWorld;

    Model *appleModel, *sceneModel;
    ModelInstance *sceneModelInstance;
    MeshInstance *appleHMeshInstance, *appleMeshInstance;

    BT_EntityConvexHull* appleEntity;
    BT_EntityTriangleMesh* sceneEntity;

    int launch();
private:
    void createImGUI();
    void createWindow(GameWindow *window);

    void renderImGUI();
    void renderETC();

    void cleanup();
};