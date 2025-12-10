#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	struct Cloud { // Cloud structure (for generating multiple).
		ofMesh mesh; // Mesh of cloud.
		glm::vec3 pos; // Position of cloud.
		float size; // Size of cloud.
	};

	void build_Cloud(Cloud& cloud, const glm::vec3& pos, float size); // Builds the cloud.

	vector<Cloud> clouds; // List of clouds.
	ofShader cloudShader; // Cloud Shader.
	glm::vec3 cloudColor; // Cloud Color.


	float gerstnerWave(glm::vec3 pos, float t);

	// Mesh and grid
	ofMesh oceanMesh;
	vector<glm::vec3> vertices;
	int N; // Grid resolution
	float L; // Grid size

	// Gerstner wave parameters
	vector<float> amplitudes;
	vector<float> wavelengths;
	vector<float> speeds;
	vector<glm::vec2> directions;

	// Camera
	ofEasyCam cam;

	// Shader system
	ofShader oceanShader;

	// Lighting
	glm::vec3 lightPosition;
	glm::vec3 lightColor;

	// Water colors
	glm::vec3 waterColorDeep;
	glm::vec3 waterColorShallow;
	glm::vec3 foamColor;

	// Atmospheric effects
	glm::vec3 fogColor;
	float fogDensity;

	// Animation
	bool animateWaves;
	float timeScale;

	// Wireframe Enabling:
	bool Enable_WF = false; // Change to true if you wish to see wireframes instead of shaded drawings.
};
