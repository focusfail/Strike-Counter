#version 460

out vec2 texcoord;
out vec2 texture_size;
flat out uint layer; 

uniform layout (location = 0) mat4 view;
uniform layout (location = 1) mat4 proj;

layout (binding = 0, std430) readonly buffer ssbo0 {
    vec4 positions[];
};

struct Face {
    vec4 s;
    vec4 t;
    vec2 tex_size;
    uint tex_layer;
    uint side;
};

layout (binding = 1, std430) readonly buffer ssbo1 {
    Face faces[];
};

void main() {
    uint face_id = gl_DrawID; 
    vec3 world_pos = positions[gl_VertexID].xyz;

    Face face = faces[face_id];
    texcoord.x = (dot(face.s.xyz, world_pos) + face.s.w);
    texcoord.y = (dot(face.t.xyz, world_pos) + face.t.w);

    if (face.side != 1) {
        texcoord.x = -texcoord.x;
    } 
    
    texcoord /= face.tex_size;
    layer  = face.tex_layer;

    //? flipping x makes maps and textures render correctly?
    world_pos.x = -world_pos.x;
    texture_size = face.tex_size;
    gl_Position = proj * view * vec4(world_pos.xzy, 1.0);
}
