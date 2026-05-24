#version 330 core

// Ouput
out vec4 color;

// Values that stay constant for the whole mesh.
uniform vec4 PickingColor;

// returns the color of the picked mesh
void main(){

    color = PickingColor;

}