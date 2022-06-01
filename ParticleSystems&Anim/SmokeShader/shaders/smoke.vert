#version 440 core
subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexVelocity;
layout (location = 2) in float VertexStartTime;
layout (location = 3) in vec3 VertexInitialVelocity;

out float Transp; //Transparency of the particle
layout( xfb_buffer = 0, xfb_offset=0 ) out vec3 Position; //Position of the particle to tranform feedback
layout( xfb_buffer = 1, xfb_offset=0 ) out vec3 Velocity; //Velocity of the particle to tranform feedback
layout( xfb_buffer = 2, xfb_offset=0 ) out float StartTime; //Start time of the particle to tranform feedback

uniform float Time; //Animation time
uniform float H; //Elapsed time between frames
uniform vec3 Accel; //Particle acceleration
uniform float ParticleLifetime; //Max particle lifetime

uniform float MinParticleSize;
uniform float MaxParticleSize;

uniform mat4 MVP; //Model-view-projection matrix

subroutine (RenderPassType)
void update(){

	Position = VertexPosition;
	Velocity = VertexVelocity;
	StartTime = VertexStartTime;

	//Particle doesn't exist until the start Time
	if(Time >= StartTime){
		float t = Time - StartTime; //Time since start (age)
		
		if(t > ParticleLifetime){
			//Particle is dead, recycle
			Position = vec3(0.0);
			Velocity = VertexInitialVelocity;
			StartTime = Time;
		} else {
			//Particle is alive
			Position += Velocity * H;
			Velocity += Accel * H;
		}
	}
}

subroutine(RenderPassType)
void render(){
	float age = Time - VertexStartTime;
	Transp = 0.0;
	
	if(Time >= VertexStartTime){
		float agePct = age / ParticleLifetime;
		Transp = 1.0 - agePct;
		gl_PointSize = mix(MinParticleSize, MaxParticleSize, agePct);
	}
	
	gl_Position = MVP * vec4(VertexPosition, 1.0);
}

void main(){
	RenderPass();
}


