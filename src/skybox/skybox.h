#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image/stb_image.h>
#include <../src/shader/shader.h>
#include <vector>
#include <string>
#include <iostream>

class Skybox
{
public:
    Skybox(const std::vector<std::string>& faces)
    {
        setupMesh();
        textureID = loadCubemap(faces);
        shader = new Shader("src/skybox/skybox.vs", "src/skybox/skybox.fs");
        shader->use();
        shader->setInt("skybox", 0);
    }

    ~Skybox()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteTextures(1, &textureID);
        delete shader;
    }

    void Draw(const glm::mat4& view, const glm::mat4& projection)
    {
        glm::mat4 skyView = glm::mat4(glm::mat3(view));

        glDepthFunc(GL_LEQUAL);
        shader->use();
        shader->setMat4("view",       skyView);
        shader->setMat4("projection", projection);

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

private:
    unsigned int VAO, VBO, textureID;
    Shader* shader;

    void setupMesh()
    {
        float vertices[] = {
            -1, -1,  1,   1, -1,  1,   1,  1,  1,   1,  1,  1,  -1,  1,  1,  -1, -1,  1,
            -1, -1, -1,  -1,  1, -1,   1,  1, -1,   1,  1, -1,   1, -1, -1,  -1, -1, -1,
            -1,  1, -1,  -1,  1,  1,   1,  1,  1,   1,  1,  1,   1,  1, -1,  -1,  1, -1,
            -1, -1, -1,   1, -1, -1,   1, -1,  1,   1, -1,  1,  -1, -1,  1,  -1, -1, -1,
             1, -1, -1,   1,  1, -1,   1,  1,  1,   1,  1,  1,   1, -1,  1,   1, -1, -1,
            -1, -1, -1,  -1, -1,  1,  -1,  1,  1,  -1,  1,  1,  -1,  1, -1,  -1, -1, -1,
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    unsigned int loadCubemap(const std::vector<std::string>& faces)
    {
        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        stbi_set_flip_vertically_on_load(false);

        int w, h, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++) {
            unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                             w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                std::cout << "Cubemap failed to load: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_set_flip_vertically_on_load(true);
        return id;
    }
};