/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef __SPLINES_H
#define __SPLINES_H

#ifdef RTCW_VANILLA
//extern "C" {
#ifdef Q3RADIANT
  #include "../qgl.h"
#else
  #include "qgl.h"
#endif
//}
#endif // RTCW_VANILLA
#include "util_list.h"
#include "util_str.h"
#include "math_vector.h"

typedef int fileHandle_t;

extern void glBox( idVec3 &color, idVec3 &point, float size );
extern void glLabeledPoint( idVec3 &color, idVec3 &point, float size, const char *label );

static idVec4 blue( 0, 0, 1, 1 );
static idVec4 red( 1, 0, 0, 1 );

class idPointListInterface {
public:
idPointListInterface() {
	selectedPoints.Clear();
};
virtual ~idPointListInterface() {
};

virtual int numPoints() {
	return 0;
}

virtual void addPoint( const float x, const float y, const float z ) {}
virtual void addPoint( const idVec3 &v ) {}
virtual void removePoint( int index ) {}
virtual idVec3 *getPoint( int index ) { return NULL; }

int selectPointByRay( float ox, float oy, float oz, float dx, float dy, float dz, bool single ) {
	idVec3 origin( ox, oy, oz );
	idVec3 dir( dx, dy, dz );
	return selectPointByRay( origin, dir, single );
}

int selectPointByRay( const idVec3 origin, const idVec3 direction, bool single ) {
	int i, besti, count;
	float d, bestd;
	idVec3 temp, temp2;

	// find the point closest to the ray
	besti = -1;
	bestd = 8;
	count = numPoints();

	for ( i = 0; i < count; i++ ) {
		temp = *getPoint( i );
		temp2 = temp;
		temp -= origin;
		d = DotProduct( temp, direction );
		__VectorMA( origin, d, direction, temp );
		temp2 -= temp;
		d = temp2.Length();
		if ( d <= bestd ) {
			bestd = d;
			besti = i;
		}
	}

	if ( besti >= 0 ) {
		selectPoint( besti, single );
	}

	return besti;
}

int isPointSelected( int index ) {
	int count = selectedPoints.Num();
	for ( int i = 0; i < count; i++ ) {
		if ( selectedPoints[i] == index ) {
			return i;
		}
	}
	return -1;
}

int selectPoint( int index, bool single ) {
	if ( index >= 0 && index < numPoints() ) {
		if ( single ) {
			deselectAll();
		} else {
			if ( isPointSelected( index ) >= 0 ) {
				selectedPoints.Remove( index );
			}
		}
		return selectedPoints.Append( index );
	}
	return -1;
}

void selectAll() {
	selectedPoints.Clear();
	for ( int i = 0; i < numPoints(); i++ ) {
		selectedPoints.Append( i );
	}
}

void deselectAll() {
	selectedPoints.Clear();
}

int numSelectedPoints();

idVec3 *getSelectedPoint( int index ) {
	assert( index >= 0 && index < numSelectedPoints() );
	return getPoint( selectedPoints[index] );
}

virtual void updateSelection( float x, float y, float z ) {
	idVec3 move( x, y, z );
	updateSelection( move );
}

virtual void updateSelection( const idVec3 &move ) {
	int count = selectedPoints.Num();
	for ( int i = 0; i < count; i++ ) {
		*getPoint( selectedPoints[i] ) += move;
	}
}

#ifdef RTCW_VANILLA
void drawSelection() {
	int count = selectedPoints.Num();
	for ( int i = 0; i < count; i++ ) {
		glBox( red, *getPoint( selectedPoints[i] ), 4 );
	}
}
#endif // RTCW_VANILLA

protected:
idList<int> selectedPoints;

};


