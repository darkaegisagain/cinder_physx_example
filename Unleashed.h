//
//  Unleashed.h
//  Unleashed
//
//  Created by Michael Larson on 5/14/15.
//
//

#ifndef Unleashed_Unleashed_h
#define Unleashed_Unleashed_h

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"

#define NDEBUG
#include "PxPhysicsApi.h"
#include "PxExtensionsAPI.h"
#include "PxDefaultErrorCallback.h"
#include "PxDefaultAllocator.h"
#include "PxDefaultSimulationFilterShader.h"
#include "PxDefaultCpuDispatcher.h"
#include "PxShapeExt.h"
#include "PxMat33.h"
#include "PxSimpleFactory.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace physx;

class UnleashedApp : public AppNative {
public:
    ~UnleashedApp()
    {
        if (mFoundation)
            mFoundation->release();
        
        if (mPhysics)
            mPhysics->release();
    }
    
    void setup();
    void mouseDown( MouseEvent event );
    void keyDown( KeyEvent event );
    void update();

    void getColumnMajor(PxMat33 m, PxVec3 t, float* mat);

    void drawAxis();
    void drawGrid(int grid_size);
    void drawBox(PxShape *pShape, PxRigidActor *pActor);
    void drawSphere(PxShape *pShape, PxRigidActor *pActor);
    void drawShape(PxShape *pShape, PxRigidActor *pActor);

    void drawActor(PxRigidActor* actor);
    void drawRigidDynamicActors();
    void drawRigidActors();
    void drawActors();
    
    void draw();
    
    void fatalError(const char *err);
    uint64_t deltaTime();
    
    unsigned                mFrameCount;
    CameraOrtho             mCamera;
    Quatf                   mSceneRotation;
    
    uint64_t                mLastUpdate;
    
    PxFoundation            *mFoundation;
    PxProfileZoneManager    *mProfileZoneManager;
    PxPhysics               *mPhysics;
    PxMaterial              *mMaterial;
    PxScene                 *mScene;
    PxDefaultCpuDispatcher  *mCpuDispatcher;
    
    PxReal                  myTimestep = 1.0f/60.0f;
    PxRigidActor            *box;
};

#endif
