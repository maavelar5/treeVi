#version 330 core

in vec2  tex_coords;
out vec4 color;

uniform vec3      u_color;
uniform vec4      u_offset;
uniform sampler2D u_image;

uniform bool  u_type = true;
uniform float u_alpha;

void main ()
{
    vec2 t = tex_coords;

    if (u_type == true)
        color = texture (u_image, t * u_offset.zw + u_offset.xy) * u_alpha;
    else color = vec4 (1, 1, 0, 1);
}
