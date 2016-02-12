/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of Generic Shooter Game.
 *
 * Generic Shooter Game is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Generic Shooter Game is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Generic Shooter Game. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#version 430

#define ALOT 340282356779733799999999999999999999999.999999999999999


/// THIS SHADER MAY OR MAY NOT BE COMPLETELY BROKEN
// -- FRAGMENT INPUT DATA --
smooth in vec3 fragPos_world;
smooth in vec2 fragTexCoords;
smooth in mat3 tangentToWorldMat3;

// -- TEXTURE SAMPLERS --
#map DIFFUSE_TEXTURE_SAMPLER
uniform sampler2D diffuseSampler;
#map SPECULAR_TEXTURE_SAMPLER
uniform sampler2D specularSampler;
#map NORMAL_TEXTURE_SAMPLER
uniform sampler2D normalSampler;

// -- UNIFORM DATA --
#map GAUSSIAN_SPECULAR_TERM
uniform float mat_gaussianSpecularTerm;
#map CAMERA_POSITION_WORLD
uniform vec3 camPos_world;

// -- LIGHTING DATA --
const uint MAX_POINT_LIGHTS = 20u;
#map ACTIVE_POINT_LIGHTS
uniform uint ACTIVE_POINT_LIGHTS;
struct PointLight {
	vec3 pos_world;	
	vec3 color;
	vec3 atten;
};

const uint MAX_SPOT_LIGHTS = 20u;
#map ACTIVE_SPOT_LIGHTS
uniform uint ACTIVE_SPOT_LIGHTS;
struct SpotLight {
	vec3 pos_world;
	vec3 dir_world;
	vec3 color;
	vec3 atten;
	float angle;
};

#map LIGHT_DATA
layout(std140) uniform LightingData {
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
	vec3 directionalLight_dir_world;
	vec3 directionalLight_color;
	vec3 ambientLight_color;
};

// -- FRAGMENT OUTPUT COLOR --
out vec4 fragColor;

vec3 totalDiffuseLight = vec3(0.0, 0.0, 0.0);
vec3 totalSpecularLight = vec3(0.0, 0.0, 0.0);

void addLight(vec3 lightIntensity, vec3 fragToLightDir, vec3 fragNormal, vec3 fragToCamDir, float gaussianSpecularShininess) {
	// cosine of the angle between the FRAG_NORMAL and the FRAG_TO_LIGHT_DIR
	float cosAngIncidence = dot(fragNormal, fragToLightDir);
	cosAngIncidence = clamp(cosAngIncidence, 0.0, 1.0);

	// calculate the Gaussian specular shading term
	float gaussianTerm = 0.0;
	if (cosAngIncidence != 0.0) {
		// the vector halfway between FRAG_TO_LIGHT_DIR and FRAG_TO_CAMERA_DIR
		vec3 halfAngleDir = normalize(fragToLightDir + fragToCamDir);
		
		// angle between the FRAG_NORMAL and the halfway vector / material specular term
		float exponent = acos(dot(fragNormal, halfAngleDir)) / gaussianSpecularShininess;
		exponent = -(exponent * exponent);
		gaussianTerm = exp(exponent);
	}

	totalDiffuseLight += lightIntensity * cosAngIncidence;
	totalSpecularLight += lightIntensity * gaussianTerm;
}

vec4 calculateLighting(vec3 FRAG_NORMAL, vec3 FRAG_TO_CAM_DIR, vec4 DIFFUSE_TEX, vec4 SPECULAR_TEX) {
	// -- POINT LIGHTING --
	for (uint i_pLight = 0u; i_pLight < ACTIVE_POINT_LIGHTS; i_pLight++) {
		// get the indexed point light
		PointLight light = pointLights[i_pLight];

		vec3 fragToLight = light.pos_world - fragPos_world;
		float distanceToLight = length(fragToLight);

		vec3 pointLightIntensity = light.color / (light.atten.x + light.atten.y * distanceToLight + light.atten.z * pow(distanceToLight, 2));
		addLight(pointLightIntensity, normalize(fragToLight), FRAG_NORMAL, FRAG_TO_CAM_DIR, mat_gaussianSpecularTerm);
	}

	// -- SPOT LIGHTING --
	for (uint i_sLight = 0u; i_sLight < ACTIVE_SPOT_LIGHTS; i_sLight++) {
		// get the indexed spot light
		SpotLight light = spotLights[i_sLight];

		vec3 fragToLight = light.pos_world - fragPos_world;
		float distanceToLight = length(fragToLight);
		
		// the cosine of the angle between the light direction and the light to fragment direction mapped to [0; 1]
		fragToLight = normalize(fragToLight);
		float cosAngSpreadMapped = 1.0 - ((1.0 - dot(light.dir_world, -fragToLight))/(1.0 - cos(light.angle)));
		cosAngSpreadMapped = clamp(cosAngSpreadMapped, 0.0, 1.0);

		if (cosAngSpreadMapped != 0.0) {
			vec3 spotLightIntensity = light.color / (light.atten.x + light.atten.y * distanceToLight + light.atten.z * pow(distanceToLight, 2));
			spotLightIntensity = spotLightIntensity * pow(cosAngSpreadMapped, 2);
			addLight(spotLightIntensity, fragToLight, FRAG_NORMAL, FRAG_TO_CAM_DIR, mat_gaussianSpecularTerm);
		}
	}

	// -- DIRECTIONAL LIGHTING --
	addLight(directionalLight_color, -directionalLight_dir_world, FRAG_NORMAL, FRAG_TO_CAM_DIR, mat_gaussianSpecularTerm);

	// -- AMBIENT LIGHTING --
	totalDiffuseLight += ambientLight_color;

	// -- COLOR OUTPUT --
	return  (DIFFUSE_TEX * vec4(totalDiffuseLight, 1.0))
			+
			(SPECULAR_TEX * vec4(totalSpecularLight, 1.0));
}

void main() {
	// direction from this fragment to the camera in world space
	vec3 fragToCamDir_world = normalize(camPos_world - fragPos_world);
	vec3 fragToCamDir_tangent = normalize(fragToCamDir_world * tangentToWorldMat3);

	float parScale = 1.0;
	float parBias = 0.01;
	float height = texture(normalSampler, fragTexCoords).a;

	float parValue = parScale * height + parBias;

	//vec2 TEX_COORDS = fragTexCoords + parValue * fragToCamDir_tangent.xy;
	vec2 TEX_COORDS = fragTexCoords;

	// use normal mapping to retrieve the fragment normal
	vec3 FRAG_NORMAL = normalize(tangentToWorldMat3 * (255.0/128.0 * texture2D(normalSampler, TEX_COORDS).rgb - 1.0));

	fragColor = calculateLighting(FRAG_NORMAL, fragToCamDir_world, texture2D(diffuseSampler, TEX_COORDS).rgba, texture2D(specularSampler, TEX_COORDS).rgba);

	//fragColor /= ALOT;
	//fragColor += parValue;

	// TEX_COORDS fails:
	// fragTexCoords is fine (texture sampling works)
	// parValue SEEMS to be okay (output to fragment color, looks properly
	// fragToCamDir_tangent hmmm
	// fragToCamDir_world or tangentToWorldMat3 is wrong??
}
