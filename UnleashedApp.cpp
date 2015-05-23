
#include "Unleashed.h"
#include <mach/mach_time.h>

#include "cinder/params/Params.h"

static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader=PxDefaultSimulationFilterShader;

float randf(float range)
{
    float f = (random() & 0xffffff);
    
    f = f / pow(2.0, 24);
    
    return range * f;
}

Vec3f randVec(float range)
{
    return Vec3f(randf(range), randf(range), randf(range));
}

void UnleashedApp::fatalError(const char *err)
{
    __builtin_printf("fatalError: %s\n", err);
    
    assert(0);
}

uint64_t UnleashedApp::deltaTime()
{
    uint64_t end = mach_absolute_time();
    uint64_t delta;
    
    if (end > mLastUpdate)
    {
        delta = end - mLastUpdate;
    }
    else
    {
        delta = 0;
        
        delta -= mLastUpdate;
        
        delta += end;
    }
    
    mach_timebase_info_t info;
    
    mach_timebase_info(info);
    
    delta = delta * info->numer / info->denom;
    
    return delta;
}

void UnleashedApp::setup()
{
    // Cinder setup
    WindowRef window = getWindow();
    
    window->setSize(512, 512);
    window->setPos(32, 64);

    params::InterfaceGl mParams;
    mParams = params::InterfaceGl( "Unleashed", Vec2i( 225, 200 ) );
    mParams.addParam( "Scene Rotation", &mSceneRotation );
    
    mLastUpdate = mach_absolute_time();
    gl::disableVerticalSync();
    
    // PhysX setup
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
                                     gDefaultErrorCallback);
    if(!mFoundation)
        fatalError("PxCreateFoundation failed!");
    
    bool recordMemoryAllocations = true;
    
    mProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(mFoundation);
    if(!mProfileZoneManager)
        fatalError("PxProfileZoneManager::createProfileZoneManager failed!");
    
    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation,
                               PxTolerancesScale(), recordMemoryAllocations, mProfileZoneManager);
    if(!mPhysics)
        fatalError("PxCreatePhysics failed!");

    //Create the scene
    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity=PxVec3(0.0f, -9.8f, 0.0f);
    if(!sceneDesc.cpuDispatcher) {
        PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
        if(!mCpuDispatcher)
            cerr<<"PxDefaultCpuDispatcherCreate failed!"<<endl;
        sceneDesc.cpuDispatcher = mCpuDispatcher;
    }
    if(!sceneDesc.filterShader)
        sceneDesc.filterShader  = gDefaultFilterShader;
    
    mScene = mPhysics->createScene(sceneDesc);
    if (!mScene)
        cerr<<"createScene failed!"<<endl;
    mScene->setVisualizationParameter(PxVisualizationParameter::eSCALE,     1.0);
    mScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
    
    mMaterial = mPhysics->createMaterial(0.5,0.5,0.5);

    //Create actors
    //1) Create ground plane
    PxTransform pose = PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
    PxRigidStatic* plane = mPhysics->createRigidStatic(pose);
    if (!plane)
        cerr<<"create plane failed!"<<endl;
    PxShape* shape = plane->createShape(PxPlaneGeometry(), *mMaterial);
    if (!shape)
        cerr<<"create shape failed!"<<endl;
    mScene->addActor(*plane);
    
    //2) Create sphere
    PxReal density = 1.0f;
    PxTransform sphereTransform(PxVec3(0.0f, 4.0f, 0.0f));
    PxSphereGeometry sphere(0.5);
    
    PxRigidDynamic *actor = PxCreateDynamic(*mPhysics, sphereTransform, sphere, *mMaterial, density);
    actor->setAngularDamping(0.75);
    actor->setLinearVelocity(PxVec3(0,0,0));
    if (!actor)
        cerr<<"create actor failed!"<<endl;
    mScene->addActor(*actor);
}

void UnleashedApp::mouseDown( MouseEvent event )
{
}

