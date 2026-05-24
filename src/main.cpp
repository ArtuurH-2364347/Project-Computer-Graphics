// glad and GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// standard
#include <iostream>
#include <filesystem>
#include <vector>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stb_image
#include <stb_image/stb_image.h>

// self made files
#include <../src/shader/shader.h>
#include <../src/camera.h>
#include <../src/mesh.h>
#include <../src/model.h>
#include "circuit.h"
#include "input.h"
#include "lighting.h"
#include "skybox/skybox.h"
#include "config.h"
#include "cammode.h"
#include <../src/hud/hud.h>

using namespace std;

// -----------------------------------------------------------------------
//  SETTINGS
// -----------------------------------------------------------------------
const int screenWidth  = SCREEN_WIDTH;
const int screenHeight = SCREEN_HEIGHT;
const float carSize = CAR_SIZE;

// -----------------------------------------------------------------------
//  GLOBALS
// -----------------------------------------------------------------------
Camera camera;
CamMode camMode = CAM_FOLLOW;

float  deltaTime = 0.0f;
float  lastFrame = 0.0f;
char speedIncrease = 'n'; // nothing

glm::vec3 tireStackPos;

// hud toggle
bool hideHud = false;

// pitsop
bool pitstop = false;

// bezier toggle
bool hideBC = false;

// filter toggles
bool filterBlur     = false;
bool filterSharpen  = false;
bool filterScanline = false;

// chroma key toggle
bool chromaKeyActive = false;

// legacy driving toggle
bool legacyDriving = false;

// -----------------------------------------------------------------------
//  PATH DEBUG LINE
// -----------------------------------------------------------------------
    struct PathMesh
{
    unsigned int VAO, VBO;
    int          vertCount;
};

