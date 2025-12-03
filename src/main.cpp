#include "ofApp.h"
#include "ofMain.h"

//========================================================================
int main() {
	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 2); // Use OpenGL 3.2
	settings.setSize(1920, 1080);
	ofCreateWindow(settings);
	ofRunApp(new ofApp());
}
