#version 430 core

out vec4 vFragDNCPos;

void main()
{
//     // custom calc is needed
//     // opengl -z is front
//     float lightDistance = ((vFragDNCPos.xyz / vFragDNCPos.w).z + 1.0f) / 2.f;
    
//     // map to [0;1] range by dividing by far_plane
//     // lightDistance = lightDistance / uFarPlane;
    
//     // write this as modified depth
//     gl_FragDepth = lightDistance;
}
