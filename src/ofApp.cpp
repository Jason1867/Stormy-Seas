#include "ofApp.h"
#include "ofMath.h" // for ofNoise

//--------------------------------------------------------------
void ofApp::setup() {
	ofEnableGLDebugLog();
	ofLogNotice() << "GL_VERSION: " << (const char)glGetString(GL_VERSION);
	ofLogNotice() << "GLSL_VERSION: " << (const char)glGetString(GL_SHADING_LANGUAGE_VERSION);

	ofEnableDepthTest();
	ofDisableArbTex(); // normalized coordinates

	// Mesh setup
	N = 2048; // Higher resolution for better foam detail
	L = 2000.0f; // size

	Enable_WF = false; // Enable Wireframe [MODIFIABLE].

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
	}
	else {
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
	
	// ~~~~~~~~~~~~~~~~~~~~~~ [[[ MODIFIABLE CLOUD DATA ]]] ~~~~~~~~~~~~~~~~~~~~~~ 
	// Clouds Pre-Gen data -> (Position data with respect to the grid, etc):
	ofSeedRandom(0); // Current test seed for now: [0].
	//ofSeedRandom(); // True Random.
	float min_X = -(L/2), max_X = (L/2); // Minimum and maximum X position (with respect to the grid).
	float min_Y = 350, max_Y = 400; // Minimum and maximum HEIGHT/Y position.
	float min_Z = -(L/2), max_Z = (L/2); // Minimum and maximum Z position (with respect to the grid).
	float min_S = 1.0f, max_S = 2.0f; // Minimum and maximum size.
	float c_Num = 100; // Cloud Count.

	cloudColor = glm::vec3(0.75, 0.75, 0.75); // White/grey for clouds.
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// -------------------------- [ CLOUD GENERATION ] --------------------------
	for (int i = 0; i < c_Num; ++i) { // Create c_Num amount of clouds all with their own random data.
		Cloud cloud; // Create cloud.
		float xPos = ofRandom(min_X,max_X); // Choose random X.
		float yPos = ofRandom(min_Y,max_Y); // Choose random Y.
		float zPos = ofRandom(min_Z,max_Z); // Choose random Z.
		float set_S = ofRandom(min_S, max_S); // Choose random size.
		build_Cloud(cloud, glm::vec3(xPos, yPos, zPos), set_S); // Build our cloud with generated data.
		clouds.push_back(cloud); // Add our new cloud to our list.
	}

	// -------------------------- [ CLOUD SHADERS ] --------------------------
	cloudShader.load("cloud_light.vert", "cloud_light.frag"); // Attempt to load cloud shaders.
	if (!cloudShader.isLoaded()) { // Check if CLOUD shader loaded successfully.
		ofLogError("ofApp") << "Failed to load CLOUD shader!";
		ofLogError("ofApp") << "Make sure cloud.vert and cloud.frag are in bin/data/";
	}
	else {
		ofLogNotice("ofApp") << "CLOUD shader loaded successfully";
		ofLogNotice("ofApp") << "Shader program ID: " << cloudShader.getProgram();

		// Print active uniforms and attributes for debugging:
		cloudShader.printActiveUniforms();
		cloudShader.printActiveAttributes();
	}
	//
}

