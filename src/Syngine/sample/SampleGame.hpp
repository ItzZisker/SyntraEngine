#pragma once

#include "SampleCallbacks.hpp"

#include "Syngine/Syngine.hpp"

#include "Syngine/engine/RenderTable.hpp"

#include "Syngine/modules/CascadedShadowMapper.hpp"
#include "Syngine/modules/BatchRenderer.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/EnvironmentMap.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/Skybox.hpp"

#include "Syngine/world/World.hpp"
#include "Syngine/world/entity/BT_EntityConvexHull.hpp"
#include "Syngine/world/entity/BT_EntityTriangleMesh.hpp"

using namespace syng;

static const int maxFrameCount = 30;

class SampleGame {
public:
    GameWindow *window;
    Camera *camera;
    Scene *scene;

    EnvironmentMap *environmentMap;
    MaterialBatchRenderer *materialBatch;
    CascadedShadowMapper *shadowMapper;
    Framebuffer *framebuffer, *framebuffer_VHS;
    Skybox *skybox;

    float framerate[maxFrameCount];
    int frameCount;

    float gamma = 2.2f;
    float yaw = 0, pitch = 0;
    float hdrExposure = 0.06f;
    float hdrSkyBoost = 30.0f;
    float roughnessConstrant = 1.0f;
    bool mouseCaptured = true;
    bool firstMouse;

    SampleMouseEventHandler *mouseEventHandler;
    SampleKeyHandler *keyHandler;

    BT_World* overWorld;

    Model *appleModel, *sceneModel;
    ModelInstance *sceneModelInstance;
    MeshInstance *appleHMeshInstance, *appleMeshInstance;

    Shader batchShader, screenShader, skyboxShader, depthShader, irrShader;

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