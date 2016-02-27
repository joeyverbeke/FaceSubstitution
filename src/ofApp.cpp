#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/joey.jpg");
#endif

#ifdef TARGET_OPENGLES
    shader.load("shadersES2/shader");
#else
    if(ofIsGLProgrammableRenderer()){
        shader.load("shadersGL3/shader");
    }else{
        shader.load("shadersGL2/shader");
    }
#endif
    
	ofSetVerticalSync(true);
	cloneReady = false;
	cam.initGrabber(1280, 720);
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);

    faceSaverTester.setup();
    faceSaverTester.setIterations(25);
    faceSaverTester.setAttempts(4);
    
	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");
	currentFace = 0;
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    croppedFace.allocate(cam.getWidth(), cam.getHeight(), OF_IMAGE_COLOR);
    defaultFace.load("/Users/joey/Documents/of_v0.9.2_osx_release/apps/myApps/FaceSubstitution/FaceSubstitution-master/FaceSubstitution/bin/data/defaultFace/default.jpg");
    
    //mask setup
    blackOnTop.load("black.png");
    brush.load("faceCircle.png");
    
    circleMaskFbo.allocate(cam.getWidth(), cam.getHeight());
    displayCircleFbo.allocate(cam.getWidth(), cam.getHeight());
    
    circleMaskFbo.begin();
    ofClear(0,0,0,255);
    circleMaskFbo.end();
    
    displayCircleFbo.begin();
    ofClear(0, 0, 0, 255);
    displayCircleFbo.end();
    
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
		
		cloneReady = camTracker.getFound();
        
		if(cloneReady) {
			ofMesh camMesh = camTracker.getImageMesh();
			camMesh.clearTexCoords();
			camMesh.addTexCoords(srcPoints);
            
			maskFbo.begin();
			ofClear(0, 255);
			camMesh.draw();
			maskFbo.end();
			
			srcFbo.begin();
			ofClear(0, 255);
			src.bind();
			camMesh.draw();
			src.unbind();
			srcFbo.end();
			
			clone.setStrength(16);
			clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
            
		}
        else
        {
            loadFace("/Users/joey/Documents/of_v0.9.2_osx_release/apps/myApps/FaceSubstitution/FaceSubstitution-master/FaceSubstitution/bin/data/faces/joey.jpg");
        }
	}
}

void ofApp::draw() {
    ofBackground(ofColor::black);
    
    ofSetColor(255);
    
	if(src.getWidth() > 0 && cloneReady) {
		//clone.draw(0, 0);
        
        ofRectangle faceBox(camTracker.getPosition().x - camTracker.getScale() * 30, camTracker.getPosition().y - camTracker.getScale() * 30,
                            camTracker.getScale() * 60, camTracker.getScale() * 60);
  
        //alpha masking
        circleMaskFbo.begin();
        ofClear(0, 0, 0, 0); //might need to be 0,0,0,0
        brush.draw(camTracker.getPosition().x - camTracker.getScale() * 25, camTracker.getPosition().y - camTracker.getScale() * 25,
                   camTracker.getScale() * 50, camTracker.getScale() * 50);
        
        circleMaskFbo.end();
        
        //shader masking
        displayCircleFbo.begin();
        ofClear(0, 0, 0, 0);
        
        shader.begin();
        shader.setUniformTexture("maskTex", circleMaskFbo.getTexture(), 1);
        
        clone.draw(0, 0); //background first
//        cam.draw(0,0);
        
        
        shader.end();
        displayCircleFbo.end();
        
        blackOnTop.draw(0, 0);
        
        displayCircleFbo.draw(0, 0);
        
        
        //mirrored video output
//        cam.draw(cam.getWidth(), 0, -cam.getWidth(), cam.getHeight());
        //cam.draw(0, 0)
        
        
        //ofDrawRectangle(faceBox);
        
        ofPixels camToImage = cam.getPixels();
        croppedFace.setFromPixels(camToImage);
        croppedFace.crop(faceBox.getX(), faceBox.getY(), faceBox.getWidth(), faceBox.getHeight());
        //croppedFace.draw(0, 0);
        faceSeenLastFrame = true;
        faceHasBeenSaved = false;
        lastSixtyFaces[ofGetFrameNum() % 59].clone(croppedFace);
        
	} else {
        time_t currentTime = time(NULL);
        
        if(faceSeenLastFrame)
        {
            time(&timeWhenFaceWasLost);
            frameWhenFaceWasLost = ofGetFrameNum();
            
//            int first = ofGetFrameNum() % 59;
//            int second = (ofGetFrameNum() - 50) % 59;

            faceSeenLastFrame = false;
            cam.draw(0,0);
        }
        else if (timeWhenFaceWasLost != 0 && (time(NULL) - timeWhenFaceWasLost > 5) && !faceHasBeenSaved)
        {
            int faceIndex = 0;
            bool faceFound = false;
            
            for(int i=0; i < 60; i++)
            {
                if(lastSixtyFaces[i].getWidth() > 0)
                {
                    faceSaverTester.update(toCv(lastSixtyFaces[i]));
                
                    if(faceSaverTester.getFound())
                    {
                        faceIndex = i;
                        faceFound = true;
                    }
                    if(faceFound)
                        break;
                }
            }
            if(faceFound)
                lastSixtyFaces[faceIndex].save("/Users/joey/Documents/of_v0.9.2_osx_release/apps/myApps/FaceSubstitution/FaceSubstitution-master/FaceSubstitution/bin/data/faces/joey.jpg");
            else
                defaultFace.save("/Users/joey/Documents/of_v0.9.2_osx_release/apps/myApps/FaceSubstitution/FaceSubstitution-master/FaceSubstitution/bin/data/faces/joey.jpg");
            
//            lastSixtyFaces[((frameWhenFaceWasLost % 59) - 50) % 59].save("/Users/joey/Documents/of_v0.9.2_osx_release/apps/myApps/FaceSubstitution/FaceSubstitution-master/FaceSubstitution/bin/data/faces/joey.jpg");
            
            faceHasBeenSaved = true;
            cam.draw(0,0);
        }
        else
            cam.draw(0, 0);
        
	}
	
	if(!camTracker.getFound()) {
		drawHighlightString("camera face not found", 10, 10);
	}
	if(src.getWidth() == 0) {
		drawHighlightString("drag an image here", 10, 30);
	} else if(!srcTracker.getFound()) {
		drawHighlightString("image face not found", 10, 30);
	}
    
    drawHighlightString(ofToString(ofGetFrameRate()), 50, 50);
}

void ofApp::loadFace(string face){
	src.loadImage(face);
	if(src.getWidth() > 0) {
		srcTracker.update(toCv(src));
		srcPoints = srcTracker.getImagePoints();
	}
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0]);
}

void ofApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		currentFace++;
		break;
	case OF_KEY_DOWN:
		currentFace--;
		break;
	}
	currentFace = ofClamp(currentFace,0,faces.size());
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
}

void ofApp::iterativeFaceDraw()
{
    ofSetColor(ofColor::white);
    ofSetLineWidth(2.0f);
    
    for(int i=0; i < ofGetFrameNum() % camTracker.getImagePoints().size(); i++)
    {
        //connecting point
        int j;
        if(i == camTracker.getImagePoints().size()-1)
            j = 0;
        else
            j = i+1;
        
        ofDrawLine(camTracker.getImagePoint(i).x, camTracker.getImagePoint(i).y,
                   camTracker.getImagePoint(j).x, camTracker.getImagePoint(j).y);
    }
}
