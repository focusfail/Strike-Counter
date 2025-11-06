#version 460

uniform layout (location = 2) sampler2DArray texture_array;
uniform layout (location = 3) uint debug; 

in vec2 texcoord;
in vec2 texture_size;
flat in uint layer; 
out vec4 fragColor;

void main() {
    vec2 uv = fract(texcoord);
    vec2 texel_size = vec2(1.0) / 256.0;
    vec2 scale = texture_size / 256.0;
    uv = uv * (scale - texel_size) + texel_size * 0.5;
    
    vec4 color = texture(texture_array, vec3(uv, layer)); 

    if (color == vec4(0.0, 0.0, 1.0, 1.0)) {
        color.w = 0.0;
    }
    
    if (debug == 1) { // draw only uv 
        fragColor = vec4(fract(texcoord), 0.0, 1.0);
    } else if (debug == 2){ // draw texture and uv for black
        if (color == vec4(0.0)) {
            color = vec4(fract(texcoord), 0.0, 1.0);
        }
        fragColor = color;
    } else {
        fragColor = color;
    }
}
