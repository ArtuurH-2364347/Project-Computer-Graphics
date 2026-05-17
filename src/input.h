#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <../src/camera.h>
#include "cammode.h"

// -----------------------------------------------------------------------
//  GLOBALS
// -----------------------------------------------------------------------
extern Camera  camera;
extern float   deltaTime;
extern CamMode camMode;
extern bool filterBlur;
extern bool filterVignette;
extern bool filterSharpen;
extern bool filterScanline;

// -----------------------------------------------------------------------
//  CALLBACKS
// -----------------------------------------------------------------------
inline void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

inline void mouse_callback(GLFWwindow* window, double posX, double posY)
{
    if (glfwWindowShouldClose(window)) return;
    camera.ProcessMouse(window,
        static_cast<float>(posX),
        static_cast<float>(posY));
}

// -----------------------------------------------------------------------
//  KEYBOARD INPUT
// -----------------------------------------------------------------------
inline void processInput(GLFWwindow* window)
{
    static bool tabWasPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // cycle door de cam modes met TAB
    bool tabNow = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    if (tabNow && !tabWasPressed)
        camMode = static_cast<CamMode>((camMode + 1) % 3);
    tabWasPressed = tabNow;

    static bool fWas = false, gWas = false, hWas = false, jWas = false;

    bool fNow = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    bool gNow = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
    bool hNow = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;
    bool jNow = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;

    if (fNow && !fWas) filterSharpen  = !filterSharpen;
    if (gNow && !gWas) filterBlur     = !filterBlur;
    if (hNow && !hWas) filterVignette = !filterVignette;
    if (jNow && !jWas) filterScanline = !filterScanline;

    fWas = fNow; gWas = gNow; hWas = hNow; jWas = jNow;
    
    // Free-cam movement
    if (camMode == CAM_FREE)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD,  deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT,     deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT,    deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP,       deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN,     deltaTime);
    }
}