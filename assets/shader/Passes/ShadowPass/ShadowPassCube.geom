#version 430 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

out vec4 gFragPos;

layout(std140) uniform ShadowCube {
    mat4 uShadowVPs[6];
    float uFarPlane;
};

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            gFragPos = gl_in[i].gl_Position;
            gl_Position = uShadowVPs[face] * gFragPos; // specify depth
            EmitVertex();
        }
        EndPrimitive();
    }
}