//--------------------------------------------------------------
void ofApp::build_Cloud(Cloud& cloud, const glm::vec3& pos, float size) { // Cloud builder.
	cloud.mesh.clear();
	cloud.mesh.setMode(OF_PRIMITIVE_TRIANGLES); // Set primitive triangles as the mode.
	cloud.mesh.enableNormals(); // Enable normals for the mesh.
	cloud.pos = pos; // Set the position.
	cloud.size = size; // Set the size.

	// Create layers of the cloud.
	struct Layer {
		glm::vec3 off; // Offset with respect to the mesh position.
		float rad; // Radius/Size.
	};

	vector<Layer> layers; // Layer list.
	Layer centre; // Centre layer.
	centre.off = glm::vec3(0.0f, 0.0f, 0.0f); // Set its offset.
	centre.rad = ofRandom(60.0f,80.0f); // Set its radius.
	layers.push_back(centre); // Add the centre to the list of layers.

	int lay_C = (int)ofRandom(12,20); // Layer count for clouds(pick random amount between 12 and 20).
	for (int i = 0; i < lay_C; ++i) {
		float rot = ofRandom(0,TWO_PI); // Set the rotation (not necessary for spheres typically but useful if we use custom textures).
		float dist = ofRandom(25.0f,75.0f); // Determine the distance for the layer from the centre.
		float yOff = ofRandom(-25.0f,25.0f); // Determine the offset on y axis for the layer from the centre.
		float rad = ofRandom(25.0f, 50.0f); // Size of the layer.
		glm::vec3 offset(cos(rot)* dist, yOff, sin(rot)* dist); // Set the offset values.

		layers.push_back({offset,rad}); // Construct/add new layer.
	}

	// Merge all of the layers to form the cloud itself:
	for (const auto& lay : layers) { // Traverse the list of layers and merge them.
		ofSpherePrimitive sphere; // Create the sphere.
		sphere.setRadius(lay.rad); // Set the radius.

		const ofMesh& mesh = sphere.getMesh(); // Grab mesh data (so we can update it).

		for (int i = 0; i < mesh.getNumVertices(); ++i) { // Traverse each vertex of the sphere, and add it to the primary cloud itself.
			cloud.mesh.addVertex(mesh.getVertex(i) + lay.off); // Add the spheres vertex to the cloud with its offset.
			cloud.mesh.addNormal(mesh.getNormal(i)); // Add the spheres normal to the clouds mesh normals.
		}

		const vector<ofIndexType>& indices = mesh.getIndices(); // Get ALL indices of our current sphere after updating the vertices and normals.
		for (int i = 0; i < indices.size(); ++i) { // Go through all the new layers indices and update the main clouds vertex indices with it.
			cloud.mesh.addIndex(cloud.mesh.getNumVertices() + indices[i]); // Add the new indices from our new layer to the main cloud.
		}
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

	if (oceanShader.isLoaded() and !Enable_WF) {
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
	}
	else {
		// Fallback: wireframe without shader
		ofSetColor(100, 150, 200);
		oceanMesh.drawWireframe();
	}

	if (!clouds.empty()) { // Double check to make sure clouds actually exist.
		if (cloudShader.isLoaded() and !Enable_WF) { // If cloud shader is loaded, draw with it.
			cloudShader.begin(); // Initiate shader.
			cloudShader.setUniform3f("lightPos",lightPosition); // Set the light position (that we got from setup).
			cloudShader.setUniform3f("viewPos", 0.0f, 0.0f, 0.0f); // Camera origin.
			cloudShader.setUniform3f("lightColor", lightColor); // Set light color.
			cloudShader.setUniform3f("objectColor", cloudColor); // Set cloud color.

			for (auto& cloud : clouds) { // Traverse all clouds and draw.
				ofPushMatrix(); // Start the stack for this clouds draw.
				ofTranslate(cloud.pos); // Apply position.
				ofScale(cloud.size, cloud.size, cloud.size); // Apply size.
				cloud.mesh.draw(); // Draw the cloud.
				ofPopMatrix(); // So last cloud drawn doesn't effect the new one.
			}

			cloudShader.end();

		}
		else { // Fallback: Wireframe without shader.
			ofSetColor(255,255,255); // Set white color.
			for (auto& cloud : clouds) { // Traverse all clouds and draw.
				ofPushMatrix(); // Start the stack for this clouds draw.
				ofTranslate(cloud.pos); // Apply position.
				ofScale(cloud.size, cloud.size, cloud.size); // Apply size.
				cloud.mesh.drawWireframe(); // Draw the wireframe.
				ofPopMatrix(); // So last cloud drawn doesn't effect the new one.
			}
		}
	}

	cam.end();

	// Display info
	ofSetColor(0,255,0); // Set to green for better visibility.
	string info = "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
	info += "Vertices: " + ofToString(oceanMesh.getNumVertices()) + "\n";
	info += "Indices: " + ofToString(oceanMesh.getNumIndices()) + "\n";
	info += "Ocean Shader: " + string(oceanShader.isLoaded() ? "LOADED" : "NOT LOADED") + "\n";
	info += "Cloud Shader: " + string(cloudShader.isLoaded() ? "LOADED" : "NOT LOADED") + "\n\n";
	info += "Press 'r' to reset camera\n\n";
	info += "[LMB + Hold + Drag] - Move the camera.\n";
	info += "[MMB + Down/Up] - Zoom in and out.\n";
	ofDrawBitmapString(info, 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == ' ') {
		animateWaves = !animateWaves;
	}
	else if (key == '+' || key == '=') {
		timeScale += 0.1f;
	}
	else if (key == '-' || key == '_') {
		timeScale = max(0.0f, timeScale - 0.1f);
	}
	else if (key == 'r' || key == 'R') {
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
