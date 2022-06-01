#version 400 core

in float Transp;

uniform sampler2D ParticleTexture;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(ParticleTexture, gl_PointCoord);
	FragColor.a *= Transp;
}