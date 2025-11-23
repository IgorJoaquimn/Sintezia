#version 330 core
layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

uniform mat4 uWorldTransform;
uniform mat4 uProjection;
uniform vec2 texOffset;
uniform vec2 texScale;

void main()
{
    TexCoords = vertex.zw * texScale + texOffset;
    gl_Position = uProjection * uWorldTransform * vec4(vertex.xy, 0.0, 1.0);
}
