#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in float StartTime;

out float Transp; //Transparency of the particle

uniform float Time; //Animation time
uniform vec3 Gravity = vec3(0.0, -0.05, 0.0); //Gravity - world coords
uniform float ParticleLifetime; //Max particle lifetime

uniform mat4 MVP; //Model-view-projection matrix

void main(){

	//Assume the initial position is the origin (0,0,0)
	vec3 pos = vec3(0.0);
	Transp = 0.0;

	//Particle doesn't exist until the start Time
	if(Time > StartTime){
		float t = Time - StartTime; //Time since start
		
		if(t < ParticleLifetime){
		//Particle is alive
			
			//Calculate the position of the particle
			pos = vertex * t + Gravity * t * t;
			
			//Calculate the transparency of the particle
			Transp = 1.0 - t / ParticleLifetime;
		}
	}
	
	//Transform the position to clip space
	gl_Position = MVP * vec4(pos, 1.0);

}