PathMesh buildPathMesh(const vector<BezierSegment>& circuit)
{
    const int PATH_STEPS = 20;
    vector<float> verts;

    for (const auto& seg : circuit) {
        for (int i = 0; i <= PATH_STEPS; i++) {
            float     t = (float)i / PATH_STEPS;
            glm::vec3 p = cubicBezier(seg.P0, seg.C0, seg.C1, seg.P1, t);
            verts.push_back(p.x);
            verts.push_back(p.y + 0.05f);
            verts.push_back(p.z);
        }
    }

    PathMesh pm;
    pm.vertCount = (int)verts.size() / 3;

    glGenVertexArrays(1, &pm.VAO);
    glGenBuffers(1, &pm.VBO);
    glBindVertexArray(pm.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, pm.VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return pm;
}

// -----------------------------------------------------------------------
//  MAIN
// -----------------------------------------------------------------------
int main()
{
    cout << "BEGINNING OF THE DEBUG CONSOLE\n" << endl;

    // ------------------------------------------------------------------
    //  GLFW / OpenGL init
    // ------------------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight,
                                          "F1 Computer Graphics Project", NULL, NULL);
    if (!window)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_click_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ---- Window Icon ----
    GLFWimage images[1];
    images[0].pixels = stbi_load("models/icon/F1-Logo.png", &images[0].width, &images[0].height, 0, 4); // rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
        // ---- Model and Shader Declaration ----
        Shader myShader("src/shader/shader.vs", "src/shader/shader.fs");
        Model  myModel(filesystem::path("models/2021_F1_Mercedes-Benz_W12/2021_F1_Mercedes-Benz_W12.obj")); // => obj file
        // Model myModel(filesystem::path("models/2021_f1_mercedes-benz_w12_gLTF/scene.gltf")); // => gltf file

        // ---- Track Laden en Aanpassen ----
        Model myTrack(filesystem::path("models/nurburgring_race_driver_grid_ds_gltf/scene.gltf"));
        glm::vec3 circuitPos = glm::vec3(-420.0f, -20.0f, 390.0f);

        // ---- Tires Laden en Aanpassen
        Model myTires(filesystem::path("models/tire-stack/source/Untitled3.obj"));
        tireStackPos = glm::vec3(120.0f, 0.75f, -250.9f);

        std::vector<std::string> skyFaces = {
            "models/skybox/right.jpg",
            "models/skybox/left.jpg",
            "models/skybox/top.jpg",
            "models/skybox/bottom.jpg",
            "models/skybox/front.jpg",
            "models/skybox/back.jpg"
        };
        Skybox skybox(skyFaces);

        // Build circuit & GPU path mesh & lighting
        vector<BezierSegment> nbrCircuit = buildNBRCircuit();
        vector<BezierSegment> pitstopCircuit = buildNBRCircuit(true);

        auto nbrArcTable = buildArcLengthTable(nbrCircuit);
        float nbrCircuitLength =
            nbrArcTable.back().distance;

        auto pitstopArcTable = buildArcLengthTable(pitstopCircuit);
        float pitstopCircuitLength =
            pitstopArcTable.back().distance;

        vector<PointLight> sceneLights = buildSceneLights();
        PathMesh pathMesh = buildPathMesh(nbrCircuit);
        PathMesh pitstopPathMesh = buildPathMesh(pitstopCircuit);
        const int NUM_SEGMENTS = (int)nbrCircuit.size();
        const int NUM_PS_SEGMENTS = (int)pitstopCircuit.size();

        // auto animation state
        float carT     = 0.0f;
        float distanceTravelled = 0.0f;
        float distanceTravelledAbs = 0.0f;
        // float carSpeed = 0;

        // --- MRT framebuffer (scene + bright) ---
        unsigned int hdrFBO;
        glGenFramebuffers(1, &hdrFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        unsigned int colorBuffers[2];
        glGenTextures(2, colorBuffers);
        for (int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
        }
        unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);

        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // --- ping-pong FBOs voor blur ---
        unsigned int pingpongFBO[2];
        unsigned int pingpongBuffer[2];
        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongBuffer);
        for (int i = 0; i < 2; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Shader blurShader("src/shader/post.vs", "src/shader/blur.fs");

        // --- Screen quad ---
        float quadVertices[] = {
            // pos         // uv
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
        };
        unsigned int quadVAO, quadVBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        Shader postShader("src/shader/post.vs", "src/shader/post.fs");

        // ---- HUD implementation ----
        Hud hud;
        hud.setupHud(camMode);

        // Cursor
        float cursorVertices[] = {
            // pos         // uv
            -0.0025f, 0.004f, 0.0f, 1.0f,  // top left
            -0.0025f, -0.004f, 0.0f, 0.0f, // bottom left
            0.0025f, -0.004f, 1.0f, 0.0f,  // bottom right

            -0.0025f, 0.004f, 0.0f, 1.0f, // top left
            0.0025f, -0.004f, 1.0f, 0.0f, // bottom right
            0.0025f, 0.004f, 1.0f, 1.0f   // top right
        };

        unsigned int cursorVAO, cursorVBO;
        glGenVertexArrays(1, &cursorVAO);
        glGenBuffers(1, &cursorVBO);
        glBindVertexArray(cursorVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cursorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cursorVertices), cursorVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        Shader cursorShader("src/hud/cursor.vs", "src/hud/cursor.fs");

        // Pitstop icon
        float psVertices[] = {
            // pos         // uv
            0.80f, 0.98f, 0.0f, 1.0f, // top left
            0.80f, 0.78f, 0.0f, 0.0f, // bottom left
            0.98f, 0.78f, 1.0f, 0.0f, // bottom right

            0.80f, 0.98f, 0.0f, 1.0f, // top left
            0.98f, 0.78f, 1.0f, 0.0f, // bottom right
            0.98f, 0.98f, 1.0f, 1.0f  // top right
        };

        unsigned int psVAO, psVBO;
        glGenVertexArrays(1, &psVAO);
        glGenBuffers(1, &psVBO);
        glBindVertexArray(psVAO);
        glBindBuffer(GL_ARRAY_BUFFER, psVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(psVertices), psVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        // texture binding
        unsigned int pitstopIconId;
        glGenTextures(1, &pitstopIconId);
        glBindTexture(GL_TEXTURE_2D, pitstopIconId);

        int width, height, nrChannels;
        unsigned char *pitstopIconData = stbi_load("models/pitstop icon/pitstop-icon.jpg", &width, &height, &nrChannels, 0);
        if (pitstopIconData)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pitstopIconData);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(pitstopIconData);
        }
        else
        {
            std::cout << "Pitstop icon texture failed to load: " << std::endl;
            stbi_image_free(pitstopIconData);
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // ---- chroma keying shader ----
        Shader chromaKeyShader("src/shader/chromaKey.vs", "src/shader/chromaKey.fs");

        unsigned int CKTexture;
        glGenTextures(1, &CKTexture);
        glBindTexture(GL_TEXTURE_2D, CKTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        
        unsigned char *CKdata = stbi_load("models/chroma_keying/ChromaOverlay.png", &width, &height, &nrChannels, 0);
        if (CKdata)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, CKdata);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(CKdata);
        }
        else
        {
            std::cout << "Chroma key texture failed to load: " << std::endl;
            stbi_image_free(CKdata);
        }

        // --------------------------------------------------------------
        //  RENDER LOOP
        // --------------------------------------------------------------
        while (!glfwWindowShouldClose(window))
        {
            float currentFrame = (float)glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // ---- input ----
            processInput(window);
            glfwPollEvents();
            
            // ---- auto vooruit doen ---- (en een beetje interactie)

            float carSpeed = CAR_SPEED * 0.6f;

            if (legacyDriving)
            {
                carSpeed = 1.2f * 0.6f;
            }

            switch (speedIncrease)
            {
            case 's': // snel
                carSpeed *= 3;
                break;
            case 't': // traag
                carSpeed *= 0.1; 
                break;
            case 'r': // rem
                carSpeed = 0;
                break;
            case 'a': // achteruit
                carSpeed *= -0.8;
                break;
            default:
                break;
            }

            carT += carSpeed * deltaTime; // legacy code for past (and future) implementations
            if (carT >= (float)NUM_SEGMENTS)
                carT -= (float)NUM_SEGMENTS;
            distanceTravelled += carSpeed * deltaTime;
            distanceTravelledAbs += glm::abs(carSpeed * deltaTime);

            // ---- auto positie en draai ----
            float currentTrackLength = pitstop ? pitstopCircuitLength : nbrCircuitLength;
            auto& activeArcTable     = pitstop ? pitstopArcTable      : nbrArcTable;

            while (distanceTravelled < 0.0f)
                distanceTravelled += currentTrackLength;
            while (distanceTravelled >= currentTrackLength)
                distanceTravelled -= currentTrackLength;

            float t = calculateDistanceAlongTrack(activeArcTable, distanceTravelled);

            if (legacyDriving)
            {
                t = carT;
                carSpeed = 1.2f * 0.6f;
            }
            

            auto& activeCircuit = pitstop ? pitstopCircuit : nbrCircuit;

            float     curvature   = sampleCurvature(activeCircuit, t);
            float     steeringAngleDeg = glm::clamp(-curvature * 1200.0f, -90.0f, 90.0f);
            glm::vec3 carAfgeleide = sampleCircuitAfgeleide(activeCircuit, t);
            glm::vec3 carPos       = sampleCircuit(activeCircuit, t);

            if (pitstop &&
                carPos.x < -110 && carPos.x > -130 &&
                carPos.z <  113 && carPos.z >   93)
            {
                carSpeed *= 0.1f;
            }
            
            glm::vec3 up     = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(carAfgeleide, up));
            glm::vec3 realUp = glm::cross(right, carAfgeleide);

            glm::mat4 rotMat(1.0f);
            rotMat[0] = glm::vec4(right,      0.0f);
            rotMat[1] = glm::vec4(realUp, 0.0f);
            rotMat[2] = glm::vec4(carAfgeleide, 0.0f);

            updateCamera(camera, camMode, carPos, carAfgeleide, realUp, right, currentFrame, carSpeed);

            // Debug functie die de camera positie print
            // glm::vec3 camPos = camera.Position;
            // std::cout << "X: " << camPos.x << "  |  Y: " << camPos.y << "  |  Z: " << camPos.z << "\n";

            // ---- PASS 1: render scene naar hdrFBO ----
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 5000.0f);

            myShader.use();

            // ---- lighting ----
            applyLights(myShader, sceneLights);
            myShader.setMat4("view",       view);
            myShader.setMat4("projection", projection);

            // ---- Draw track ----
            glm::mat4 trackModel = glm::mat4(1.0f);
            trackModel = glm::translate(trackModel, circuitPos);
            myShader.setMat4("model", trackModel);
            myTrack.Draw(myShader);

            // ---- Draw auto ----
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, carPos);
            model *= rotMat;
            model = glm::scale(model, glm::vec3(carSize, carSize, carSize));
            myShader.setMat4("model", model);
            myModel.DrawCar(myShader, steeringAngleDeg, distanceTravelled);

            // ---- Draw tire stack ----
            glm::mat4 tireStackModel = glm::mat4(1.0f);
            tireStackModel = glm::translate(tireStackModel, tireStackPos);
            myShader.setMat4("model", tireStackModel);
            myTires.Draw(myShader);

            // ---- Draw debug path ----
            if (!hideBC)
            {
                myShader.setMat4("model", glm::mat4(1.0f));
                glBindVertexArray(pitstopPathMesh.VAO);
                glDrawArrays(GL_LINE_STRIP, 0, pitstopPathMesh.vertCount);
                glBindVertexArray(0);
                glBindVertexArray(pathMesh.VAO);
                glDrawArrays(GL_LINE_STRIP, 0, pathMesh.vertCount);
                glBindVertexArray(0);
            }
            skybox.Draw(view, projection);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ---- PASS 2: blur op bright buffer ----
            bool horizontal = true;
            blurShader.use();
            blurShader.setInt("image", 0);
            glActiveTexture(GL_TEXTURE0);
            for (int i = 0; i < 10; i++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                blurShader.setBool("horizontal", horizontal);
                glBindTexture(GL_TEXTURE_2D, i == 0 ? colorBuffers[1] : pingpongBuffer[!horizontal]);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                horizontal = !horizontal;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ---- PASS 3: post processing + bloom ----
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            postShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
            postShader.setInt("screenTexture", 0);
            postShader.setInt("bloomTexture",  1);
            postShader.setInt("bloomEnabled",  1);
            postShader.setInt("camMode",       (int)camMode);
            postShader.setVec2("resolution",   glm::vec2(screenWidth, screenHeight));
            postShader.setInt("filterBlur",    filterBlur);
            postShader.setInt("filterSharpen", filterSharpen);
            postShader.setInt("filterScanline",filterScanline);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

             // ---- PASS 4: HUD Element & Chroma Keying ----
            // chroma key
            if (chromaKeyActive)
            {
                glDisable(GL_DEPTH_TEST);
                chromaKeyShader.use();

                chromaKeyShader.setInt("CKTexture", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, CKTexture);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glEnable(GL_DEPTH_TEST);
            }

            if (!hideHud)
            {
                glDisable(GL_DEPTH_TEST);
                hud.Draw(camMode);

                if (pitstop)
                {
                    hud.shader->use();

                    hud.shader->setInt("hudTexture", 0);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, pitstopIconId);
                    glBindVertexArray(psVAO);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }                

                if (camMode == CAM_FREE)
                {
                    cursorShader.use();
                    glBindVertexArray(cursorVAO);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }

                glEnable(GL_DEPTH_TEST);
            }

            glfwSwapBuffers(window);
        }

        // ---- Cleanup GPU resources ----
        glDeleteVertexArrays(1, &pathMesh.VAO);
        glDeleteBuffers(1, &pathMesh.VBO);
        glDeleteVertexArrays(1, &pitstopPathMesh.VAO);
        glDeleteBuffers(1, &pitstopPathMesh.VBO);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteFramebuffers(1, &hdrFBO);
        glDeleteFramebuffers(2, pingpongFBO);
        glDeleteTextures(2, colorBuffers);
        glDeleteTextures(2, pingpongBuffer);
        glDeleteRenderbuffers(1, &rboDepth);

    }

    glfwTerminate();
    return 0;
}