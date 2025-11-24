#include "ofApp.h"
#include "ofMath.h" // for ofNoise
//--------------------------------------------------------------
void ofApp::setup() {
	ofEnableDepthTest();
	ofDisableArbTex(); // normalized coordinates

	// Mesh setup
	N = 256; // resolution
	L = 2000.0f; // size

	oceanMesh.setMode(OF_PRIMITIVE_TRIANGLES);

	// Gerstner wave parameters
	amplitudes = { 15.0f, 8.0f, 4.0f };
	wavelengths = { 200.0f, 100.0f, 50.0f };
	speeds = { 10.0f, 5.0f, 7.0f };
	directions = { { 1, 0 }, { 0.5, 0.5 }, { 1, -0.2 } };

	// Create grid vertices
	for (int z = 0; z < N; z++) {
		for (int x = 0; x < N; x++) {
			glm::vec3 v;
			v.x = (x - N / 2.0f) * L / N;
			v.y = 0;
			v.z = (z - N / 2.0f) * L / N;
			vertices.push_back(v);
			oceanMesh.addVertex(v);
		}
	}

	// Create mesh indices
	for (int z = 0; z < N - 1; z++) {
		for (int x = 0; x < N - 1; x++) {
			int i = z * N + x;
			oceanMesh.addIndex(i);
			oceanMesh.addIndex(i + 1);
			oceanMesh.addIndex(i + N);

			oceanMesh.addIndex(i + 1);
			oceanMesh.addIndex(i + N + 1);
			oceanMesh.addIndex(i + N);
		}
	}

	// basic normals for future lighting
	oceanMesh.enableNormals();

	// Set camera position
	cam.setDistance(800);
	cam.setNearClip(0.1f);
	cam.setFarClip(3000.0f);
	cam.setPosition(0, 100, 600);
	cam.lookAt(glm::vec3(20, 100, 0));

	float t0 = ofGetElapsedTimef();
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].y = gerstnerWave(vertices[i], t0);
		oceanMesh.setVertex(i, vertices[i]);
	}
}

//--------------------------------------------------------------
float ofApp::gerstnerWave(glm::vec3 pos, float t) {
	float y = 0;

	for (int i = 0; i < amplitudes.size(); i++) {
		// Slightly randomize wavelength per vertex
		float wavelengthVariation = wavelengths[i] * (0.9 + 0.6 * ofNoise(pos.x * 0.01, pos.z * 0.01));
		float k = TWO_PI / wavelengthVariation;
		float c = speeds[i];

		// Slight random phase per wave component
		float phase = ofNoise(i * 0.1) * TWO_PI;

		// Slightly randomize wave direction per vertex
		float angleOffset = ofNoise(pos.x * 0.02, pos.z * 0.02) * 0.2; // radians
		glm::vec2 dir = glm::rotate(directions[i], angleOffset);

		float dot = glm::dot(dir, glm::vec2(pos.x, pos.z));

		// Slightly randomize amplitude per vertex
		float randAmp = amplitudes[i] * (0.8 + 0.4 * ofNoise(pos.x * 0.01, pos.z * 0.01));

		// Add main Gerstner wave component
		y += randAmp * sin(k * dot - c * t + phase);
	}

	// Add small high-frequency “chop” waves for roughness
	float chopAmplitude = 1.0f + 2.0f * ofNoise(pos.x * 0.1, pos.z * 0.1, t * 0.5);
	float chopWavelength = 10.0f + 5.0f * ofNoise(pos.x * 0.05, pos.z * 0.05);
	float chopK = TWO_PI / chopWavelength;
	y += chopAmplitude * sin(chopK * (pos.x + pos.z) - 10 * t);

	// Large-scale low-frequency height modulation
	float heightMod = 0.8 + 0.4 * ofNoise(pos.x * 0.005, pos.z * 0.005);
	y *= heightMod;

	return y;
}



//--------------------------------------------------------------
void ofApp::update() {
	/*float t = ofGetElapsedTimef();
	for (int i = 0; i < vertices.size(); i++) {
		glm::vec3 & v = vertices[i];
		v.y = gerstnerWave(v, t);
		oceanMesh.setVertex(i, v);
	}*/
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(50, 60, 80); // basic stormy-blue background

	cam.begin();

	ofSetColor(100, 100, 140); // flat water color
	oceanMesh.drawWireframe(); // show realistic wave geometry

	cam.end();
}
