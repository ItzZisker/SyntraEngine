#version 400 core

#define NR_CASCADES ${NR_CASCADES=2}

layout(triangles, invocations = 3) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 lightSpaceMatrices[NR_CASCADES + 1];

void main() {
	for (int i = 0; i < 3; ++i) {
		gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}  
