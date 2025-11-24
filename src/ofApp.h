#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	// Ocean mesh
	ofMesh oceanMesh;
	std::vector<glm::vec3> vertices;

	// Mesh resolution and size
	int N; // grid resolution
	float L; // ocean patch size

	// Gerstner wave parameters
	std::vector<float> amplitudes;
	std::vector<float> wavelengths;
	std::vector<float> speeds;
	std::vector<glm::vec2> directions;

	// Wave function
	float gerstnerWave(glm::vec3 pos, float t);

	// Camera
	ofEasyCam cam;
};
