#version 330 core

#define NR_CASCADES ${NR_CASCADES=4}

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 lightSpaceMatrices[NR_CASCADES];

void main() {
	for (int i = 0; i < 3; ++i) {
		gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}  
