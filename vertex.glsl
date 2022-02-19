#version 330 core

layout (location = 0) in vec2 position;

out vec2 tex_coords;

uniform mat4 u_model;
uniform mat4 u_projection;

void main ()
{
    tex_coords = position;

    gl_Position = u_projection * u_model * vec4 (position, 1.0, 1.0);
}
