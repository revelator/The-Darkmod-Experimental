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

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile( "$Id$" );

#include "Memory.h"
#include "AI.h"

namespace ai {
Memory::Memory( idAI *owningAI ) :
	owner( owningAI ),
	lastAlertRiseTime( -10000 ),
	deadTimeAfterAlertRise( 300 ),
	lastPatrolChatTime( -1 ),
	lastTimeFriendlyAISeen( -10000 ), // grayman #3472 - must be less than -5000
	lastTimeEnemySeen( -1 ),
	lastTimeVisualStimBark( -1 ),
	lastTimeAlertBark( -MIN_TIME_BETWEEN_ALERT_BARKS ), // grayman #3496
	nextTimeLightStimBark( -1 ),	// grayman #2603
	countEvidenceOfIntruders( 0 ),
	nextHeadTurnCheckTime( 0 ),
	currentlyHeadTurning( false ),
	headTurnEndTime( 0 ),
	idlePosition( idMath::INFINITY, idMath::INFINITY, idMath::INFINITY ),
	idleYaw( 0 ),
	playIdleAnimations( true ),
	enemiesHaveBeenSeen( false ),
	hasBeenAttackedByEnemy( false ),
	itemsHaveBeenStolen( false ),
	itemsHaveBeenBroken( false ),
	unconsciousPeopleHaveBeenFound( false ),
	deadPeopleHaveBeenFound( false ),
	alertPos( idMath::INFINITY, idMath::INFINITY, idMath::INFINITY ), // grayman #3438

