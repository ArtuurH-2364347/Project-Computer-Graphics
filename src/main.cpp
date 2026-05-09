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
bool filterBlur     = false; // G
bool filterVignette = false; // H
bool filterSharpen  = false; // F
bool filterScanline = false; // J

float  deltaTime = 0.0f;
float  lastFrame = 0.0f;

// Screen quad vertices (positie + texcoords)
float quadVertices[] = {
    // pos        // texcoords
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f
};

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    // ------------------------------------------------------------------
    //  POST-PROCESSING SHADER
    // ------------------------------------------------------------------
    Shader postShader("src/shader/post.vs", "src/shader/post.fs");

    // Screen quad VAO
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

    // ------------------------------------------------------------------
    //  FBO aanmaken
    // ------------------------------------------------------------------
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Color texture
    unsigned int colorTexture;
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0,
                GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, colorTexture, 0);

    // Renderbuffer voor depth + stencil
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: Framebuffer is niet compleet!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
        Shader myShader("src/shader/shader.vs", "src/shader/shader.fs");
        Model  myModel(filesystem::path("models/2021_F1_Mercedes-Benz_W12/2021_F1_Mercedes-Benz_W12.obj"));

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
        vector<BezierSegment> spaCircuit = buildSpaCircuit();
        vector<PointLight> sceneLights = buildSceneLights();
        PathMesh pathMesh = buildPathMesh(spaCircuit);
        const int NUM_SEGMENTS = (int)spaCircuit.size();

        // auto animation state
        float carT     = 0.0f;
        float carSpeed = CAR_SPEED;
        float distanceTraveled = 0.0f;

        // --------------------------------------------------------------
        //  RENDER LOOP
        // --------------------------------------------------------------
        while (!glfwWindowShouldClose(window))
            {
                float currentFrame = (float)glfwGetTime();
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;

                processInput(window);
                glfwPollEvents();

                carT += carSpeed * deltaTime;
                if (carT >= (float)NUM_SEGMENTS)
                    carT -= (float)NUM_SEGMENTS;
                distanceTraveled += carSpeed * deltaTime;

                glm::vec3 carPos     = sampleCircuit(spaCircuit, carT);
                glm::vec3 carTangent = sampleCircuitAfgeleide(spaCircuit, carT);
                glm::vec3 up         = glm::vec3(0.0f, 1.0f, 0.0f);
                glm::vec3 right      = glm::normalize(glm::cross(up, carTangent));
                glm::vec3 realUp     = glm::cross(carTangent, right);

                glm::mat4 rotMat(1.0f);
                rotMat[0] = glm::vec4(right,      0.0f);
                rotMat[1] = glm::vec4(realUp,     0.0f);
                rotMat[2] = glm::vec4(carTangent, 0.0f);

                float curvature    = sampleCurvature(spaCircuit, carT);
                const float MAX_CURVATURE     = 0.08f;
                const float MAX_STEER_DEGREES = 180.0f;
                float steeringAngle = glm::clamp(curvature / MAX_CURVATURE, -1.0f, 1.0f)
                                    * MAX_STEER_DEGREES;

                if (camMode == CAM_FOLLOW)
                {
                    glm::vec3 camPos    = carPos - carTangent * CAM_DISTANCE
                                                + glm::vec3(0.0f, CAM_HEIGHT, 0.0f);
                    glm::vec3 camTarget = carPos + carTangent * CAM_DISTANCE;
                    camera.SetLookAt(camPos, camTarget, up);
                }
                else if (camMode == CAM_FIRST_PERSON)
                {
                    camera.SetFirstPersonShake(carPos, carTangent, realUp, right,
                                    (float)glfwGetTime(), carSpeed);
                }

                // ================================================================
                //  PASS 1: render scene naar FBO
                // ================================================================
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                glEnable(GL_DEPTH_TEST);
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                myShader.use();
                myShader.setVec3("viewPos", camera.Position);
                for (int i = 0; i < (int)sceneLights.size(); i++) {
                    string base = "pointLights[" + to_string(i) + "].";
                    myShader.setVec3 (base + "position",  sceneLights[i].position);
                    myShader.setVec3 (base + "color",     sceneLights[i].color);
                    myShader.setFloat(base + "intensity", sceneLights[i].intensity);
                    myShader.setFloat(base + "constant",  sceneLights[i].constant);
                    myShader.setFloat(base + "linear",    sceneLights[i].linear);
                    myShader.setFloat(base + "quadratic", sceneLights[i].quadratic);
                }

                glm::mat4 view = camera.GetViewMatrix();
                glm::mat4 projection = glm::perspective(
                    glm::radians(45.0f),
                    (float)screenWidth / (float)screenHeight,
                    0.1f, 500.0f
                );
                myShader.setMat4("view",       view);
                myShader.setMat4("projection", projection);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, carPos);
                model = model * rotMat;
                model = glm::scale(model, glm::vec3(carSize, carSize, carSize));
                myShader.setMat4("model", model);
                myModel.DrawCar(myShader, steeringAngle, distanceTraveled);

                myShader.setMat4("model", glm::mat4(1.0f));
                glBindVertexArray(pathMesh.VAO);
                glDrawArrays(GL_LINE_STRIP, 0, pathMesh.vertCount);
                glBindVertexArray(0);

                skybox.Draw(view, projection);

                // ================================================================
                //  PASS 2: render screen quad met post-processing
                // ================================================================
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDisable(GL_DEPTH_TEST);
                glClear(GL_COLOR_BUFFER_BIT);

                postShader.use();
                postShader.setInt("screenTexture", 0);
                postShader.setInt ("camMode",          (int)camMode);
                postShader.setInt ("filterBlur",       filterBlur     ? 1 : 0);
                postShader.setInt ("filterVignette",   filterVignette ? 1 : 0);
                postShader.setInt ("filterSharpen",    filterSharpen  ? 1 : 0);
                postShader.setInt ("filterScanline",   filterScanline ? 1 : 0);
                postShader.setVec2("resolution",
                    glm::vec2((float)screenWidth, (float)screenHeight));

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, colorTexture);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glfwSwapBuffers(window);
            }

            // Cleanup
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(1, &colorTexture);
            glDeleteRenderbuffers(1, &rbo);
            glDeleteVertexArrays(1, &quadVAO);
            glDeleteBuffers(1, &quadVBO);

        // ---- Cleanup GPU resources ----
        glDeleteVertexArrays(1, &pathMesh.VAO);
        glDeleteBuffers(1, &pathMesh.VBO);

    }

    glfwTerminate();
    return 0;
}