#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include <time.h>

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	void loadFace(string face);
    void iterativeFaceDraw();
	
	void keyPressed(int key);

	ofxFaceTrackerThreaded camTracker;
	ofVideoGrabber cam;
	
	ofxFaceTracker srcTracker;
	ofImage src;
	vector<ofVec2f> srcPoints;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo, maskFbo;

	ofDirectory faces;
	int currentFace;
    
    ofImage croppedFace;
    bool faceSeenLastFrame = false;
    ofImage lastSixtyFaces [60];
    
    time_t timeWhenFaceWasLost;
    int frameWhenFaceWasLost;
    bool faceHasBeenSaved = false;
    
    //testing for face before saving pic
    ofxFaceTracker faceSaverTester;
    ofImage defaultFace;
    
    //alpha masking
    ofFbo circleMaskFbo;
    ofFbo displayCircleFbo;
    ofShader shader;
    ofImage blackOnTop;
    ofImage brush;
};