	// grayman #2903 - for saving position where AI encounter an alert, and a timestamp for that alert
	posEnemySeen( 0, 0, 0 ),
	posMissingItem( 0, 0, 0 ),
	posEvidenceIntruders( 0, 0, 0 ),
	mandatory( false ),			// grayman #3331
	timeEvidenceIntruders( 0 ),
	visualAlert( false ),			// grayman #2422
	stopRelight( false ),			// grayman #2603
	stopExaminingRope( false ),	// grayman #2872
	stopReactingToHit( false ),	// grayman #2816
	stopReactingToPickedPocket( false ), // grayman #3559
	latchPickedPocket( false ),	// grayman #3559
	insideAlertWindow( false ),	// grayman #3559
	stopHandlingDoor( false ),	// grayman #2816
	stopHandlingElevator( false ), // grayman #2816
	nextTime2GenRandomSpot( 0 ),	// grayman #2422
	alertClass( EAlertNone ),
	alertType( EAlertTypeNone ),
	alertRadius( -1 ),
	lastAudioAlertTime( -1 ),
	stimulusLocationItselfShouldBeSearched( false ),
	investigateStimulusLocationClosely( false ),
	alertedDueToCommunication( false ),
	lastAlertPosSearched( 0, 0, 0 ), // grayman #3492 - reinstate this
	alertSearchCenter( idMath::INFINITY, idMath::INFINITY, idMath::INFINITY ),
	alertSearchVolume( 0, 0, 0 ),
	alertSearchExclusionVolume( 0, 0, 0 ),
	lastEnemyPos( 0, 0, 0 ),
	canHitEnemy( false ),
	willBeAbleToHitEnemy( false ),
	canBeHitByEnemy( false ),
	currentSearchSpot( 0, 0, 0 ),
	hidingSpotTestStarted( false ),
	hidingSpotSearchDone( true ),
	noMoreHidingSpots( false ),
	restartSearchForHidingSpots( false ),
	hidingSpotThinkFrameCount( 0 ),
	firstChosenHidingSpotIndex( 0 ),
	currentChosenHidingSpotIndex( 0 ),
	chosenHidingSpot( 0, 0, 0 ),
	hidingSpotInvestigationInProgress( false ),
	fleeingDone( true ),
	positionBeforeTakingCover( 0, 0, 0 ),
	resolvingMovementBlock( false ),
	issueMoveToPositionTask( false ), // grayman #3052
	closeSuspiciousDoor( false ), // grayman #1327
	currentSearchEventID( -1 ), // grayman #3424
	playerResponsible( false ), // grayman #3679 - is the player responsible for the attack?
	stayPut( false ), // grayman #3528
	combatState( -1 ), /// grayman #3507
	susDoorSameAsCurrentDoor( false ) { // grayman #3643
	attacker = NULL; // grayman #3679 - who attacked me
}

// Save/Restore routines
void Memory::Save( idSaveGame *savefile ) const {
	savefile->WriteObject( owner );
	currentPath.Save( savefile );
	nextPath.Save( savefile );
	lastPath.Save( savefile );
	savefile->WriteInt( lastAlertRiseTime );
	savefile->WriteInt( lastPatrolChatTime );
	savefile->WriteInt( lastTimeFriendlyAISeen );
	savefile->WriteInt( lastTimeEnemySeen );
	savefile->WriteInt( lastTimeVisualStimBark );
	savefile->WriteInt( lastTimeAlertBark ); // grayman #3496
	savefile->WriteBool( agitatedSearched ); // grayman #3496
	savefile->WriteBool( mightHaveSeenPlayer ); // grayman #3515
	savefile->WriteInt( countEvidenceOfIntruders );
	savefile->WriteInt( nextHeadTurnCheckTime );
	savefile->WriteBool( currentlyHeadTurning );
	savefile->WriteBool( currentlyBarking ); // grayman #3182
	savefile->WriteInt( headTurnEndTime );
	savefile->WriteVec3( idlePosition );
	savefile->WriteFloat( idleYaw );
	savefile->WriteBool( playIdleAnimations );
	savefile->WriteBool( enemiesHaveBeenSeen );
	savefile->WriteBool( hasBeenAttackedByEnemy );
	savefile->WriteBool( itemsHaveBeenStolen );
	savefile->WriteBool( itemsHaveBeenBroken );
	savefile->WriteBool( unconsciousPeopleHaveBeenFound );
	savefile->WriteBool( deadPeopleHaveBeenFound );
	savefile->WriteBool( prevSawEvidence ); // grayman #3424
	savefile->WriteBool( stayPut ); // grayman #3528
	savefile->WriteVec3( alertPos );
	// grayman #2903 - for saving position where AI encounter an alert, and a timestamp for that alert
	savefile->WriteVec3( posEnemySeen );
	savefile->WriteVec3( posMissingItem );
	savefile->WriteVec3( posEvidenceIntruders );
	savefile->WriteBool( mandatory );				// grayman #3331
	savefile->WriteInt( timeEvidenceIntruders );
	savefile->WriteBool( visualAlert );			// grayman #2422
	savefile->WriteBool( stopRelight );			// grayman #2603
	savefile->WriteBool( stopExaminingRope );		// grayman #2872
	savefile->WriteBool( stopReactingToHit );		// grayman #2816
	savefile->WriteBool( stopReactingToPickedPocket ); // grayman #3559
	savefile->WriteBool( latchPickedPocket );		// grayman #3559
	savefile->WriteBool( insideAlertWindow );		// grayman #3559
	savefile->WriteBool( stopHandlingDoor );		// grayman #2816
	savefile->WriteBool( stopHandlingElevator );	// grayman #2816
	savefile->WriteInt( nextTime2GenRandomSpot );	// grayman #2422
	savefile->WriteInt( static_cast<int>( alertClass ) );
	savefile->WriteInt( static_cast<int>( causeOfPain ) ); // grayman #3140
	savefile->WriteBool( painStatePushedThisFrame ); // grayman #3424
	savefile->WriteInt( static_cast<int>( alertType ) );
	savefile->WriteFloat( alertRadius );
	savefile->WriteBool( stimulusLocationItselfShouldBeSearched );
	savefile->WriteBool( investigateStimulusLocationClosely );
	savefile->WriteBool( alertedDueToCommunication );
	savefile->WriteVec3( lastAlertPosSearched ); // grayman #3492 - reinstate this
	savefile->WriteVec3( alertSearchCenter );
	savefile->WriteVec3( alertSearchVolume );
	savefile->WriteVec3( alertSearchExclusionVolume );
	savefile->WriteVec3( lastEnemyPos );
	savefile->WriteBool( canHitEnemy );
	savefile->WriteBool( willBeAbleToHitEnemy );
	savefile->WriteBool( canBeHitByEnemy );
	savefile->WriteVec3( currentSearchSpot );
	savefile->WriteBool( hidingSpotTestStarted );
	savefile->WriteBool( hidingSpotSearchDone );
	savefile->WriteBool( noMoreHidingSpots );
	savefile->WriteBool( restartSearchForHidingSpots );
	savefile->WriteInt( hidingSpotThinkFrameCount );
	savefile->WriteInt( firstChosenHidingSpotIndex );
	savefile->WriteInt( currentChosenHidingSpotIndex );
	savefile->WriteVec3( chosenHidingSpot );
	savefile->WriteBool( hidingSpotInvestigationInProgress );
	savefile->WriteBool( fleeingDone );
	savefile->WriteVec3( positionBeforeTakingCover );
	savefile->WriteBool( resolvingMovementBlock );
	lastDoorHandled.Save( savefile ); // grayman #2712
	hitByThisMoveable.Save( savefile ); // grayman #2816
	corpseFound.Save( savefile );		 // grayman #3424
	relightLight.Save( savefile );	 // grayman #2603
	savefile->WriteInt( nextTimeLightStimBark );	// grayman #2603
	doorRelated.currentDoor.Save( savefile );
	savefile->WriteInt( doorRelated.doorInfo.size() );
	for( DoorInfoMap::const_iterator i = doorRelated.doorInfo.begin();
			i != doorRelated.doorInfo.end(); ++i ) {
		savefile->WriteObject( i->first );
		i->second->Save( savefile );
	}
	// greebo: Don't save the AAS areaNum => DoorInfo mapping (can be re-calculated at restore time)
	savefile->WriteInt( static_cast<int>( greetingInfo.size() ) );
	for( ActorGreetingInfoMap::const_iterator i = greetingInfo.begin();
			i != greetingInfo.end(); ++i ) {
		savefile->WriteObject( i->first );
		savefile->WriteInt( i->second.nextGreetingTime ); // grayman #3415
		savefile->WriteInt( i->second.nextWarningTime ); // grayman #3424
	}
	// grayman #2866 - start of changes
	closeMe.Save( savefile );
	frontPos.Save( savefile );
	backPos.Save( savefile );
	savefile->WriteBool( closeSuspiciousDoor );
	savefile->WriteBool( doorSwingsToward );
	savefile->WriteBool( closeFromAwayPos );
	savefile->WriteInt( susDoorCloseFromThisSide ); // grayman #3643
	savefile->WriteBool( susDoorSameAsCurrentDoor );
	savefile->WriteFloat( savedAlertLevelDecreaseRate );
	// end of #2866 changes
	savefile->WriteInt( currentSearchEventID ); // grayman #3424
	attacker.Save( savefile ); // grayman #3679
	savefile->WriteBool( playerResponsible ); // grayman #3679
	savefile->WriteInt( combatState ); // grayman #3507
	savefile->WriteBool( issueMoveToPositionTask ); // grayman #3052
}

void Memory::Restore( idRestoreGame *savefile ) {
	savefile->ReadObject( reinterpret_cast<idClass *&>( owner ) );
	currentPath.Restore( savefile );
	nextPath.Restore( savefile );
	lastPath.Restore( savefile );
	savefile->ReadInt( lastAlertRiseTime );
	savefile->ReadInt( lastPatrolChatTime );
	savefile->ReadInt( lastTimeFriendlyAISeen );
	savefile->ReadInt( lastTimeEnemySeen );
	savefile->ReadInt( lastTimeVisualStimBark );
	savefile->ReadInt( lastTimeAlertBark ); // grayman #3496
	savefile->ReadBool( agitatedSearched ); // grayman #3496
	savefile->ReadBool( mightHaveSeenPlayer ); // grayman #3515
	savefile->ReadInt( countEvidenceOfIntruders );
	savefile->ReadInt( nextHeadTurnCheckTime );
	savefile->ReadBool( currentlyHeadTurning );
	savefile->ReadBool( currentlyBarking ); // grayman #3182
	savefile->ReadInt( headTurnEndTime );
	savefile->ReadVec3( idlePosition );
	savefile->ReadFloat( idleYaw );
	savefile->ReadBool( playIdleAnimations );
	savefile->ReadBool( enemiesHaveBeenSeen );
	savefile->ReadBool( hasBeenAttackedByEnemy );
	savefile->ReadBool( itemsHaveBeenStolen );
	savefile->ReadBool( itemsHaveBeenBroken );
	savefile->ReadBool( unconsciousPeopleHaveBeenFound );
	savefile->ReadBool( deadPeopleHaveBeenFound );
	savefile->ReadBool( prevSawEvidence ); // grayman #3424
	savefile->ReadBool( stayPut ); // grayman #3528
	savefile->ReadVec3( alertPos );
	// grayman #2903 - for saving position where AI encounter an alert, and a timestamp for that alert
	savefile->ReadVec3( posEnemySeen );
	savefile->ReadVec3( posMissingItem );
	savefile->ReadVec3( posEvidenceIntruders );
	savefile->ReadBool( mandatory );				// grayman #3331
	savefile->ReadInt( timeEvidenceIntruders );
	savefile->ReadBool( visualAlert );			// grayman #2422
	savefile->ReadBool( stopRelight );			// grayman #2603
	savefile->ReadBool( stopExaminingRope );		// grayman #2872
	savefile->ReadBool( stopReactingToHit );		// grayman #2816
	savefile->ReadBool( stopReactingToPickedPocket ); // grayman #3559
	savefile->ReadBool( latchPickedPocket );		// grayman #3559
	savefile->ReadBool( insideAlertWindow );		// grayman #3559
	savefile->ReadBool( stopHandlingDoor );		// grayman #2816
	savefile->ReadBool( stopHandlingElevator );	// grayman #2816
	savefile->ReadInt( nextTime2GenRandomSpot );	// grayman #2422
	int temp;
	savefile->ReadInt( temp );
	alertClass = static_cast<EAlertClass>( temp );
	savefile->ReadInt( temp );
	causeOfPain = static_cast<EPainCause>( temp ); // grayman #3140
	savefile->ReadBool( painStatePushedThisFrame ); // grayman #3424
	savefile->ReadInt( temp );
	alertType = static_cast<EAlertType>( temp );
	savefile->ReadFloat( alertRadius );
	savefile->ReadBool( stimulusLocationItselfShouldBeSearched );
	savefile->ReadBool( investigateStimulusLocationClosely );
	savefile->ReadBool( alertedDueToCommunication );
	savefile->ReadVec3( lastAlertPosSearched ); // grayman #3492 - reinstate this
	savefile->ReadVec3( alertSearchCenter );
	savefile->ReadVec3( alertSearchVolume );
	savefile->ReadVec3( alertSearchExclusionVolume );
	savefile->ReadVec3( lastEnemyPos );
	savefile->ReadBool( canHitEnemy );
	savefile->ReadBool( willBeAbleToHitEnemy );
	savefile->ReadBool( canBeHitByEnemy );
	savefile->ReadVec3( currentSearchSpot );
	savefile->ReadBool( hidingSpotTestStarted );
	savefile->ReadBool( hidingSpotSearchDone );
	savefile->ReadBool( noMoreHidingSpots );
	savefile->ReadBool( restartSearchForHidingSpots );
	savefile->ReadInt( hidingSpotThinkFrameCount );
	savefile->ReadInt( firstChosenHidingSpotIndex );
	savefile->ReadInt( currentChosenHidingSpotIndex );
	savefile->ReadVec3( chosenHidingSpot );
	savefile->ReadBool( hidingSpotInvestigationInProgress );
	savefile->ReadBool( fleeingDone );
	savefile->ReadVec3( positionBeforeTakingCover );
	savefile->ReadBool( resolvingMovementBlock );
	lastDoorHandled.Restore( savefile );	 // grayman #2712
	hitByThisMoveable.Restore( savefile ); // grayman #2816
	corpseFound.Restore( savefile );		 // grayman #3424
	relightLight.Restore( savefile );		 // grayman #2603
	savefile->ReadInt( nextTimeLightStimBark );	// grayman #2603
	doorRelated.currentDoor.Restore( savefile );
	// Clear the containers before restoring them
	doorRelated.doorInfo.clear();
	doorRelated.areaDoorInfoMap.clear();
	int num;
	savefile->ReadInt( num );
	for( int i = 0; i < num; i++ ) {
		CFrobDoor *door;
		savefile->ReadObject( reinterpret_cast<idClass *&>( door ) );
		// Allocate a new doorinfo structure and insert it into the map
		DoorInfoPtr info( new DoorInfo );
		std::pair<DoorInfoMap::iterator, bool> result =
			doorRelated.doorInfo.insert( DoorInfoMap::value_type( door, info ) );
		// The insertion must succeed (unique pointers in the map!), otherwise we have inconsistent data
		assert( result.second == true );
		info->Restore( savefile );
	}
	// greebo: Reconstruct the AAS areaNum => DoorInfo mapping
	for( DoorInfoMap::iterator i = doorRelated.doorInfo.begin();
			i != doorRelated.doorInfo.end(); ++i ) {
		// Use the areanumber as index and insert the pointer into the map
		doorRelated.areaDoorInfoMap.insert(
			AreaToDoorInfoMap::value_type( i->second->areaNum, i->second )
		);
	}
	greetingInfo.clear();
	savefile->ReadInt( temp );
	for( int i = 0; i < temp; ++i ) {
		idAI *ai;
		savefile->ReadObject( reinterpret_cast<idClass *&>( ai ) );
		std::pair<ActorGreetingInfoMap::iterator, bool> result = greetingInfo.insert(
					ActorGreetingInfoMap::value_type( ai, GreetingInfo() ) );
		savefile->ReadInt( result.first->second.nextGreetingTime ); // grayman #3415
		savefile->ReadInt( result.first->second.nextWarningTime ); // grayman #3424
	}
	// grayman #2866 - start of changes
	closeMe.Restore( savefile );
	frontPos.Restore( savefile );
	backPos.Restore( savefile );
	savefile->ReadBool( closeSuspiciousDoor );
	savefile->ReadBool( doorSwingsToward );
	savefile->ReadBool( closeFromAwayPos );
	savefile->ReadInt( susDoorCloseFromThisSide ); // grayman #3643
	savefile->ReadBool( susDoorSameAsCurrentDoor );
	savefile->ReadFloat( savedAlertLevelDecreaseRate );
	// end of #2866 changes
	savefile->ReadInt( currentSearchEventID ); // grayman #3424
	attacker.Restore( savefile ); // grayman #3679
	savefile->ReadBool( playerResponsible ); // grayman #3679
	savefile->ReadInt( combatState ); // grayman #3507
	savefile->ReadBool( issueMoveToPositionTask ); // grayman #3052
}

DoorInfo &Memory::GetDoorInfo( CFrobDoor *door ) {
	DoorInfoMap::iterator i = doorRelated.doorInfo.find( door );
	if( i != doorRelated.doorInfo.end() ) {
		return *( i->second );
	} else {
		DoorInfoPtr info( new DoorInfo );
		// Set the area number
		info->areaNum = door->GetAASArea( owner->GetAAS() );
		// Insert into the map
		std::pair<DoorInfoMap::iterator, bool> result =
			doorRelated.doorInfo.insert( DoorInfoMap::value_type( door, info ) );
		// Add the areaNum => info mapping for faster lookup using area numbers
		doorRelated.areaDoorInfoMap.insert(
			AreaToDoorInfoMap::value_type( info->areaNum, info )
		);
		return *( result.first->second );
	}
}

DoorInfoPtr Memory::GetDoorInfo( int areaNum ) {
	AreaToDoorInfoMap::iterator i = doorRelated.areaDoorInfoMap.find( areaNum );
	return ( i != doorRelated.areaDoorInfoMap.end() ) ? i->second : DoorInfoPtr();
}

Memory::GreetingInfo &Memory::GetGreetingInfo( idActor *actor ) {
	// Insert structure if not existing
	ActorGreetingInfoMap::iterator i = greetingInfo.find( actor );
	if( i == greetingInfo.end() ) {
		i = greetingInfo.insert( ActorGreetingInfoMap::value_type( actor, GreetingInfo() ) ).first;
	}
	return i->second;
}

void Memory::StopReacting() {
	if( owner->m_RelightingLight ) {
		stopRelight = true; // grayman #2603 - abort a relight in progress
	}
	if( owner->m_ExaminingRope ) {
		stopExaminingRope = true; // grayman #2872 - abort a rope examination
	}
	if( owner->m_ReactingToHit ) {
		stopReactingToHit = true; // grayman #2816
	}
	if( owner->m_ReactingToPickedPocket ) {
		stopReactingToPickedPocket = true; // grayman #3559
	}
}
} // namespace ai