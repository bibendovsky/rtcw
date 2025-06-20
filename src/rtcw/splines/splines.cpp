/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#if defined RTCW_SP
#include "q_splineshared.h"
#else
//#include "stdafx.h"
//#include "qe3.h"

#include "q_splineshared.h"  //DAJ was q_shared.h conflicted with qcommon
#endif // RTCW_XX

#include "splines.h"

//extern "C" {
int FS_Write( const void *buffer, int len, fileHandle_t h );
int FS_ReadFile( const char *qpath, void **buffer );
void FS_FreeFile( void *buffer );
fileHandle_t FS_FOpenFileWrite( const char *filename );
void FS_FCloseFile( fileHandle_t f );
void Cbuf_AddText( const char *text );
void Cbuf_Execute( void );
//}

//float Q_fabs( float f ) {
//	int tmp = *( int * ) &f;
//	tmp &= 0x7FFFFFFF;
//	return *( float * ) &tmp;
//}

// (SA) making a list of cameras so I can use
//		the splines as targets for other things.
//		Certainly better ways to do this, but this lets
//		me get underway quickly with ents that need spline
//		targets.
#define MAX_CAMERAS 64

idCameraDef camera[MAX_CAMERAS];

//extern "C" {
qboolean loadCamera( int camNum, const char *name ) {
	if ( camNum < 0 || camNum >= MAX_CAMERAS ) {
		return qfalse;
	}
	camera[camNum].clear();
	// TTimo static_cast confused gcc, went for C-style casting
	return (qboolean)( camera[camNum].load( name ) );
}

qboolean getCameraInfo( int camNum, int time, float *origin, float *angles, float *fov ) {
	idVec3 dir, org;
	if ( camNum < 0 || camNum >= MAX_CAMERAS ) {
		return qfalse;
	}
	org[0] = origin[0];
	org[1] = origin[1];
	org[2] = origin[2];
	if ( camera[camNum].getCameraInfo( time, org, dir, fov ) ) {
		origin[0] = org[0];
		origin[1] = org[1];
		origin[2] = org[2];
		angles[1] = atan2( dir[1], dir[0] ) * 180 / 3.14159;
		angles[0] = asin( dir[2] ) * 180 / 3.14159;
		return qtrue;
	}
	return qfalse;
}

void startCamera( int camNum, int time ) {
	if ( camNum < 0 || camNum >= MAX_CAMERAS ) {
		return;
	}
	camera[camNum].startCamera( time );
}

//}


//#include "../shared/windings.h"
//#include "qcommon.h"
//#include "../sys/sys_public.h"
//#include "game_entity.h"

idCameraDef splineList;
idCameraDef *g_splineList = &splineList;

idVec3 idSplineList::zero( 0,0,0 );

#ifdef RTCW_VANILLA
void glLabeledPoint( idVec3 &color, idVec3 &point, float size, const char *label ) {
	glColor3fv( color );
	glPointSize( size );
	glBegin( GL_POINTS );
	glVertex3fv( point );
	glEnd();
	idVec3 v = point;
	v.x += 1;
	v.y += 1;
	v.z += 1;
	glRasterPos3fv( v );
	glCallLists( strlen( label ), GL_UNSIGNED_BYTE, label );
}

void glBox( idVec3 &color, idVec3 &point, float size ) {
	idVec3 mins( point );
	idVec3 maxs( point );
	mins[0] -= size;
	mins[1] += size;
	mins[2] -= size;
	maxs[0] += size;
	maxs[1] -= size;
	maxs[2] += size;
	glColor3fv( color );
	glBegin( GL_LINE_LOOP );
	glVertex3f( mins[0],mins[1],mins[2] );
	glVertex3f( maxs[0],mins[1],mins[2] );
	glVertex3f( maxs[0],maxs[1],mins[2] );
	glVertex3f( mins[0],maxs[1],mins[2] );
	glEnd();
	glBegin( GL_LINE_LOOP );
	glVertex3f( mins[0],mins[1],maxs[2] );
	glVertex3f( maxs[0],mins[1],maxs[2] );
	glVertex3f( maxs[0],maxs[1],maxs[2] );
	glVertex3f( mins[0],maxs[1],maxs[2] );
	glEnd();

	glBegin( GL_LINES );
	glVertex3f( mins[0],mins[1],mins[2] );
	glVertex3f( mins[0],mins[1],maxs[2] );
	glVertex3f( mins[0],maxs[1],maxs[2] );
	glVertex3f( mins[0],maxs[1],mins[2] );
	glVertex3f( maxs[0],mins[1],mins[2] );
	glVertex3f( maxs[0],mins[1],maxs[2] );
	glVertex3f( maxs[0],maxs[1],maxs[2] );
	glVertex3f( maxs[0],maxs[1],mins[2] );
	glEnd();

}

void splineTest() {
	//g_splineList->load("p:/doom/base/maps/test_base1.camera");
}

void splineDraw() {
	//g_splineList->addToRenderer();
}

//extern void D_DebugLine( const idVec3 &color, const idVec3 &start, const idVec3 &end );

