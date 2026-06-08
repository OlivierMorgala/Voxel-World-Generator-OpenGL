#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 localPosition;

out vec3 ourColor;
out vec2 TexCoord;
out vec3 FragPosition;

uniform mat4 model;      // Gdzie jest obiekt
uniform mat4 view;       // Gdzie jest kamera
uniform mat4 projection; // Perspektywa

void main() {
FragPosition = vec3(model * vec4(pos, 1.0)); 

    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    ourColor = color;
    TexCoord = localPosition;
}