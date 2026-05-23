#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <../src/camera.h>
#include "cammode.h"

// -----------------------------------------------------------------------
//  GLOBALS
// -----------------------------------------------------------------------
extern char speedIncrease;
extern float   deltaTime;

extern Camera  camera;
extern CamMode camMode;

extern bool filterBlur;
extern bool filterSharpen;
extern bool filterScanline;

extern bool hideHud;
extern bool pitstop;
extern bool hideBC;
extern bool chromaKeyActive;
extern bool legacyDriving;

// -----------------------------------------------------------------------
//  CALLBACKS
// -----------------------------------------------------------------------
inline void framebuffer_size_callback(GLFWwindow *window, int width, int height)
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
    speedIncrease = 'n';
    static bool tabWasPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // cycle door de cam modes met TAB
    bool tabNow = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    if (tabNow && !tabWasPressed)
        camMode = static_cast<CamMode>((camMode + 1) % 4);
    tabWasPressed = tabNow;

    static bool fWas = false, gWas = false, hWas = false, jWas = false;

    bool fNow = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    bool gNow = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
    bool jNow = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;

    if (fNow && !fWas) filterSharpen  = !filterSharpen;
    if (gNow && !gWas) filterBlur     = !filterBlur;
    if (jNow && !jWas) filterScanline = !filterScanline;

    fWas = fNow; gWas = gNow; jWas = jNow;

    // hud hiding
    static bool hideHudWas = false;
    bool toggleHudNow = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;

    if (toggleHudNow && !hideHudWas)
        hideHud = !hideHud;

    hideHudWas = toggleHudNow;

    // pitsop call
    static bool pitLast = false;
    bool pitNow = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;

    if (pitNow && !pitLast)
        pitstop = !pitstop;

    pitLast = pitNow;

    // bezier curve hiding
    static bool hideBCWas = false;
    bool toggleBCNow = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;

    if (toggleBCNow && !hideBCWas)
        hideBC = !hideBC;

    hideBCWas = toggleBCNow;

    // chroma keying hiding
    static bool hideKCWas = false;
    bool toggleKCNow = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;

    if (toggleKCNow && !hideKCWas)
        chromaKeyActive = !chromaKeyActive;

    hideKCWas = toggleKCNow;

    // legacyDriving toggle
    static bool LDWas = false;
    bool toggleLDNow = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

    if (toggleLDNow && !LDWas)
        legacyDriving = !legacyDriving;

    LDWas = toggleLDNow;

    // Free-cam movement
    if (camMode == CAM_FREE)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
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
    else
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            speedIncrease = 's'; // snel
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            speedIncrease = 't'; // traag
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            speedIncrease = 'r'; // rem
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            speedIncrease = 'a'; // achteruit
        }
    }
}