class idSplineList {

public:

idSplineList() {
	clear();
}

idSplineList( const char *p ) {
	clear();
	name = p;
};

~idSplineList() {
	clear();
};

void clearControl() {
	for ( int i = 0; i < controlPoints.Num(); i++ ) {
		delete controlPoints[i];
	}
	controlPoints.Clear();
}

void clearSpline() {
	for ( int i = 0; i < splinePoints.Num(); i++ ) {
		delete splinePoints[i];
	}
	splinePoints.Clear();
}

void parse( const char *( *text ) );
void write( fileHandle_t file, const char *name );

void clear() {
	clearControl();
	clearSpline();
	splineTime.Clear();
	selected = NULL;
	dirty = true;
	activeSegment = 0;
	granularity = 0.025;
	pathColor.set( 1.0, 0.5, 0.0 );
	controlColor.set( 0.7, 0.0, 1.0 );
	segmentColor.set( 0.0, 0.0, 1.0 );
	activeColor.set( 1.0, 0.0, 0.0 );
}

void initPosition( int startTime, int totalTime );
const idVec3 *getPosition( int time );


#ifdef RTCW_VANILLA
void draw( bool editMode );
void addToRenderer();
#endif // RTCW_VANILLA

void setSelectedPoint( idVec3 *p );
idVec3 *getSelectedPoint() {
	return selected;
}

void addPoint( const idVec3 &v ) {
	controlPoints.Append( new idVec3( v ) );
	dirty = true;
}

void addPoint( float x, float y, float z ) {
	controlPoints.Append( new idVec3( x, y, z ) );
	dirty = true;
}

void updateSelection( const idVec3 &move );

void startEdit() {
	editMode = true;
}

void stopEdit() {
	editMode = false;
}

void buildSpline();

void setGranularity( float f ) {
	granularity = f;
}

float getGranularity() {
	return granularity;
}

int numPoints() {
	return controlPoints.Num();
}

idVec3 *getPoint( int index ) {
	assert( index >= 0 && index < controlPoints.Num() );
	return controlPoints[index];
}

idVec3 *getSegmentPoint( int index ) {
	assert( index >= 0 && index < splinePoints.Num() );
	return splinePoints[index];
}


void setSegmentTime( int index, int time ) {
	assert( index >= 0 && index < splinePoints.Num() );
	splineTime[index] = time;
}

int getSegmentTime( int index ) {
	assert( index >= 0 && index < splinePoints.Num() );
	return (int)splineTime[index];
}
void addSegmentTime( int index, int time ) {
	assert( index >= 0 && index < splinePoints.Num() );
	splineTime[index] += time;
}

float totalDistance();

static idVec3 zero;

int getActiveSegment() {
	return activeSegment;
}

void setActiveSegment( int i ) {
	//assert(i >= 0 && (splinePoints.Num() > 0 && i < splinePoints.Num()));
	activeSegment = i;
}

int numSegments() {
	return splinePoints.Num();
}

void setColors( idVec3 &path, idVec3 &segment, idVec3 &control, idVec3 &active ) {
	pathColor = path;
	segmentColor = segment;
	controlColor = control;
	activeColor = active;
}

const char *getName() {
	return name.c_str();
}

void setName( const char *p ) {
	name = p;
}

bool validTime() {
	if ( dirty ) {
		buildSpline();
	}
	// gcc doesn't allow static casting away from bools
	// why?  I've no idea...
	return (bool)( splineTime.Num() > 0 && splineTime.Num() == splinePoints.Num() );
}

void setTime( int t ) {
	time = t;
}

void setBaseTime( int t ) {
	baseTime = t;
}

protected:
idStr name;
float calcSpline( int step, float tension );
idList<idVec3*> controlPoints;
idList<idVec3*> splinePoints;
idList<double> splineTime;
idVec3 *selected;
idVec3 pathColor, segmentColor, controlColor, activeColor;
float granularity;
bool editMode;
bool dirty;
int activeSegment;
int baseTime;
int time;
friend class idCamera;
};

// time in milliseconds
// velocity where 1.0 equal rough walking speed
struct idVelocity {
	idVelocity( int start, int duration, float s ) {
		startTime = start;
		time = duration;
		speed = s;
	}
	int startTime;
	int time;
	float speed;
};