void debugLine( idVec3 &color, float x, float y, float z, float x2, float y2, float z2 ) {
//	idVec3 from(x, y, z);
//	idVec3 to(x2, y2, z2);
	//D_DebugLine(color, from, to);
}

void idSplineList::addToRenderer() {

	if ( controlPoints.Num() == 0 ) {
		return;
	}

	idVec3 mins, maxs;
	idVec3 yellow( 1.0, 1.0, 0 );
	idVec3 white( 1.0, 1.0, 1.0 );
	int i;

	for ( i = 0; i < controlPoints.Num(); i++ ) {
		VectorCopy( *controlPoints[i], mins );
		VectorCopy( mins, maxs );
		mins[0] -= 8;
		mins[1] += 8;
		mins[2] -= 8;
		maxs[0] += 8;
		maxs[1] -= 8;
		maxs[2] += 8;
		debugLine( yellow, mins[0], mins[1], mins[2], maxs[0], mins[1], mins[2] );
		debugLine( yellow, maxs[0], mins[1], mins[2], maxs[0], maxs[1], mins[2] );
		debugLine( yellow, maxs[0], maxs[1], mins[2], mins[0], maxs[1], mins[2] );
		debugLine( yellow, mins[0], maxs[1], mins[2], mins[0], mins[1], mins[2] );

		debugLine( yellow, mins[0], mins[1], maxs[2], maxs[0], mins[1], maxs[2] );
		debugLine( yellow, maxs[0], mins[1], maxs[2], maxs[0], maxs[1], maxs[2] );
		debugLine( yellow, maxs[0], maxs[1], maxs[2], mins[0], maxs[1], maxs[2] );
		debugLine( yellow, mins[0], maxs[1], maxs[2], mins[0], mins[1], maxs[2] );

	}

	int step = 0;
	idVec3 step1;
	for ( i = 3; i < controlPoints.Num(); i++ ) {
		for ( float tension = 0.0f; tension < 1.001f; tension += 0.1f ) {
			float x = 0;
			float y = 0;
			float z = 0;
			for ( int j = 0; j < 4; j++ ) {
				x += controlPoints[i - ( 3 - j )]->x * calcSpline( j, tension );
				y += controlPoints[i - ( 3 - j )]->y * calcSpline( j, tension );
				z += controlPoints[i - ( 3 - j )]->z * calcSpline( j, tension );
			}
			if ( step == 0 ) {
				step1[0] = x;
				step1[1] = y;
				step1[2] = z;
				step = 1;
			} else {
				debugLine( white, step1[0], step1[1], step1[2], x, y, z );
				step = 0;
			}

		}
	}
}
#endif // RTCW_VANILLA

void idSplineList::buildSpline() {
	//int start = Sys_Milliseconds();
	clearSpline();
	for ( int i = 3; i < controlPoints.Num(); i++ ) {
		for ( float tension = 0.0f; tension < 1.001f; tension += granularity ) {
			float x = 0;
			float y = 0;
			float z = 0;
			for ( int j = 0; j < 4; j++ ) {
				x += controlPoints[i - ( 3 - j )]->x * calcSpline( j, tension );
				y += controlPoints[i - ( 3 - j )]->y * calcSpline( j, tension );
				z += controlPoints[i - ( 3 - j )]->z * calcSpline( j, tension );
			}
			splinePoints.Append( new idVec3( x, y, z ) );
		}
	}
	dirty = false;
	//Com_Printf("Spline build took %f seconds\n", (float)(Sys_Milliseconds() - start) / 1000);
}

#ifdef RTCW_VANILLA
void idSplineList::draw( bool editMode ) {
	int i;
	idVec4 yellow( 1, 1, 0, 1 );

	if ( controlPoints.Num() == 0 ) {
		return;
	}

	if ( dirty ) {
		buildSpline();
	}


	glColor3fv( controlColor );
	glPointSize( 5 );

	glBegin( GL_POINTS );
	for ( i = 0; i < controlPoints.Num(); i++ ) {
		glVertex3fv( *controlPoints[i] );
	}
	glEnd();

	if ( editMode ) {
		for ( i = 0; i < controlPoints.Num(); i++ ) {
			glBox( activeColor, *controlPoints[i], 4 );
		}
	}

	//Draw the curve
	glColor3fv( pathColor );
	glBegin( GL_LINE_STRIP );
	int count = splinePoints.Num();
	for ( i = 0; i < count; i++ ) {
		glVertex3fv( *splinePoints[i] );
	}
	glEnd();

	if ( editMode ) {
		glColor3fv( segmentColor );
		glPointSize( 3 );
		glBegin( GL_POINTS );
		for ( i = 0; i < count; i++ ) {
			glVertex3fv( *splinePoints[i] );
		}
		glEnd();
	}
	if ( count > 0 ) {
		//assert(activeSegment >=0 && activeSegment < count);
		if ( activeSegment >= 0 && activeSegment < count ) {
			glBox( activeColor, *splinePoints[activeSegment], 6 );
			glBox( yellow, *splinePoints[activeSegment], 8 );
		}
	}

}
#endif // RTCW_VANILLA

