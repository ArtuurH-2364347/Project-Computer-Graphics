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

float  deltaTime = 0.0f;
float  lastFrame = 0.0f;
char speedIncrease = 'n'; // nothing

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

    {
        // Model and Shader declaration
        Shader myShader("src/shader/shader.vs", "src/shader/shader.fs");
        Model  myModel(filesystem::path("models/2021_F1_Mercedes-Benz_W12/2021_F1_Mercedes-Benz_W12.obj")); // => obj file
        // Model myModel(filesystem::path("models/2021_f1_mercedes-benz_w12_gLTF/scene.gltf")); // => gltf file

        Model myTrack(filesystem::path("models/nurburgring_race_driver_grid_ds_gltf/scene.gltf"));

        // ---- track aanpassingen ----
        glm::vec3 circuitPos = glm::vec3(-420.0f, -20.0f, 390.0f);
        
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
        vector<PointLight> sceneLights = buildSceneLights();
        PathMesh pathMesh = buildPathMesh(nbrCircuit);
        const int NUM_SEGMENTS = (int)nbrCircuit.size();

        // auto animation state
        
        float carT     = 0.0f;
        
        //float carSpeed = 0;

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
            float carSpeed = CAR_SPEED * 0.7;

            switch (speedIncrease)
            {
            case 's': // snel
                carSpeed *= 1.5;
                break;
            case 't': // traag
                carSpeed *= 0.1; 
                break;
            case 'r': // rem
                carSpeed *= 0;
                break;
            case 'a': // achteruit
                carSpeed *= -0.3;
                break;
            default:
                break;
            }

            carT += carSpeed * deltaTime;
            if (carT >= (float)NUM_SEGMENTS)
                carT -= (float)NUM_SEGMENTS;

            // ---- auto positie en draai ----
            glm::vec3 carPos     = sampleCircuit(nbrCircuit, carT);
            glm::vec3 carAfgeleide = sampleCircuitAfgeleide(nbrCircuit, carT);

            glm::vec3 up     = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(carAfgeleide, up));
            glm::vec3 realUp = glm::cross(right, carAfgeleide);

            glm::mat4 rotMat(1.0f);
            rotMat[0] = glm::vec4(right,      0.0f);
            rotMat[1] = glm::vec4(realUp, 0.0f);
            rotMat[2] = glm::vec4(carAfgeleide, 0.0f);

            if (camMode == CAM_FOLLOW)
            {
                glm::vec3 camPos    = carPos - carAfgeleide * CAM_DISTANCE
                                            + glm::vec3(0.0f, CAM_HEIGHT, 0.0f);
                glm::vec3 camTarget = carPos + carAfgeleide * CAM_DISTANCE;
                camera.SetLookAt(camPos, camTarget, up);
            }
            else if (camMode == CAM_FIRST_PERSON)
            {
                camera.SetFirstPerson(carPos, carAfgeleide, realUp, right);
            }

            //glm::vec3 camPos = camera.Position;
            //std::cout << "X: " << camPos.x << "  |  Y: " << camPos.y << "  |  Z: " << camPos.z << "\n";

            // ---- clear ----
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            myShader.use();

            // ---- lighting ----
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

            // ---- matrices ----
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                (float)screenWidth / (float)screenHeight,
                0.1f, 500.0f
            );

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
            myModel.Draw(myShader);

            // ---- Draw debug path ----
            myShader.setMat4("model", glm::mat4(1.0f));
            glBindVertexArray(pathMesh.VAO);
            glDrawArrays(GL_LINE_STRIP, 0, pathMesh.vertCount);
            glBindVertexArray(0);

            skybox.Draw(view, projection);

            glfwSwapBuffers(window);
        }

        // ---- Cleanup GPU resources ----
        glDeleteVertexArrays(1, &pathMesh.VAO);
        glDeleteBuffers(1, &pathMesh.VBO);

    }

    glfwTerminate();
    return 0;
}