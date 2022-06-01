#version 330 core

uniform vec3 camPosition; // so we can compute the view vector

// light uniforms
uniform vec4 LightPosition;
uniform vec3 LightIntensity;

//material properties
uniform vec3 ambientReflectance; //How much ambient light the object reflects.
uniform vec3 diffuseReflectance; //How much diffuse light the object reflects.
uniform vec3 specularReflectance; //How much specular light the object reflects.
uniform float specularExponent; //How concentrated the spotlight is, the higher the value, the smoother is the surface of the material.

// variables from vertex shader
in vec4 Position;
in vec3 Normal;

// output color of this fragment
layout ( location = 0 ) out vec4 FragColor;


void main()
{
	//Phong Shading
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(LightPosition.xyz - Position.xyz);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(-Position.xyz);
	
	// calculate diffuse and specular lighting
	float diffuseFactor = max(dot(lightDir, normal), 0.0);
	float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), specularExponent);
	
	// calculate final color
	vec3 diffuseColor = diffuseFactor * diffuseReflectance;
	vec3 specularColor = specularFactor * specularReflectance;
	vec3 finalColor = (ambientReflectance + diffuseColor + specularColor) * LightIntensity;
	
	//FragColor = vec4(0.0f,0.0f,1.0f, 1.0);
	FragColor = vec4(finalColor, 1.0);
	
}