float idSplineList::totalDistance() {

	// FIXME: save dist and return
	//
	if ( controlPoints.Num() == 0 ) {
		return 0.0;
	}

	if ( dirty ) {
		buildSpline();
	}

	float dist = 0.0;
	idVec3 temp;
	int count = splinePoints.Num();
	for ( int i = 1; i < count; i++ ) {
		temp = *splinePoints[i - 1];
		temp -= *splinePoints[i];
		dist += temp.Length();
	}
	return dist;
}

void idSplineList::initPosition( int bt, int totalTime ) {

	if ( dirty ) {
		buildSpline();
	}

	if ( splinePoints.Num() == 0 ) {
		return;
	}

	baseTime = bt;
	time = totalTime;

	// calc distance to travel ( this will soon be broken into time segments )
	splineTime.Clear();
	splineTime.Append( bt );
	double dist = totalDistance();
	double distSoFar = 0.0;
	idVec3 temp;
	int count = splinePoints.Num();
	//for(int i = 2; i < count - 1; i++) {
	for ( int i = 1; i < count; i++ ) {
		temp = *splinePoints[i - 1];
		temp -= *splinePoints[i];
		distSoFar += temp.Length();
		double percent = distSoFar / dist;
		percent *= totalTime;
		splineTime.Append( percent + bt );
	}
	assert( splineTime.Num() == splinePoints.Num() );
	activeSegment = 0;
}



float idSplineList::calcSpline( int step, float tension ) {
	switch ( step ) {
	case 0: return ( pow( 1 - tension, 3 ) ) / 6;
	case 1: return ( 3 * pow( tension, 3 ) - 6 * pow( tension, 2 ) + 4 ) / 6;
	case 2: return ( -3 * pow( tension, 3 ) + 3 * pow( tension, 2 ) + 3 * tension + 1 ) / 6;
	case 3: return pow( tension, 3 ) / 6;
	}
	return 0.0;
}



void idSplineList::updateSelection( const idVec3 &move ) {
	if ( selected ) {
		dirty = true;
		VectorAdd( *selected, move, *selected );
	}
}


void idSplineList::setSelectedPoint( idVec3 *p ) {
	if ( p ) {
		p->Snap();
		for ( int i = 0; i < controlPoints.Num(); i++ ) {
			if ( *p == *controlPoints[i] ) {
				selected = controlPoints[i];
			}
		}
	} else {
		selected = NULL;
	}
}

const idVec3 *idSplineList::getPosition( int t ) {
	static idVec3 interpolatedPos;
//	static long lastTime = -1; // TTimo unused

	int count = splineTime.Num();
	if ( count == 0 ) {
		return &zero;
	}

//	Com_Printf("Time: %d\n", t);
	assert( splineTime.Num() == splinePoints.Num() );

	while ( activeSegment < count ) {
		if ( splineTime[activeSegment] >= t ) {
			if ( activeSegment > 0 && activeSegment < count - 1 ) {
				double timeHi = splineTime[activeSegment + 1];
				double timeLo = splineTime[activeSegment - 1];
				double percent = ( timeHi - t ) / ( timeHi - timeLo );
				// pick two bounding points
				idVec3 v1 = *splinePoints[activeSegment - 1];
				idVec3 v2 = *splinePoints[activeSegment + 1];
				v2 *= ( 1.0 - percent );
				v1 *= percent;
				v2 += v1;
				interpolatedPos = v2;
				return &interpolatedPos;
			}
			return splinePoints[activeSegment];
		} else {
			activeSegment++;
		}
	}
	return splinePoints[count - 1];
}

void idSplineList::parse( const char *( *text )  ) {
	const char *token;
	//Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );

		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp( token, "}" ) ) {
			break;
		}

		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !Q_stricmp( token, "(" ) || !Q_stricmp( token, "}" ) ) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine( text );
			const char *token = Com_Parse( text );
			if ( Q_stricmp( key.c_str(), "granularity" ) == 0 ) {
				granularity = atof( token );
			} else if ( Q_stricmp( key.c_str(), "name" ) == 0 ) {
				name = token;
			}
			token = Com_Parse( text );

		} while ( 1 );

		if ( !Q_stricmp( token, "}" ) ) {
			break;
		}

		Com_UngetToken();
		// read the control point
		idVec3 point;
		Com_Parse1DMatrix( text, 3, point );
		addPoint( point.x, point.y, point.z );
	} while ( 1 );

	//Com_UngetToken();
	//Com_MatchToken( text, "}" );
	dirty = true;
}

void idSplineList::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	//s = va("\t\tname %s\n", name.c_str());
	//FS_Write(s.c_str(), s.length(), file);
	s = va( "\t\t\tgranularity %f\n", granularity );
	FS_Write( s.c_str(), s.length(), file );
	int count = controlPoints.Num();
	for ( int i = 0; i < count; i++ ) {
		s = va( "\t\t\t( %f %f %f )\n", controlPoints[i]->x, controlPoints[i]->y, controlPoints[i]->z );
		FS_Write( s.c_str(), s.length(), file );
	}
	s = "\t\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}


