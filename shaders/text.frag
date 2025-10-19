#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;
uniform int isColorTexture;

void main()
{
    vec4 sampled = texture(text, TexCoords);
    if (isColorTexture == 1) {
        // Color texture already contains alpha premultiplied if provided by font bitmap
        FragColor = sampled;
    } else {
        // Regular text: sampled.r holds the mask
        FragColor = vec4(textColor * sampled.r, sampled.r);
    }
}