// can either be a look at or origin position for a camera
//
class idCameraPosition : public idPointListInterface {
public:

#if !defined RTCW_MP
virtual void clearVelocities() {
	// TTimo: MSVCism
	int i;
	for ( i = 0; i < velocities.Num(); i++ ) {
		delete velocities[i];
		velocities[i] = NULL;
	}
	velocities.Clear();
}
#endif // RTCW_XX

#if !defined RTCW_MP
virtual void clear() {
	editMode = false;
	clearVelocities();
}
#else
virtual void clear() {
	editMode = false;
	for ( int i = 0; i < velocities.Num(); i++ ) {
		delete velocities[i];
		velocities[i] = NULL;
	}
	velocities.Clear();
}
#endif // RTCW_XX

idCameraPosition( const char *p ) {
	name = p;
}

idCameraPosition() {
	time = 0;
	name = "position";
}

idCameraPosition( int t ) {
	time = t;
}

virtual ~idCameraPosition() {
	clear();
}


// this can be done with RTTI syntax but i like the derived classes setting a type
// makes serialization a bit easier to see
//
enum positionType {
	FIXED = 0x00,
	INTERPOLATED,
	SPLINE,
	POSITION_COUNT
};


virtual void start( int t ) {
	startTime = t;
}

int getTime() {
	return time;
}

virtual void setTime( int t ) {
	time = t;
}

float getBaseVelocity() {
	return baseVelocity;
}

float getVelocity( int t ) {
	int check = t - startTime;
	for ( int i = 0; i < velocities.Num(); i++ ) {
		if ( check >= velocities[i]->startTime && check <= velocities[i]->startTime + velocities[i]->time ) {
			return velocities[i]->speed;
		}
	}
	return baseVelocity;
}

void addVelocity( int start, int duration, float speed ) {
	velocities.Append( new idVelocity( start, duration, speed ) );
}

virtual const idVec3 *getPosition( int t ) {
	assert( true );
	return NULL;
}

#ifdef RTCW_VANILLA
virtual void draw( bool editMode ) {};
#endif // RTCW_VANILLA

virtual void parse( const char *( *text ) ) {};
virtual void write( fileHandle_t file, const char *name );
virtual bool parseToken( const char *key, const char *( *text ) );

const char *getName() {
	return name.c_str();
}

void setName( const char *p ) {
	name = p;
}

virtual void startEdit() {      //DAJ added void
	editMode = true;
}

virtual void stopEdit() {       //DAJ added void
	editMode = false;
}

#ifdef RTCW_VANILLA
virtual void draw() {};
#endif // RTCW_VANILLA

const char *typeStr() {
	return positionStr[static_cast<int>( type )];
}

void calcVelocity( float distance ) {
	if ( time ) {   //DAJ BUGFIX
		float secs = (float)time / 1000;
		baseVelocity = distance / secs;
	}
}

protected:
static const char* positionStr[POSITION_COUNT];
int startTime;
int time;
idCameraPosition::positionType type;
idStr name;
bool editMode;
idList<idVelocity*> velocities;
float baseVelocity;
};

class idFixedPosition : public idCameraPosition {
public:

void init() {
	pos.Zero();
	type = idCameraPosition::FIXED;
}

idFixedPosition() : idCameraPosition() {
	init();
}

idFixedPosition( idVec3 p ) : idCameraPosition() {
	init();
	pos = p;
}

virtual void addPoint( const idVec3 &v ) {
	pos = v;
}

virtual void addPoint( const float x, const float y, const float z ) {
	pos.set( x, y, z );
}


~idFixedPosition() {
}
virtual const idVec3 *getPosition( int t ) {
	return &pos;
}

void parse( const char *( *text ) );
void write( fileHandle_t file, const char *name );

virtual int numPoints() {
	return 1;
}

virtual idVec3 *getPoint( int index ) {
	if ( index != 0 ) {
		assert( true );
	}
	;
	return &pos;
}

#ifdef RTCW_VANILLA
virtual void draw( bool editMode ) {
	glLabeledPoint( blue, pos, ( editMode ) ? 5 : 3, "Fixed point" );
}
#endif // RTCW_VANILLA

protected:
idVec3 pos;
};