void idCameraDef::getActiveSegmentInfo( int segment, idVec3 &origin, idVec3 &direction, float *fov ) {
#if 0
	if ( !cameraSpline.validTime() ) {
		buildCamera();
	}
	double d = (double)segment / numSegments();
	getCameraInfo( d * totalTime * 1000, origin, direction, fov );
#endif
/*
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	origin = *cameraSpline.getSegmentPoint(segment);


	idVec3 temp;

	int numTargets = getTargetSpline()->controlPoints.Num();
	int count = cameraSpline.splineTime.Num();
	if (numTargets == 0) {
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
		}
	} else if (numTargets == 1) {
		temp = *getTargetSpline()->controlPoints[0];
	} else {
		temp = *getTargetSpline()->getSegmentPoint(segment);
	}

	temp -= origin;
	temp.Normalize();
	direction = temp;
*/
}

bool idCameraDef::getCameraInfo( int time, idVec3 &origin, idVec3 &direction, float *fv ) {

	char buff[1024];

	if ( ( time - startTime ) / 1000 > totalTime ) {
		return false;
	}


	for ( int i = 0; i < events.Num(); i++ ) {
		if ( time >= startTime + events[i]->getTime() && !events[i]->getTriggered() ) {
			events[i]->setTriggered( true );
			if ( events[i]->getType() == idCameraEvent::EVENT_TARGET ) {
				setActiveTargetByName( events[i]->getParam() );
				getActiveTarget()->start( startTime + events[i]->getTime() );
				//Com_Printf("Triggered event switch to target: %s\n",events[i]->getParam());
			} else if ( events[i]->getType() == idCameraEvent::EVENT_TRIGGER ) {

#if defined RTCW_SP
				// empty!
#else
				//idEntity *ent = NULL;
				//ent = level.FindTarget( ent, events[i]->getParam());
				//if (ent) {
				//	ent->signal( SIG_TRIGGER );
				//	ent->ProcessEvent( &EV_Activate, world );
				//}
#endif // RTCW_XX

			} else if ( events[i]->getType() == idCameraEvent::EVENT_FOV ) {
				memset( buff, 0, sizeof( buff ) );
				strcpy( buff, events[i]->getParam() );
				const char *param1 = strtok( buff, " \t,\0" );
				const char *param2 = strtok( NULL, " \t,\0" );
				float len = ( param2 ) ? atof( param2 ) : 0;
				float newfov = ( param1 ) ? atof( param1 ) : 90;
				fov.reset( fov.getFOV( time ), newfov, time, len );
				//*fv = fov = atof(events[i]->getParam());
			} else if ( events[i]->getType() == idCameraEvent::EVENT_FADEIN ) {
				float time = atof( events[i]->getParam() );
				Cbuf_AddText( va( "fade 0 0 0 0 %f", time ) );
				Cbuf_Execute();
			} else if ( events[i]->getType() == idCameraEvent::EVENT_FADEOUT ) {
				float time = atof( events[i]->getParam() );
				Cbuf_AddText( va( "fade 0 0 0 255 %f", time ) );
				Cbuf_Execute();
			} else if ( events[i]->getType() == idCameraEvent::EVENT_CAMERA ) {
				memset( buff, 0, sizeof( buff ) );
				strcpy( buff, events[i]->getParam() );
				const char *param1 = strtok( buff, " \t,\0" );
				const char *param2 = strtok( NULL, " \t,\0" );

				if ( param2 ) {
					loadCamera( atoi( param1 ), va( "cameras/%s.camera", param2 ) );
					startCamera( time );
				} else {
					loadCamera( 0, va( "cameras/%s.camera", events[i]->getParam() ) );
					startCamera( time );
				}
				return true;
			} else if ( events[i]->getType() == idCameraEvent::EVENT_STOP ) {
				return false;
			}
		}
	}

	origin = *cameraPosition->getPosition( time );

// BBi
//#if !defined RTCW_MP
//	CHECK_NAN_VEC( origin );
//#endif // RTCW_XX
// BBi

	*fv = fov.getFOV( time );

	idVec3 temp = origin;

	int numTargets = targetPositions.Num();
	if ( numTargets == 0 ) {

#if defined RTCW_SP
		// empty!
#else
/*
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
			if (temp == origin) {
				int index = cameraSpline.getActiveSegment() + 2;
				while (temp == origin && index < count - 1) {
					temp = *cameraSpline.splinePoints[index++];
				}
			}
		}
*/
#endif // RTCW_XX

	} else {
		temp = *getActiveTarget()->getPosition( time );
	}

	temp -= origin;
	temp.Normalize();
	direction = temp;

	return true;
}

bool idCameraDef::waitEvent( int index ) {
	//for (int i = 0; i < events.Num(); i++) {
	//	if (events[i]->getSegment() == index && events[i]->getType() == idCameraEvent::EVENT_WAIT) {
	//		return true;
	//	}
	//}
	return false;
}


#define NUM_CCELERATION_SEGS 10
#define CELL_AMT 5