void UnleashedApp::keyDown( KeyEvent event )
{
    switch(event.getCode())
    {
        case KeyEvent::KEY_SPACE:
        {
            PxReal density = 1.0f;
            PxTransform transform(PxVec3(0.0f, 10.0f, 0.0f), PxQuat::createIdentity());
            PxSphereGeometry sphere(0.5);
            
            PxRigidDynamic *actor = PxCreateDynamic(*mPhysics, transform, sphere, *mMaterial, density);
            actor->setAngularDamping(0.75);
            actor->setLinearVelocity(PxVec3(0,0,0));
            if (!actor)
                cerr<<"create actor failed!"<<endl;
            mScene->addActor(*actor);
            break;
        }
            
        default:
            break;
    }
}

void UnleashedApp::update()
{
    uint64_t timeStep = deltaTime();
    
    float dtF = (float)timeStep / 1E9;
    
    printf("dtF:%f\n", dtF);
    
    mScene->simulate(dtF);
    
    //...perform useful work here using previous frame's state data
    while(!mScene->fetchResults() )
    {
        // do something useful
    }
    
    if ((mFrameCount % 10) == 0)
    {
        if (mFrameCount & 1)
        {
            PxReal density = randf(1.0f);
            PxTransform transform(PxVec3(0.0f, 10.0f, 0.0f), PxQuat::createIdentity());
            PxSphereGeometry sphere(0.5);
            
            PxRigidDynamic *actor = PxCreateDynamic(*mPhysics, transform, sphere, *mMaterial, density);
            actor->setAngularDamping(0.75);
            actor->setLinearVelocity(PxVec3(randf(1.0), randf(1.0), randf(1.0)));
            if (!actor)
                cerr<<"create actor failed!"<<endl;
            mScene->addActor(*actor);
        }
        else
        {
            PxReal density = 1.0f;
            PxTransform transform(PxVec3(0.0f, 10.0f, 0.0f), PxQuat::createIdentity());
            PxBoxGeometry box(0.5, 0.5, 0.5);
            
            PxRigidDynamic *actor = PxCreateDynamic(*mPhysics, transform, box, *mMaterial, density);
            actor->setAngularDamping(0.75);
            actor->setLinearVelocity(PxVec3(randf(1.0), randf(1.0), randf(1.0)));
            if (!actor)
                cerr<<"create actor failed!"<<endl;
            mScene->addActor(*actor);
        }
    }
    
    mLastUpdate = mach_absolute_time();
}

void clear( const ColorA &color, bool clearDepthBuffer )
{
    glClearColor( color.r, color.g, color.b, color.a );

    if( clearDepthBuffer )
    {
        glDepthMask( GL_TRUE );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }
    else
    {
        glClear( GL_COLOR_BUFFER_BIT );
    }
}

void UnleashedApp::getColumnMajor(PxMat33 m, PxVec3 t, float* mat)
{
    mat[0] = m.column0[0];
    mat[1] = m.column0[1];
    mat[2] = m.column0[2];
    mat[3] = 0;
    
    mat[4] = m.column1[0];
    mat[5] = m.column1[1];
    mat[6] = m.column1[2];
    mat[7] = 0;
    
    mat[8] = m.column2[0];
    mat[9] = m.column2[1];
    mat[10] = m.column2[2];
    mat[11] = 0;
    
    mat[12] = t[0];
    mat[13] = t[1];
    mat[14] = t[2];
    mat[15] = 1;
}

void UnleashedApp::drawAxis()
{
    gl::color(1.0, 0.0, 0.0);
    gl::drawLine(Vec3f(0,0,0), Vec3f(1,0,0));
    gl::color(0.0, 1.0, 0.0);
    gl::drawLine(Vec3f(0,0,0), Vec3f(0,1,0));
    gl::color(0.0, 0.0, 1.0);
    gl::drawLine(Vec3f(0,0,0), Vec3f(0,0,1));
}

