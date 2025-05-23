#include <KEngine/KEngine.hpp>
#include <KEngine/modules/Camera.hpp>
#include <KEngine/modules/Model.hpp>

#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sstream>
#include <fstream>

kwindow::GameWindow window("Sample", 800, 600);

glm::mat4 projection = glm::mat4(1.0f);
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

float yaw = 0, pitch;
double lastX, lastY;
bool firstMouse, paused = true;

kcomp::World* overWorld = new kcomp::World(0, "overworld");
Model apple(overWorld, "models/apple/apple.obj");
Camera camera(overWorld, glm::vec3(5.0f, 0.0f, 5.0f), yaw, pitch);

void glfw_mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse) // initially set to true
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed: y ranges bottom to top

    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    camera.setYawPitch(yaw, pitch);
}

void glfw_process_input(GLFWwindow *glfwWindow)
{
    const float cameraSpeed = 1.5f * window.getLastFrameTime(); // adjust accordingly
    glm::vec3 horizontalDirection = glm::vec3(0.0f);

    horizontalDirection.x = cos(glm::radians(yaw));
    horizontalDirection.z = sin(glm::radians(yaw));

    glm::vec3 cameraPos = camera.getPosition();
    glm::vec3 cameraUp = camera.getUp();

    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * horizontalDirection;
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * horizontalDirection;
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;

    camera.setPosition(cameraPos);

    if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS)
        lightPos += glm::vec3(1.0f, 0.0f, 0.0f) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
        lightPos -= glm::vec3(1.0f, 0.0f, 0.0f) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
        lightPos -= glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
        lightPos += glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed;

    if (glfwGetKey(glfwWindow, GLFW_KEY_P) == GLFW_PRESS)
        paused = false;
}

void glfw_framebuffer_resize_callback(GLFWwindow *glfwWindow, int width, int height)
{
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(65.0f), (float)width / (float)height, 0.1f, 100.0f);

    Shader batchShader = window.getBatchShader();
    batchShader.use();
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
}

void init(kwindow::GameWindow *window)
{
    glEnable(GL_DEPTH_TEST);

    stbi_set_flip_vertically_on_load(true);

    apple.loadModel();
    apple.setPosition(glm::vec3(0.0f));

    kwindow::WindowSize size = window->getWindowSize();
    projection = glm::perspective(glm::radians(65.0f), (float)size.width / (float)size.height, 0.1f, 100.0f);

    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window->getGLFWWindowPtr(), glfw_framebuffer_resize_callback);
    glfwSetInputMode(window->getGLFWWindowPtr(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window->getGLFWWindowPtr(), glfw_mouse_callback);

    window->getRenderTable()->add("camera", &camera);
    window->getRenderTable()->add("apple", &apple);
}

void render(kwindow::GameWindow *window)
{
    glfw_process_input(window->getGLFWWindowPtr());

    Shader batchShader = window->getBatchShader();

    batchShader.use();
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);

    batchShader.setVec3f("dirLight.direction", -0.2f, -1.0f, -0.3f);
    batchShader.setVec3f("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    batchShader.setVec3f("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    batchShader.setVec3f("dirLight.specular", 0.5f, 0.5f, 0.5f);

    batchShader.setVec3f("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    batchShader.setVec3f("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    batchShader.setVec3f("spotLight.specular", 1.0f, 1.0f, 1.0f);
    batchShader.setFloat("spotLight.constant", 1.0f);
    batchShader.setFloat("spotLight.linear", 0.09f);
    batchShader.setFloat("spotLight.quadratic", 0.032f);
    batchShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    batchShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

int main()
{
    window.addInitTask([](kwindow::GameWindow *window)
                       { init(window); });
    window.addRenderTask([](kwindow::GameWindow *window)
                         { render(window); });
    window.initWindow();

    return 0;
}