void idCameraDef::buildCamera() {
	int i;
	//int lastSwitch = 0; // TTimo: unused
	idList<float> waits;
	idList<int> targets;

	totalTime = baseTime;

#if !defined RTCW_ET
	cameraPosition->setTime( totalTime * 1000 );
#else
	cameraPosition->setTime( (int)( totalTime * 1000 ) );
#endif // RTCW_XX

	// we have a base time layout for the path and the target path
	// now we need to layer on any wait or speed changes
	for ( i = 0; i < events.Num(); i++ ) {
		//idCameraEvent *ev = events[i]; // TTimo: unused
		events[i]->setTriggered( false );
		switch ( events[i]->getType() ) {
		case idCameraEvent::EVENT_TARGET: {
			targets.Append( i );
			break;
		}

#if !defined RTCW_MP
		case idCameraEvent::EVENT_FEATHER: {
			int startTime = 0;
			float speed = 0;
			int loopTime = 10;
			float stepGoal = cameraPosition->getBaseVelocity() / ( 1000 / loopTime );
			while ( startTime <= 1000 ) {
				cameraPosition->addVelocity( startTime, loopTime, speed );
				speed += stepGoal;
				if ( speed > cameraPosition->getBaseVelocity() ) {
					speed = cameraPosition->getBaseVelocity();
				}
				startTime += loopTime;
			}

			// TTimo gcc warns: assignment to `long int' from `float'
			// more efficient to do (long int)(totalTime) * 1000 - 1000
			// safer to (long int)(totalTime * 1000 - 1000)
			startTime = ( int )( totalTime * 1000 - 1000 );
			int endTime = startTime + 1000;
			speed = cameraPosition->getBaseVelocity();
			while ( startTime < endTime ) {
				speed -= stepGoal;
				if ( speed < 0 ) {
					speed = 0;
				}
				cameraPosition->addVelocity( startTime, loopTime, speed );
				startTime += loopTime;
			}
			break;

		}
#endif // RTCW_XX

		case idCameraEvent::EVENT_WAIT: {
			waits.Append( atof( events[i]->getParam() ) );

			//FIXME: this is quite hacky for Wolf E3, accel and decel needs
			// do be parameter based etc..
			int startTime = events[i]->getTime() - 1000;
			if ( startTime < 0 ) {
				startTime = 0;
			}
			float speed = cameraPosition->getBaseVelocity();
			int loopTime = 10;
			float steps = speed / ( ( events[i]->getTime() - startTime ) / loopTime );
			while ( startTime <= events[i]->getTime() - loopTime ) {
				cameraPosition->addVelocity( startTime, loopTime, speed );
				speed -= steps;
				startTime += loopTime;
			}

#if !defined RTCW_ET
			cameraPosition->addVelocity( events[i]->getTime(), atof( events[i]->getParam() ) * 1000, 0 );
#else
			cameraPosition->addVelocity( events[i]->getTime(), (int)( atof( events[i]->getParam() ) * 1000 ), 0 );
#endif // RTCW_XX

			startTime = ( int )( events[i]->getTime() + atof( events[i]->getParam() ) * 1000 );
			int endTime = startTime + 1000;
			speed = 0;
			while ( startTime <= endTime ) {
				cameraPosition->addVelocity( startTime, loopTime, speed );
				speed += steps;
				startTime += loopTime;
			}
			break;
		}
		case idCameraEvent::EVENT_TARGETWAIT: {
			//targetWaits.Append(i);
			break;
		}
		case idCameraEvent::EVENT_SPEED: {
/*
				// take the average delay between up to the next five segments
				float adjust = atof(events[i]->getParam());
				int index = events[i]->getSegment();
				total = 0;
				count = 0;

				// get total amount of time over the remainder of the segment
				for (j = index; j < cameraSpline.numSegments() - 1; j++) {
					total += cameraSpline.getSegmentTime(j + 1) - cameraSpline.getSegmentTime(j);
					count++;
				}

				// multiply that by the adjustment
				double newTotal = total * adjust;
				// what is the difference..
				newTotal -= total;
				totalTime += newTotal / 1000;

				// per segment difference
				newTotal /= count;
				int additive = newTotal;

				// now propogate that difference out to each segment
				for (j = index; j < cameraSpline.numSegments(); j++) {
					cameraSpline.addSegmentTime(j, additive);
					additive += newTotal;
				}
				break;
*/
		default:
			break;
		}
		}
	}


	for ( i = 0; i < waits.Num(); i++ ) {
		totalTime += waits[i];
	}

	// on a new target switch, we need to take time to this point ( since last target switch )
	// and allocate it across the active target, then reset time to this point
	int timeSoFar = 0;
	int total = ( int )( totalTime * 1000 );
	for ( i = 0; i < targets.Num(); i++ ) {
		int t;
		if ( i < targets.Num() - 1 ) {
			t = events[targets[i + 1]]->getTime();
		} else {
			t = total - timeSoFar;
		}
		// t is how much time to use for this target
		setActiveTargetByName( events[targets[i]]->getParam() );
		getActiveTarget()->setTime( t );
		timeSoFar += t;
	}


}