void UnleashedApp::drawGrid(int grid_size)
{
    glBegin(GL_LINES);
    glColor3f(0.75f, 0.75f, 0.75f);
    for(int i=-grid_size;i<=grid_size;i++)
    {
        glVertex3f((float)i,0,(float)-grid_size);
        glVertex3f((float)i,0,(float)grid_size);
        
        glVertex3f((float)-grid_size,0,(float)i);
        glVertex3f((float)grid_size,0,(float)i);
    }
    glEnd();
}

void UnleashedApp::drawBox(PxShape *pShape, PxRigidActor *pActor)
{
    PxTransform pT = PxShapeExt::getGlobalPose(*pShape, *pActor);
    PxBoxGeometry bg;
    
    pShape->getBoxGeometry(bg);
    
    PxMat33 m = PxMat33(pT.q );
    float mat[16];
    getColumnMajor(m,pT.p, mat);
    
    glPushMatrix();
    glMultMatrixf(mat);
    
    float size = bg.halfExtents.x*2;
    
    gl::drawCube( Vec3f(0,0,0), Vec3f(size, size, size));
    
    glPopMatrix();
}

void UnleashedApp::drawSphere(PxShape *pShape, PxRigidActor *pActor)
{
    PxTransform pT = PxShapeExt::getGlobalPose(*pShape, *pActor);
    PxSphereGeometry sg;
    
    pShape->getSphereGeometry(sg);
    
    PxMat33 m = PxMat33(pT.q );
    float mat[16];
    getColumnMajor(m,pT.p, mat);
    
    glPushMatrix();
    glMultMatrixf(mat);
    
    gl::drawSphere( Vec3f(0,0,0), sg.radius, 30);
    
    glPopMatrix();
}


void UnleashedApp::drawShape(PxShape *shape, PxRigidActor *actor)
{
    PxGeometryType::Enum type = shape->getGeometryType();
    switch(type)
    {
        case PxGeometryType::eBOX:
            drawBox(shape, actor);
            break;

        case PxGeometryType::eSPHERE:
            drawSphere(shape, actor);
            break;
            
        default:
            break;
    }
}

void UnleashedApp::drawActor(PxRigidActor* actor)
{
    PxU32 nShapes = actor->getNbShapes();
    PxShape **shapes=new PxShape*[nShapes];
    
    actor->getShapes(shapes, nShapes);
    while (nShapes--)
    {
        drawShape(shapes[nShapes], actor);
    } 
    delete [] shapes;
}

void UnleashedApp::drawRigidDynamicActors()
{
    PxU32 nActors = mScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
    PxRigidDynamic **actors = new PxRigidDynamic*[nActors];
    
    mScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, (PxActor **)actors, nActors, 0);
    while (nActors--)
    {
        drawActor(actors[nActors]);
    }
    delete [] actors;
}

void UnleashedApp::drawRigidActors()
{
    PxU32 nActors = mScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC);
    PxRigidActor **actors = new PxRigidActor*[nActors];
    
    mScene->getActors(physx::PxActorTypeFlag::eRIGID_STATIC, (PxActor **)actors, nActors, 0);
    while (nActors--)
    {
        drawActor(actors[nActors]);
    }
    delete [] actors;    
}

void UnleashedApp::drawActors()
{
    drawRigidActors();
    drawRigidDynamicActors();
}

void UnleashedApp::draw()
{
    WindowRef window = getWindow();
    Vec2f size = window->getSize();
    Rectf frame(0,0, size.x, size.y);

    CameraPersp camera(getWindowWidth(), getWindowHeight(), 60.0, 1.0, 1000.0);
    
    float dist=-10;
    camera.lookAt(Vec3f(dist, -dist, dist), Vec3f(0,0,0), Vec3f(0,1,0));
    glMatrixMode(GL_PROJECTION);
    gl::setMatrices(camera);

    // clear out the window with black
    gl::clear( Color( 0, 0, 0 ) );
    
    // draw axis
    drawAxis();
    
    // draw grid
    gl::color(0.0, 0.0, 1.0);
    drawGrid(10);
    
    gl::color(1.0, 0.0, 0.0);
    drawActors();
    
    mFrameCount++;
}

CINDER_APP_NATIVE( UnleashedApp, RendererGl )
