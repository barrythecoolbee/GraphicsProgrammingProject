#version 330 core
layout (location = 0) in vec3 VertexPosition;
//layout (location = 1) in vec3 VertexNormal;

uniform float Time; //time of animation

//Wave parameters (Mathematic equation)
uniform float K = 25; //Wavenumber -> replaces 2*pi/lamba
uniform float A = 06; //Amplitude
uniform float V = 25; //Velocity

uniform mat4 ModelViewMatrix; // represents model coordinates in the world coord space (*model*)
//uniform mat3 NormalMatrix;
uniform mat4 MVP; //represents the view and projection matrices combined (*viewProjection*)

out vec4 Position;
//out vec3 Normal;

//uniform int hasNormal;

void main(){

	vec4 pos = vec4(VertexPosition, 1.0);

	//Wave equation -> y-coordinate of the surface
	pos.y = A * sin(K * (pos.x - V * Time));

	//Normal vector
	vec3 n = vec3(0.0);
	n.xy = normalize(vec2( -A * K * cos(K * (pos.x - V * Time)), 1.0));

	//Send Position and Normal (in camera coords) to fragment shader
	Position = ModelViewMatrix * pos;
	//if(hasNormal == 1)
		//Normal = NormalMatrix * n;

	//The position in clip coordinates
	gl_Position = MVP * pos;
}

/*
Takes the position of the vertex and updates the y-coordinate using the wave equation.
After the first three statements, the variable pos is just a copy of the input variable *vertex* with the modified y-coordinate
We compute the normal vector using the (partial) derivate of the previous equation, normalize it and store in the variable *n*.
Since the wave is just two-dimentional, the z component of the normal will be zero.
We pass along the new *Position* and *Normal* to the fragment shader after converting to camera coordinates. 
We also pass the position in clip coordinates to the built-in variable *gl_Position* (INVESTIGATE)
*/