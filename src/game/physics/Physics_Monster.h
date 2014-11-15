/*****************************************************************************
                    The Dark Mod GPL Source Code

 This file is part of the The Dark Mod Source Code, originally based
 on the Doom 3 GPL Source Code as published in 2011.

 The Dark Mod Source Code is free software: you can redistribute it
 and/or modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of the License,
 or (at your option) any later version. For details, see LICENSE.TXT.

 Project: The Dark Mod (http://www.thedarkmod.com/)

 $Revision$ (Revision of last commit)
 $Date$ (Date of last commit)
 $Author$ (Author of last commit)

******************************************************************************/

#ifndef __PHYSICS_MONSTER_H__
#define __PHYSICS_MONSTER_H__

/*
===================================================================================

	Monster physics

	Simulates the motion of a monster through the environment. The monster motion
	is typically driven by animations.

===================================================================================
*/

typedef enum {
	MM_OK,
	MM_SLIDING,
	MM_BLOCKED,
	MM_STEPPED,
	MM_FALLING
} monsterMoveResult_t;

typedef struct monsterPState_s {
	int						atRest;
	bool					onGround;
	idVec3					origin;
	idVec3					velocity;
	idVec3					localOrigin;
	idVec3					pushVelocity;
} monsterPState_t;

class idPhysics_Monster : public idPhysics_Actor {
public:
	CLASS_PROTOTYPE( idPhysics_Monster );

	idPhysics_Monster( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	// maximum step up the monster can take, default 18 units
	void					SetMaxStepHeight( const float newMaxStepHeight );
	float					GetMaxStepHeight( void ) const;

	// Translates the entity upwards by this amount when stepping up to fight gravity during stepping
	void					SetStepUpIncrease( float incr );

	// minimum cosine of floor angle to be able to stand on the floor
	void					SetMinFloorCosine( const float newMinFloorCosine );
	// set delta for next move
	void					SetDelta( const idVec3 &d );
	// returns true if monster is standing on the ground
	bool					OnGround( void ) const;
	// returns the movement result
	monsterMoveResult_t		GetMoveResult( void ) const;
	// overrides any velocity for pure delta movement
	void					ForceDeltaMove( bool force );
	// whether velocity should be affected by gravity
	void					UseFlyMove( bool force );
	// don't use delta movement
	void					UseVelocityMove( bool force );
	// get entity blocking the move
	idEntity 				*GetSlideMoveEntity( void ) const;
	// enable/disable activation by impact
	void					EnableImpact( void );
	void					DisableImpact( void );

public:	// common physics interface
	bool					Evaluate( int timeStepMSec, int endTimeMSec );
	void					UpdateTime( int endTimeMSec );
	int						GetTime( void ) const;

	void					GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const;
	void					ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse );
	void					Activate( void );
	void					PutToRest( void );
	bool					IsAtRest( void ) const;
	int						GetRestStartTime( void ) const;

	void					SaveState( void );
	void					RestoreState( void );

	void					SetOrigin( const idVec3 &newOrigin, int id = -1 );
	void					SetAxis( const idMat3 &newAxis, int id = -1 );

	void					Translate( const idVec3 &translation, int id = -1 );
	void					Rotate( const idRotation &rotation, int id = -1 );

	void					SetLinearVelocity( const idVec3 &newLinearVelocity, int id = 0 );

	const idVec3 			&GetLinearVelocity( int id = 0 ) const;

	void					SetPushed( int deltaTime );
	const idVec3 			&GetPushedLinearVelocity( const int id = 0 ) const;

	void					SetMaster( idEntity *master, const bool orientated = true );

	void					WriteToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadFromSnapshot( const idBitMsgDelta &msg );

private:
	// monster physics state
	monsterPState_t			current;
	monsterPState_t			saved;

	// properties
	float					maxStepHeight;		// maximum step height
	float					stepUpIncrease;		// translates origin upwards by this amount when stepping
	float					minFloorCosine;		// minimum cosine of floor angle
	idVec3					delta;				// delta for next move

	bool					forceDeltaMove;
	bool					fly;
	bool					useVelocityMove;
	bool					noImpact;			// if true do not activate when another object collides

	// results of last evaluate
	monsterMoveResult_t		moveResult;
	idEntity 				*blockingEntity;

private:
	void					CheckGround( monsterPState_t &state );
	monsterMoveResult_t		SlideMove( idVec3 &start, idVec3 &velocity, const idVec3 &delta );
	monsterMoveResult_t		StepMove( idVec3 &start, idVec3 &velocity, const idVec3 &delta );
	void					Rest( void );
};

#endif /* !__PHYSICS_MONSTER_H__ */