void idCameraDef::startCamera( int t ) {

#if !defined RTCW_MP
	cameraPosition->clearVelocities();
	cameraPosition->start( t );
#endif // RTCW_XX

	buildCamera();

#if defined RTCW_MP
	cameraPosition->start( t );
#endif // RTCW_XX

	fov.reset( 90, 90, t, 0 );
	//for (int i = 0; i < targetPositions.Num(); i++) {
	//	targetPositions[i]->
	//}
	startTime = t;
	cameraRunning = true;
}


void idCameraDef::parse( const char *( *text )  ) {

	const char  *token;
	do {
		token = Com_Parse( text );

		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp( token, "}" ) ) {
			break;
		}

		if ( Q_stricmp( token, "time" ) == 0 ) {
			baseTime = Com_ParseFloat( text );
		} else if ( Q_stricmp( token, "camera_fixed" ) == 0 )        {
			cameraPosition = new idFixedPosition();
			cameraPosition->parse( text );
		} else if ( Q_stricmp( token, "camera_interpolated" ) == 0 )        {
			cameraPosition = new idInterpolatedPosition();
			cameraPosition->parse( text );
		} else if ( Q_stricmp( token, "camera_spline" ) == 0 )        {
			cameraPosition = new idSplinePosition();
			cameraPosition->parse( text );
		} else if ( Q_stricmp( token, "target_fixed" ) == 0 )        {
			idFixedPosition *pos = new idFixedPosition();
			pos->parse( text );
			targetPositions.Append( pos );
		} else if ( Q_stricmp( token, "target_interpolated" ) == 0 )        {
			idInterpolatedPosition *pos = new idInterpolatedPosition();
			pos->parse( text );
			targetPositions.Append( pos );
		} else if ( Q_stricmp( token, "target_spline" ) == 0 )        {
			idSplinePosition *pos = new idSplinePosition();
			pos->parse( text );
			targetPositions.Append( pos );
		} else if ( Q_stricmp( token, "fov" ) == 0 )        {
			fov.parse( text );
		} else if ( Q_stricmp( token, "event" ) == 0 )        {
			idCameraEvent *event = new idCameraEvent();
			event->parse( text );
			addEvent( event );
		}


	} while ( 1 );

	if ( !cameraPosition ) {
		Com_Printf( "no camera position specified\n" );
		// prevent a crash later on
		cameraPosition = new idFixedPosition();
	}

	Com_UngetToken();
	Com_MatchToken( text, "}" );

}

bool idCameraDef::load( const char *filename ) {
	char *buf;
	const char *buf_p;

#if !defined RTCW_MP
	// TTimo: unused (int length = FS_ReadFile( filename, (void **)&buf );)
#endif // RTCW_XX

	FS_ReadFile( filename, (void **)&buf );
	if ( !buf ) {
		return false;
	}

	clear();
	Com_BeginParseSession( filename );
	buf_p = buf;
	parse( &buf_p );
	Com_EndParseSession();
	FS_FreeFile( buf );

	return true;
}

void idCameraDef::save( const char *filename ) {
	fileHandle_t file = FS_FOpenFileWrite( filename );
	if ( file ) {
		int i;
		idStr s = "cameraPathDef { \n";
		FS_Write( s.c_str(), s.length(), file );
		s = va( "\ttime %f\n", baseTime );
		FS_Write( s.c_str(), s.length(), file );

		cameraPosition->write( file, va( "camera_%s",cameraPosition->typeStr() ) );

		for ( i = 0; i < numTargets(); i++ ) {
			targetPositions[i]->write( file, va( "target_%s", targetPositions[i]->typeStr() ) );
		}

		for ( i = 0; i < events.Num(); i++ ) {
			events[i]->write( file, "event" );
		}

		fov.write( file, "fov" );

		s = "}\n";
		FS_Write( s.c_str(), s.length(), file );
	}
	FS_FCloseFile( file );
}

int idCameraDef::sortEvents( const void *p1, const void *p2 ) {
	idCameraEvent *ev1 = ( idCameraEvent* )( p1 );
	idCameraEvent *ev2 = ( idCameraEvent* )( p2 );

	if ( ev1->getTime() > ev2->getTime() ) {
		return -1;
	}
	if ( ev1->getTime() < ev2->getTime() ) {
		return 1;
	}
	return 0;
}

void idCameraDef::addEvent( idCameraEvent *event ) {
	events.Append( event );
	//events.Sort(&sortEvents);

}
void idCameraDef::addEvent( idCameraEvent::eventType t, const char *param, int time ) {
	addEvent( new idCameraEvent( t, param, time ) );
	buildCamera();
}


const char *idCameraEvent::eventStr[] = {
	"NA",
	"WAIT",
	"TARGETWAIT",
	"SPEED",
	"TARGET",
	"SNAPTARGET",
	"FOV",
	"CMD",
	"TRIGGER",
	"STOP",
	"CAMERA",
	"FADEOUT",

#if !defined RTCW_MP
	"FADEIN",
	"FEATHER"
#else
	"FADEIN"
#endif // RTCW_XX

};