class idInterpolatedPosition : public idCameraPosition {
public:

void init() {
	type = idCameraPosition::INTERPOLATED;
	first = true;
	startPos.Zero();
	endPos.Zero();
}

idInterpolatedPosition() : idCameraPosition() {
	init();
}

idInterpolatedPosition( idVec3 start, idVec3 end, int time ) : idCameraPosition( time ) {
	init();
	startPos = start;
	endPos = end;
}

~idInterpolatedPosition() {
}

virtual const idVec3 *getPosition( int t );

void parse( const char *( *text ) );
void write( fileHandle_t file, const char *name );

virtual int numPoints() {
	return 2;
}

virtual idVec3 *getPoint( int index ) {
	assert( index >= 0 && index < 2 );
	if ( index == 0 ) {
		return &startPos;
	}
	return &endPos;
}

virtual void addPoint( const float x, const float y, const float z ) {
	if ( first ) {
		startPos.set( x, y, z );
		first = false;
	} else {
		endPos.set( x, y, z );
		first = true;
	}
}

virtual void addPoint( const idVec3 &v ) {
	if ( first ) {
		startPos = v;
		first = false;
	} else {
		endPos = v;
		first = true;
	}
}

#ifdef RTCW_VANILLA
virtual void draw( bool editMode ) {
	glLabeledPoint( blue, startPos, ( editMode ) ? 5 : 3, "Start interpolated" );
	glLabeledPoint( blue, endPos, ( editMode ) ? 5 : 3, "End interpolated" );
	glBegin( GL_LINES );
	glVertex3fv( startPos );
	glVertex3fv( endPos );
	glEnd();
}
#endif // RTCW_VANILLA

virtual void start( int t ) {
	idCameraPosition::start( t );
	lastTime = startTime;
	distSoFar = 0.0;
	idVec3 temp = startPos;
	temp -= endPos;
	calcVelocity( temp.Length() );
}

protected:
bool first;
idVec3 startPos;
idVec3 endPos;
int lastTime;
float distSoFar;
};

class idSplinePosition : public idCameraPosition {
public:

void init() {
	type = idCameraPosition::SPLINE;
}

idSplinePosition() : idCameraPosition() {
	init();
}

idSplinePosition( int time ) : idCameraPosition( time ) {
	init();
}

~idSplinePosition() {
}

virtual void start( int t ) {
	idCameraPosition::start( t );
	target.initPosition( t, time );
	lastTime = startTime;
	distSoFar = 0.0;
	calcVelocity( target.totalDistance() );
}

#if !defined RTCW_SP
//virtual const idVec3 *getPosition(long t) {
//	return target.getPosition(t);
//}
#endif // RTCW_XX

virtual const idVec3 *getPosition( int t );

#if !defined RTCW_SP
//virtual const idVec3 *getPosition(long t) const {
#endif // RTCW_XX

void addControlPoint( idVec3 &v ) {
	target.addPoint( v );
}

void parse( const char *( *text ) );
void write( fileHandle_t file, const char *name );

virtual int numPoints() {
	return target.numPoints();
}

virtual idVec3 *getPoint( int index ) {
	return target.getPoint( index );
}

virtual void addPoint( const idVec3 &v ) {
	target.addPoint( v );
}

virtual void addPoint( const float x, const float y, const float z ) {
	target.addPoint( x, y, z );
}

#ifdef RTCW_VANILLA
virtual void draw( bool editMode ) {
	target.draw( editMode );
}
#endif // RTCW_VANILLA

virtual void updateSelection( const idVec3 &move ) {
	idCameraPosition::updateSelection( move );
	target.buildSpline();
}

protected:
idSplineList target;
int lastTime;
float distSoFar;
};

class idCameraFOV {
public:

idCameraFOV() {
	time = 0;
	length = 0;
	fov = 90;
}

idCameraFOV( int v ) {
	time = 0;
	length = 0;
	fov = v;
}

idCameraFOV( int s, int e, int t ) {
	startFOV = s;
	endFOV = e;
	length = t;
}


~idCameraFOV() {
}

void setFOV( float f ) {
	fov = f;
}

float getFOV( int t ) {
	if ( length ) {
		float percent = ( t - startTime ) / length;
		if ( percent < 0.0 ) {
			percent = 0.0;
		} else if ( percent > 1.0 ) {
			percent = 1.0;
		}
		float temp = endFOV - startFOV;
		temp *= percent;
		fov = startFOV + temp;

		if ( percent == 1.0 ) {
			length = 0.0;
		}
	}
	return fov;
}

void start( int t ) {
	startTime = t;
}

void reset( float startfov, float endfov, int start, float len ) {
	startFOV = startfov;
	endFOV = endfov;
	startTime = start;
	length = len * 1000;
}

void parse( const char *( *text ) );
void write( fileHandle_t file, const char *name );

protected:
float fov;
float startFOV;
float endFOV;
int startTime;
int time;
float length;
};




