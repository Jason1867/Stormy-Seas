#include "ofApp.h"
#include "ofMath.h" // for ofNoise

//--------------------------------------------------------------
void ofApp::setup() {
	ofEnableDepthTest();
	ofDisableArbTex(); // normalized coordinates

	// Mesh setup
	N = 1024; // Higher resolution for better foam detail
	L = 2000.0f; // size

	oceanMesh.setMode(OF_PRIMITIVE_TRIANGLES);

	// Gerstner wave parameters - realistic wind-driven waves
	// Gentler amplitudes for ocean swells, not mountains
	amplitudes = { 10.0f, 5.0f, 3.5f, 1.5f, 0.8f };
	// Longer wavelengths relative to amplitude for gradual buildup
	wavelengths = { 200.0f, 100.0f, 55.0f, 30.0f, 16.0f };
	// Speed follows physics: c = sqrt(g*L/(2*PI)) approximately
	speeds = { 10.5f, 7.5f, 5.5f, 4.2f, 3.0f };
	// Primary wind from northwest, secondary from west
	directions = {
		{ 0.85, 0.52 }, // Main wind direction (NW)
		{ 0.92, 0.38 }, // Slight variation from main
		{ 0.78, 0.62 }, // Secondary swell angle
		{ 1.0, 0.1 }, // Minor cross-wind component
		{ 0.88, 0.47 } // High-frequency chop aligned with wind
	};

	// Create grid vertices
	for (int z = 0; z < N; z++) {
		for (int x = 0; x < N; x++) {
			glm::vec3 v;
			v.x = (x - N / 2.0f) * L / N;
			v.y = 0;
			v.z = (z - N / 2.0f) * L / N;
			vertices.push_back(v);
			oceanMesh.addVertex(v);

			// Add texture coordinates for shader
			glm::vec2 texCoord;
			texCoord.x = (float)x / (N - 1);
			texCoord.y = (float)z / (N - 1);
			oceanMesh.addTexCoord(texCoord);
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

	// Enable normals for lighting
	oceanMesh.enableNormals();

	// Set camera position
	cam.setDistance(800);
	cam.setNearClip(0.1f);
	cam.setFarClip(3000.0f);
	cam.setPosition(0, 100, 600);
	cam.lookAt(glm::vec3(20, 100, 0));

	// Load shader
	oceanShader.load("ocean.vert", "ocean.frag");

	// Check if shader loaded successfully
	if (!oceanShader.isLoaded()) {
		ofLogError("ofApp") << "Failed to load ocean shader!";
		ofLogError("ofApp") << "Make sure ocean.vert and ocean.frag are in bin/data/";
	} else {
		ofLogNotice("ofApp") << "Ocean shader loaded successfully";
		ofLogNotice("ofApp") << "Shader program ID: " << oceanShader.getProgram();

		// Print active uniforms and attributes for debugging
		oceanShader.printActiveUniforms();
		oceanShader.printActiveAttributes();
	}

	// Lighting setup - strong moonlight from above and to the side
	lightPosition = glm::vec3(500, 800, 400); // Higher and further for moon effect
	lightColor = glm::vec3(0.9, 0.95, 1.0); // Cool blue-white moonlight

	// Water colors (realistic ocean)
	waterColorDeep = glm::vec3(0.01, 0.05, 0.10); // Very dark deep water
	waterColorShallow = glm::vec3(0.08, 0.15, 0.22); // Medium blue
	foamColor = glm::vec3(0.85, 0.90, 0.95); // Bright foam

	// Fog settings (thick dark mist)
	fogColor = glm::vec3(0.02, 0.03, 0.05); // Very dark mist
	fogDensity = 0.0008f; // Much thicker fog

	// Animation settings
	animateWaves = true;
	timeScale = 1.0f;

	// Initialize wave heights
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

	// Add small high-frequency "chop" waves for roughness
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
	// Wave animation is now handled entirely in the vertex shader
	// This keeps the CPU-side mesh static and lets GPU do all the work
}

//--------------------------------------------------------------
void ofApp::draw() {
	// Stormy sky background
	ofBackgroundGradient(ofColor(80, 90, 100), ofColor(40, 50, 60), OF_GRADIENT_LINEAR);

	cam.begin();

	if (oceanShader.isLoaded()) {
		// Use WORLD SPACE light position (don't transform by camera)
		// This keeps the light fixed regardless of camera movement
		oceanShader.begin();

		// Time uniform
		float currentTime = animateWaves ? ofGetElapsedTimef() * timeScale : 0.0f;
		oceanShader.setUniform1f("time", currentTime);

		// Wave parameters
		oceanShader.setUniform1fv("amplitudes", amplitudes.data(), (int)amplitudes.size());
		oceanShader.setUniform1fv("wavelengths", wavelengths.data(), (int)wavelengths.size());
		oceanShader.setUniform1fv("speeds", speeds.data(), (int)speeds.size());

		// Pass directions as array of vec2
		for (int i = 0; i < directions.size(); i++) {
			string uniformName = "directions[" + ofToString(i) + "]";
			oceanShader.setUniform2f(uniformName, directions[i].x, directions[i].y);
		}

		// Lighting uniforms - transform to view space
		glm::mat4 viewMatrix = cam.getModelViewMatrix();
		glm::vec4 lightViewPos = viewMatrix * glm::vec4(lightPosition, 1.0);
		oceanShader.setUniform3f("lightPosition", lightViewPos.x, lightViewPos.y, lightViewPos.z);
		oceanShader.setUniform3f("lightColor", lightColor.r, lightColor.g, lightColor.b);

		// Water color uniforms
		oceanShader.setUniform3f("waterColorDeep", waterColorDeep.r, waterColorDeep.g, waterColorDeep.b);
		oceanShader.setUniform3f("waterColorShallow", waterColorShallow.r, waterColorShallow.g, waterColorShallow.b);
		oceanShader.setUniform3f("foamColor", foamColor.r, foamColor.g, foamColor.b);

		// Atmospheric uniforms
		oceanShader.setUniform3f("fogColor", fogColor.r, fogColor.g, fogColor.b);
		oceanShader.setUniform1f("fogDensity", fogDensity);

		// Draw the ocean mesh with shader
		oceanMesh.draw();

		oceanShader.end();
	} else {
		// Fallback: wireframe without shader
		ofSetColor(100, 150, 200);
		oceanMesh.drawWireframe();
	}

	cam.end();

	// Display info
	ofSetColor(255);
	string info = "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
	info += "Vertices: " + ofToString(oceanMesh.getNumVertices()) + "\n";
	info += "Indices: " + ofToString(oceanMesh.getNumIndices()) + "\n";
	info += "Shader: " + string(oceanShader.isLoaded() ? "LOADED" : "NOT LOADED") + "\n\n";
	info += "Press SPACE to toggle animation\n";
	info += "Press +/- to adjust time scale\n";
	info += "Press 'r' to reset camera\n\n";
	info += "Animation: " + string(animateWaves ? "ON" : "OFF") + "\n";
	info += "Time Scale: " + ofToString(timeScale, 2);
	ofDrawBitmapString(info, 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == ' ') {
		animateWaves = !animateWaves;
	} else if (key == '+' || key == '=') {
		timeScale += 0.1f;
	} else if (key == '-' || key == '_') {
		timeScale = max(0.0f, timeScale - 0.1f);
	} else if (key == 'r' || key == 'R') {
		// Reset camera
		cam.setPosition(0, 100, 600);
		cam.lookAt(glm::vec3(0, 0, 0));
		cam.setDistance(800);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}
