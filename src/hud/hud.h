#ifndef HUD_H
#define HUD_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stb_image/stb_image.h>

#include <../src/shader/shader.h>
#include <../src/cammode.h>

class Hud
{
private:
    unsigned int hudVAO, hudVBO;
    
    int width, height, nrChannels;
    unsigned int dataIdFree;
    unsigned int dataIdTP;
    unsigned int dataIdFP;

public :

    Shader *shader;

    Hud()
    {
        shader = new Shader("src/hud/hud.vs", "src/hud/hud.fs");

        dataIdFP = loadTexture("models/hud/FPCamHud.png");
        dataIdTP = loadTexture("models/hud/TPCamHud.png");
        dataIdFree = loadTexture("models/hud/FreeCamHud.png");
    }

    ~Hud()
    {
        glDeleteVertexArrays(1, &hudVAO);
        glDeleteBuffers(1, &hudVBO);
        delete shader;
        glDeleteTextures(1, &dataIdFP);
        glDeleteTextures(1, &dataIdTP);
        glDeleteTextures(1, &dataIdFree);
    }

    unsigned int loadTexture(const char *path)
    {
        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Hud texture failed to load: " << path << std::endl;
            stbi_image_free(data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // check

        return id;
    }

    void setupHud(CamMode camMode)
    {
        float hudQuadVertices[] = {
            // pos         // uv
            -0.98f, 0.98f, 0.0f, 1.0f, // top left
            -0.98f, 0.0f, 0.0f, 0.0f,  // bottom left
            -0.45f, 0.0f, 1.0f, 0.0f,  // bottom right

            -0.98f, 0.98f, 0.0f, 1.0f, // top left
            -0.45f, 0.0f, 1.0f, 0.0f,  // bottom right
            -0.45f, 0.98f, 1.0f, 1.0f  // top right
        };

        //  --- Bindings ---
        glGenVertexArrays(1, &hudVAO);
        glGenBuffers(1, &hudVBO);
        glBindVertexArray(hudVAO);
        glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(hudQuadVertices), hudQuadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

public:
    void Draw(CamMode camMode)
    {
        //  --- Load Texture ---
        unsigned int data;

        switch (camMode)
        {
        case CAM_FIRST_PERSON:
            data = dataIdFP;
            break;
        case CAM_FREE:
            data = dataIdFree;
            break;
        default: // CAM_FOLLOW + CAM_CINEMATIC
            data = dataIdTP;
            break;
        }

        //  --- Draw The HUD ---
        shader->use();

        shader->setInt("hudTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, data);
        glBindVertexArray(hudVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

#endif