class idCameraEvent {

#if !defined RTCW_MP
public:                     // parameters
#else
public:
#endif // RTCW_XX

enum eventType {
	EVENT_NA = 0x00,

#if !defined RTCW_MP
	EVENT_WAIT,             //
	EVENT_TARGETWAIT,       //
	EVENT_SPEED,            //
	EVENT_TARGET,           // char(name)
	EVENT_SNAPTARGET,       //
	EVENT_FOV,              // int(time), int(targetfov)
	EVENT_CMD,              //
	EVENT_TRIGGER,          //
	EVENT_STOP,             //
	EVENT_CAMERA,           //
	EVENT_FADEOUT,          // int(time)
	EVENT_FADEIN,           // int(time)
#else
	EVENT_WAIT,
	EVENT_TARGETWAIT,
	EVENT_SPEED,
	EVENT_TARGET,
	EVENT_SNAPTARGET,
	EVENT_FOV,
	EVENT_CMD,
	EVENT_TRIGGER,
	EVENT_STOP,
	EVENT_CAMERA,
	EVENT_FADEOUT,
	EVENT_FADEIN,
#endif // RTCW_XX

#if !defined RTCW_MP
	EVENT_FEATHER,          //
#endif // RTCW_XX

	EVENT_COUNT
};

static const char* eventStr[EVENT_COUNT];

idCameraEvent() {
	paramStr = "";
	type = EVENT_NA;
	time = 0;
}

idCameraEvent( eventType t, const char *param, int n ) {
	type = t;
	paramStr = param;
	time = n;
}

~idCameraEvent() {
};

eventType getType() {
	return type;
}

const char *typeStr() {
	return eventStr[static_cast<int>( type )];
}

const char *getParam() {
	return paramStr.c_str();
}

int getTime() {
	return time;
}

void setTime( int n ) {
	time = n;
}

void parse( const char *( *text ) );
void write( fileHandle_t file, const char *name );

void setTriggered( bool b ) {
	triggered = b;
}

bool getTriggered() {
	return triggered;
}

protected:
eventType type;
idStr paramStr;
int time;
bool triggered;

};

