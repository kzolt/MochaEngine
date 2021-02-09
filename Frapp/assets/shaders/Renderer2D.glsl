#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VectexOutput
{
	vec4 Color;
};

layout(location = 0) out VectexOutput Output;

void main()
{
	Output.Color = a_Color;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

struct VectexOutput
{
	vec4 Color;
};

layout(location = 0) in VectexOutput Input;

void main()
{
	color = Input.Color;
}