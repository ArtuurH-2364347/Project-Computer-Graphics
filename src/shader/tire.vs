#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 vertexPosition;

void main(){

    // Return position of the vertex: vertexPosition * position
    gl_Position =  vertexPosition * vec4(vertexPosition_modelspace,1);
}