void idCameraEvent::parse( const char *( *text )  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );

		if ( !token[0] ) {
			break;
		}
		if ( !strcmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp( token, "(" ) || !strcmp( token, "}" ) ) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine( text );
			const char *token = Com_Parse( text );
			if ( Q_stricmp( key.c_str(), "type" ) == 0 ) {
				type = static_cast<idCameraEvent::eventType>( atoi( token ) );
			} else if ( Q_stricmp( key.c_str(), "param" ) == 0 ) {
				paramStr = token;
			} else if ( Q_stricmp( key.c_str(), "time" ) == 0 ) {
				time = atoi( token );
			}
			token = Com_Parse( text );

		} while ( 1 );

		if ( !strcmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );

	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

void idCameraEvent::write( fileHandle_t file, const char *name ) {
	idStr s = va( "\t%s {\n", name );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\ttype %d\n", static_cast<int>( type ) );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\tparam \"%s\"\n", paramStr.c_str() );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\ttime %d\n", time );
	FS_Write( s.c_str(), s.length(), file );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}


const char *idCameraPosition::positionStr[] = {
	"Fixed",
	"Interpolated",
	"Spline",
};



const idVec3 *idInterpolatedPosition::getPosition( int t ) {
	static idVec3 interpolatedPos;

#if !defined RTCW_MP
	float percent = 0.0;
#endif // RTCW_XX

	float velocity = getVelocity( t );
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds
	timePassed /= 1000;

	float distToTravel = timePassed * velocity;

	idVec3 temp = startPos;
	temp -= endPos;
	float distance = temp.Length();

	distSoFar += distToTravel;

#if defined RTCW_MP
	float percent = (float)( distSoFar ) / distance;
#endif // RTCW_XX

#if !defined RTCW_MP
	// TTimo
	// show_bug.cgi?id=409
	// avoid NaN on fixed cameras
	if ( distance != 0.0 ) {   //DAJ added to protect DBZ
		percent = (float)( distSoFar ) / distance;
	}
#endif // RTCW_XX

	if ( percent > 1.0 ) {
		percent = 1.0;
	} else if ( percent < 0.0 ) {
		percent = 0.0;
	}

	// the following line does a straigt calc on percentage of time
	// float percent = (float)(startTime + time - t) / time;

	idVec3 v1 = startPos;
	idVec3 v2 = endPos;
	v1 *= ( 1.0 - percent );
	v2 *= percent;
	v1 += v2;
	interpolatedPos = v1;
	return &interpolatedPos;
}


void idCameraFOV::parse( const char *( *text )  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );

		if ( !token[0] ) {
			break;
		}
		if ( !strcmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp( token, "(" ) || !strcmp( token, "}" ) ) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine( text );
			const char *token = Com_Parse( text );
			if ( Q_stricmp( key.c_str(), "fov" ) == 0 ) {
				fov = atof( token );
			} else if ( Q_stricmp( key.c_str(), "startFOV" ) == 0 ) {
				startFOV = atof( token );
			} else if ( Q_stricmp( key.c_str(), "endFOV" ) == 0 ) {
				endFOV = atof( token );
			} else if ( Q_stricmp( key.c_str(), "time" ) == 0 ) {
				time = atoi( token );
			}
			token = Com_Parse( text );

		} while ( 1 );

		if ( !strcmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );

	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

bool idCameraPosition::parseToken( const char *key, const char *( *text ) ) {
	const char *token = Com_Parse( text );
	if ( Q_stricmp( key, "time" ) == 0 ) {
		time = atol( token );
		return true;
	} else if ( Q_stricmp( key, "type" ) == 0 ) {
		type = static_cast<idCameraPosition::positionType>( atoi( token ) );
		return true;
	} else if ( Q_stricmp( key, "velocity" ) == 0 ) {
		int t = atol( token );
		token = Com_Parse( text );
		int d = atol( token );
		token = Com_Parse( text );
		float s = atof( token );
		addVelocity( t, d, s );
		return true;
	} else if ( Q_stricmp( key, "baseVelocity" ) == 0 ) {
		baseVelocity = atof( token );
		return true;
	} else if ( Q_stricmp( key, "name" ) == 0 ) {
		name = token;
		return true;
	} else if ( Q_stricmp( key, "time" ) == 0 ) {
		time = atoi( token );
		return true;
	}
	Com_UngetToken();
	return false;
}



void idFixedPosition::parse( const char *( *text )  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );

		if ( !token[0] ) {
			break;
		}
		if ( !strcmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp( token, "(" ) || !strcmp( token, "}" ) ) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine( text );

			const char *token = Com_Parse( text );
			if ( Q_stricmp( key.c_str(), "pos" ) == 0 ) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, pos );
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken( key.c_str(), text );
			}
			token = Com_Parse( text );

		} while ( 1 );

		if ( !strcmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );

	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

void idInterpolatedPosition::parse( const char *( *text )  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );

		if ( !token[0] ) {
			break;
		}
		if ( !strcmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp( token, "(" ) || !strcmp( token, "}" ) ) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine( text );

			const char *token = Com_Parse( text );
			if ( Q_stricmp( key.c_str(), "startPos" ) == 0 ) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, startPos );
			} else if ( Q_stricmp( key.c_str(), "endPos" ) == 0 ) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, endPos );
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken( key.c_str(), text );
			}
			token = Com_Parse( text );

		} while ( 1 );

		if ( !strcmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );

	Com_UngetToken();
	Com_MatchToken( text, "}" );
}


