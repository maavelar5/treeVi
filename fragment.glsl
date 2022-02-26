#version 330 core

in vec2  tex_coords;
out vec4 color;

uniform vec3      u_color;
uniform vec4      u_offset;
uniform sampler2D u_image;

uniform int   u_type = 1;
uniform float u_alpha;

void main ()
{
    vec2 t = tex_coords;

    if (u_type == 1)
        color = texture (u_image, t * u_offset.zw + u_offset.xy) * u_alpha;
    else if (u_type == 2) color = texture (u_image, t) * u_alpha;
    // else color = vec4 (0, .4, 1, u_alpha);
    else color = vec4 (u_color, 1.f);
}