class idCameraDef {
public:

void clear() {
	currentCameraPosition = 0;
	cameraRunning = false;
	lastDirection.Zero();
	baseTime = 30;
	activeTarget = 0;
	name = "camera01";
	fov.setFOV( 90 );
	int i;
	for ( i = 0; i < targetPositions.Num(); i++ ) {
		delete targetPositions[i];
	}
	for ( i = 0; i < events.Num(); i++ ) {
		delete events[i];
	}
	delete cameraPosition;
	cameraPosition = NULL;
	events.Clear();
	targetPositions.Clear();
}

idCameraPosition *startNewCamera( idCameraPosition::positionType type ) {
	clear();
	if ( type == idCameraPosition::SPLINE ) {
		cameraPosition = new idSplinePosition();
	} else if ( type == idCameraPosition::INTERPOLATED ) {
		cameraPosition = new idInterpolatedPosition();
	} else {
		cameraPosition = new idFixedPosition();
	}
	return cameraPosition;
}

idCameraDef() {
	cameraPosition = NULL;
	clear();
}

~idCameraDef() {
	clear();
}

void addEvent( idCameraEvent::eventType t, const char *param, int time );

void addEvent( idCameraEvent *event );

static int sortEvents( const void *p1, const void *p2 );

int numEvents() {
	return events.Num();
}

idCameraEvent *getEvent( int index ) {
	assert( index >= 0 && index < events.Num() );
	return events[index];
}

void parse( const char *( *text ) );
bool load( const char *filename );
void save( const char *filename );

void buildCamera();

#if !defined RTCW_SP
//idSplineList *getcameraPosition() {
//	return &cameraPosition;
//}
#endif // RTCW_XX

static idCameraPosition *newFromType( idCameraPosition::positionType t ) {
	switch ( t ) {
	case idCameraPosition::FIXED: return new idFixedPosition();
	case idCameraPosition::INTERPOLATED: return new idInterpolatedPosition();
	case idCameraPosition::SPLINE: return new idSplinePosition();
	default:
		break;
	};
	return NULL;
}

void addTarget( const char *name, idCameraPosition::positionType type );

idCameraPosition *getActiveTarget() {
	if ( targetPositions.Num() == 0 ) {
		addTarget( NULL, idCameraPosition::FIXED );
	}
	return targetPositions[activeTarget];
}

idCameraPosition *getActiveTarget( int index ) {
	if ( targetPositions.Num() == 0 ) {
		addTarget( NULL, idCameraPosition::FIXED );
		return targetPositions[0];
	}
	return targetPositions[index];
}

int numTargets() {
	return targetPositions.Num();
}


void setActiveTargetByName( const char *name ) {
	for ( int i = 0; i < targetPositions.Num(); i++ ) {
		if ( Q_stricmp( name, targetPositions[i]->getName() ) == 0 ) {
			setActiveTarget( i );
			return;
		}
	}
}

void setActiveTarget( int index ) {
	assert( index >= 0 && index < targetPositions.Num() );
	activeTarget = index;
}

void setRunning( bool b ) {
	cameraRunning = b;
}

void setBaseTime( float f ) {
	baseTime = f;
}

float getBaseTime() {
	return baseTime;
}

float getTotalTime() {
	return totalTime;
}

void startCamera( int t );
void stopCamera() {
	cameraRunning = true;
}
void getActiveSegmentInfo( int segment, idVec3 &origin, idVec3 &direction, float *fv );

bool getCameraInfo( int time, idVec3 &origin, idVec3 &direction, float *fv );
qboolean getCameraInfo( int camNum, int time, float *origin, float *angles, float *fov );
bool getCameraInfo( int time, float *origin, float *direction, float *fv ) {
	idVec3 org, dir;
	org[0] = origin[0];
	org[1] = origin[1];
	org[2] = origin[2];
	dir[0] = direction[0];
	dir[1] = direction[1];
	dir[2] = direction[2];
	bool b = getCameraInfo( time, org, dir, fv );
	origin[0] = org[0];
	origin[1] = org[1];
	origin[2] = org[2];
	direction[0] = dir[0];
	direction[1] = dir[1];
	direction[2] = dir[2];
	return b;
}

#ifdef RTCW_VANILLA
void draw( bool editMode ) {
	// gcc doesn't allow casting away from bools
	// why?  I've no idea...
	if ( cameraPosition ) {
		cameraPosition->draw( (bool)( ( editMode || cameraRunning ) && cameraEdit ) );
		int count = targetPositions.Num();
		for ( int i = 0; i < count; i++ ) {
			targetPositions[i]->draw( (bool)( ( editMode || cameraRunning ) && i == activeTarget && !cameraEdit ) );
		}
	}
}
#endif // RTCW_VANILLA

#if !defined RTCW_SP
/*
	int numSegments() {
		if (cameraEdit) {
			return cameraPosition.numSegments();
		}
		return getTargetSpline()->numSegments();
	}

	int getActiveSegment() {
		if (cameraEdit) {
			return cameraPosition.getActiveSegment();
		}
		return getTargetSpline()->getActiveSegment();
	}

	void setActiveSegment(int i) {
		if (cameraEdit) {
			cameraPosition.setActiveSegment(i);
		} else {
			getTargetSpline()->setActiveSegment(i);
		}
	}
*/
#endif // RTCW_XX

int numPoints() {
	if ( cameraEdit ) {
		return cameraPosition->numPoints();
	}
	return getActiveTarget()->numPoints();
}

const idVec3 *getPoint( int index ) {
	if ( cameraEdit ) {
		return cameraPosition->getPoint( index );
	}
	return getActiveTarget()->getPoint( index );
}

void stopEdit() {
	editMode = false;
	if ( cameraEdit ) {
		cameraPosition->stopEdit();
	} else {
		getActiveTarget()->stopEdit();
	}
}

void startEdit( bool camera ) {
	cameraEdit = camera;
	if ( camera ) {
		cameraPosition->startEdit();
		for ( int i = 0; i < targetPositions.Num(); i++ ) {
			targetPositions[i]->stopEdit();
		}
	} else {
		getActiveTarget()->startEdit();
		cameraPosition->stopEdit();
	}
	editMode = true;
}

bool waitEvent( int index );

const char *getName() {
	return name.c_str();
}

void setName( const char *p ) {
	name = p;
}

idCameraPosition *getPositionObj() {
	if ( cameraPosition == NULL ) {
		cameraPosition = new idFixedPosition();
	}
	return cameraPosition;
}

protected:
idStr name;
int currentCameraPosition;
idVec3 lastDirection;
bool cameraRunning;
idCameraPosition *cameraPosition;
idList<idCameraPosition*> targetPositions;
idList<idCameraEvent*> events;
idCameraFOV fov;
int activeTarget;
float totalTime;
float baseTime;
int startTime;

bool cameraEdit;
bool editMode;
};

extern bool g_splineMode;

extern idCameraDef *g_splineList;


#endif