void idSplinePosition::parse( const char *( *text )  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );

		if ( !token[0] ) {
			break;
		}
		if ( !strcmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp( token, "(" ) || !strcmp( token, "}" ) ) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine( text );

			const char *token = Com_Parse( text );
			if ( Q_stricmp( key.c_str(), "target" ) == 0 ) {
				target.parse( text );
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken( key.c_str(), text );
			}
			token = Com_Parse( text );

		} while ( 1 );

		if ( !strcmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );

	Com_UngetToken();
	Com_MatchToken( text, "}" );
}



void idCameraFOV::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tfov %f\n", fov );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tstartFOV %f\n", startFOV );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tendFOV %f\n", endFOV );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\ttime %i\n", time );
	FS_Write( s.c_str(), s.length(), file );

	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}


void idCameraPosition::write( fileHandle_t file, const char *p ) {

	idStr s = va( "\t\ttime %i\n", time );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\ttype %i\n", static_cast<int>( type ) );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tname %s\n", name.c_str() );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tbaseVelocity %f\n", baseVelocity );
	FS_Write( s.c_str(), s.length(), file );

	for ( int i = 0; i < velocities.Num(); i++ ) {
		s = va( "\t\tvelocity %i %i %f\n", velocities[i]->startTime, velocities[i]->time, velocities[i]->speed );
		FS_Write( s.c_str(), s.length(), file );
	}

}

void idFixedPosition::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	idCameraPosition::write( file, p );
	s = va( "\t\tpos ( %f %f %f )\n", pos.x, pos.y, pos.z );
	FS_Write( s.c_str(), s.length(), file );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}

void idInterpolatedPosition::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	idCameraPosition::write( file, p );
	s = va( "\t\tstartPos ( %f %f %f )\n", startPos.x, startPos.y, startPos.z );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\tendPos ( %f %f %f )\n", endPos.x, endPos.y, endPos.z );
	FS_Write( s.c_str(), s.length(), file );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}

void idSplinePosition::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	idCameraPosition::write( file, p );
	target.write( file, "target" );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}

void idCameraDef::addTarget( const char *name, idCameraPosition::positionType type ) {

#if !defined RTCW_MP
	// TTimo: unused
#endif // RTCW_XX

	//const char *text = (name == NULL) ? va("target0%d", numTargets()+1) : name;
	idCameraPosition *pos = newFromType( type );
	if ( pos ) {
		pos->setName( name );
		targetPositions.Append( pos );
		activeTarget = numTargets() - 1;
		if ( activeTarget == 0 ) {
			// first one
			addEvent( idCameraEvent::EVENT_TARGET, name, 0 );
		}
	}
}

const idVec3 *idSplinePosition::getPosition( int t ) {
	static idVec3 interpolatedPos;

	float velocity = getVelocity( t );
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds
	timePassed /= 1000;

	float distToTravel = timePassed * velocity;

	distSoFar += distToTravel;
	double tempDistance = target.totalDistance();

	double percent = (double)( distSoFar ) / tempDistance;

	double targetDistance = percent * tempDistance;
	tempDistance = 0;

	double lastDistance1,lastDistance2;
	lastDistance1 = lastDistance2 = 0;
	//FIXME: calc distances on spline build
	idVec3 temp;
	int count = target.numSegments();

#if defined RTCW_SP
	// TTimo fixed MSVCism
#else
	// TTimo fixed MSVCism: for(int i = 1; ...
#endif // RTCW_XX

	int i;
	for ( i = 1; i < count; i++ ) {
		temp = *target.getSegmentPoint( i - 1 );
		temp -= *target.getSegmentPoint( i );
		tempDistance += temp.Length();
		if ( i & 1 ) {
			lastDistance1 = tempDistance;
		} else {
			lastDistance2 = tempDistance;
		}
		if ( tempDistance >= targetDistance ) {
			break;
		}
	}

	if ( i >= count - 1 ) {
		interpolatedPos = *target.getSegmentPoint( i - 1 );
	} else {
#if 0
		double timeHi = target.getSegmentTime( i + 1 );
		double timeLo = target.getSegmentTime( i - 1 );
		double percent = ( timeHi - t ) / ( timeHi - timeLo );
		idVec3 v1 = *target.getSegmentPoint( i - 1 );
		idVec3 v2 = *target.getSegmentPoint( i + 1 );
		v2 *= ( 1.0 - percent );
		v1 *= percent;
		v2 += v1;
		interpolatedPos = v2;
#else
		if ( lastDistance1 > lastDistance2 ) {
			double d = lastDistance2;
			lastDistance2 = lastDistance1;
			lastDistance1 = d;
		}

		idVec3 v1 = *target.getSegmentPoint( i - 1 );
		idVec3 v2 = *target.getSegmentPoint( i );
		double percent = ( lastDistance2 - targetDistance ) / ( lastDistance2 - lastDistance1 );
		v2 *= ( 1.0 - percent );
		v1 *= percent;
		v2 += v1;
		interpolatedPos = v2;
#endif
	}
	return &interpolatedPos;

}



