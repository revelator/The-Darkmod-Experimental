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

#include "precompiled_engine.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile( "$Id$" );

#include "tr_local.h"
#include "Model_local.h"

static const char *parametricParticle_SnapshotName = "_ParametricParticle_Snapshot_";

/*
====================
idRenderModelPrt::idRenderModelPrt
====================
*/
idRenderModelPrt::idRenderModelPrt() {
	particleSystem = NULL;
}

/*
====================
idRenderModelPrt::InitFromFile
====================
*/
void idRenderModelPrt::InitFromFile( const char *fileName ) {
	name = fileName;
	particleSystem = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, fileName ) );
}

/*
=================
idRenderModelPrt::TouchData
=================
*/
void idRenderModelPrt::TouchData( void ) {
	// Ensure our particle system is added to the list of referenced decls
	particleSystem = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, name ) );
}

/*
====================
idRenderModelPrt::InstantiateDynamicModel
====================
*/
idRenderModel *idRenderModelPrt::InstantiateDynamicModel( const struct renderEntity_s *renderEntity, const struct viewDef_s *viewDef, idRenderModel *cachedModel ) {
	idRenderModelStatic	*staticModel;
	if( cachedModel && !r_useCachedDynamicModels.GetBool() ) {
		delete cachedModel;
		cachedModel = NULL;
	}
	// this may be triggered by a model trace or other non-view related source, to which we should look like an empty model
	if( !renderEntity || !viewDef ) {
		delete cachedModel;
		return NULL;
	} else if( r_skipParticles.GetBool() ) {
		delete cachedModel;
		return NULL;
	}
	/*
	// if the entire system has faded out
	if ( renderEntity->shaderParms[SHADERPARM_PARTICLE_STOPTIME] && viewDef->renderView.time * 0.001f >= renderEntity->shaderParms[SHADERPARM_PARTICLE_STOPTIME] ) {
	delete cachedModel;
	return NULL;
	}
	*/
	else if( cachedModel ) {
		assert( dynamic_cast<idRenderModelStatic *>( cachedModel ) != NULL );
		assert( idStr::Icmp( cachedModel->Name(), parametricParticle_SnapshotName ) == 0 );
		staticModel = static_cast<idRenderModelStatic *>( cachedModel );
	} else {
		staticModel = new idRenderModelStatic;
		staticModel->InitEmpty( parametricParticle_SnapshotName );
	}
	particleGen_t g;
	g.renderEnt = renderEntity;
	g.renderView = &viewDef->renderView;
	g.origin.Zero();
	g.axis.Identity();
	for( int stageNum = 0; stageNum < particleSystem->stages.Num(); stageNum++ ) {
		idParticleStage *stage = particleSystem->stages[stageNum];
		if( !stage->material || !stage->cycleMsec ) {
			continue;
		} else if( stage->hidden ) {		// just for gui particle editor use
			staticModel->DeleteSurfaceWithId( stageNum );
			continue;
		}
		idRandom steppingRandom, steppingRandom2;
		const int stageAge = g.renderView->time + ( renderEntity->shaderParms[SHADERPARM_TIMEOFFSET] - stage->timeOffset ) * 1000;
		const int stageCycle = stageAge / stage->cycleMsec;
		// some particles will be in this cycle, some will be in the previous cycle
		steppingRandom.SetSeed( ( ( stageCycle << 10 )     & idRandom::MAX_RAND ) ^ ( int )( renderEntity->shaderParms[SHADERPARM_DIVERSITY] * idRandom::MAX_RAND ) );
		steppingRandom2.SetSeed( ( ( ( stageCycle - 1 ) << 10 ) & idRandom::MAX_RAND ) ^ ( int )( renderEntity->shaderParms[SHADERPARM_DIVERSITY] * idRandom::MAX_RAND ) );
		const int	count = stage->totalParticles * stage->NumQuadsPerParticle();
		int surfaceNum = 0;
		modelSurface_t *surf;
		if( staticModel->FindSurfaceWithId( stageNum, surfaceNum ) ) {
			surf = &staticModel->surfaces[surfaceNum];
			R_FreeStaticTriSurfVertexCaches( surf->geometry );
		} else {
			surf = &staticModel->surfaces.Alloc();
			surf->id = stageNum;
			surf->shader = stage->material;
			surf->geometry = R_AllocStaticTriSurf();
			R_AllocStaticTriSurfVerts( surf->geometry, 4 * count );
			R_AllocStaticTriSurfIndexes( surf->geometry, 6 * count );
			R_AllocStaticTriSurfPlanes( surf->geometry, 6 * count );
		}
		int numVerts = 0;
		idDrawVert *verts = surf->geometry->verts;
		for( int index = 0; index < stage->totalParticles; index++ ) {
			g.index = index;
			// calculate local age for this index
			//const int bunchOffset = (index * 1000 * stage->particleLife * stage->spawnBunching) / stage->totalParticles;
			//const int particleAge = stageAge - bunchOffset
			const int particleAge = stageAge - ( ( index * 1000 * stage->particleLife * stage->spawnBunching ) / stage->totalParticles );
			const int particleCycle = particleAge / stage->cycleMsec;
			// before the particleSystem has spawned or
			// cycled systems will only run cycle times
			if( particleCycle < 0 || ( stage->cycles && particleCycle >= stage->cycles ) ) {
				continue;
			}
			const int inCycleTime = particleAge - particleCycle * stage->cycleMsec;
			if( renderEntity->shaderParms[SHADERPARM_PARTICLE_STOPTIME] &&
					g.renderView->time - inCycleTime >= renderEntity->shaderParms[SHADERPARM_PARTICLE_STOPTIME] * 1000 ) {
				// don't fire any more particles
				continue;
			}
			// supress particles before or after the age clamp
			g.frac = ( float )inCycleTime / ( stage->particleLife * 1000 );
			if( g.frac < 0.0f || g.frac > 1.0f ) {
				// < 0.0f ; yet to be spawned
				// > 1.0f ; particle is in the deadTime band
				continue;
			} else {
				g.age = g.frac * stage->particleLife;
			}
			// bump the random
			steppingRandom.RandomInt();
			steppingRandom2.RandomInt();
			if( particleCycle == stageCycle ) {
				g.random = steppingRandom;
			} else {
				g.random = steppingRandom2;
			}
			// this is needed so aimed particles can calculate origins at different times
			g.originalRandom = g.random;
			// if the particle doesn't get drawn because it is faded out or beyond a kill region, don't increment the verts
			numVerts += stage->CreateParticle( &g, verts + numVerts );
		}
		// numVerts must be a multiple of 4
		assert( ( numVerts & 3 ) == 0 && numVerts <= 4 * count );
		// build the indexes
		int	numIndexes = 0;
		glIndex_t *indexes = surf->geometry->indexes;
		for( int i = 0; i < numVerts; i += 4 ) {
			indexes[numIndexes + 0] = i;
			indexes[numIndexes + 1] = i + 2;
			indexes[numIndexes + 2] = i + 3;
			indexes[numIndexes + 3] = i;
			indexes[numIndexes + 4] = i + 3;
			indexes[numIndexes + 5] = i + 1;
			numIndexes += 6;
		}
		surf->geometry->tangentsCalculated = false;
		surf->geometry->facePlanesCalculated = false;
		surf->geometry->numVerts = numVerts;
		surf->geometry->numIndexes = numIndexes;
		surf->geometry->bounds = stage->bounds;		// just always draw the particles
	}
	return staticModel;
}

/*
====================
idRenderModelPrt::IsDynamicModel
====================
*/
dynamicModel_t idRenderModelPrt::IsDynamicModel() const {
	return DM_CONTINUOUS;
}

/*
====================
idRenderModelPrt::Bounds
====================
*/
idBounds idRenderModelPrt::Bounds( const struct renderEntity_s *ent ) const {
	return particleSystem->bounds;
}

/*
====================
idRenderModelPrt::DepthHack
====================
*/
float idRenderModelPrt::DepthHack() const {
	return particleSystem->depthHack;
}

/*
====================
idRenderModelPrt::Memory
====================
*/
int idRenderModelPrt::Memory() const {
	int total = idRenderModelStatic::Memory();
	if( particleSystem ) {
		total += sizeof( *particleSystem );
		for( int i = 0; i < particleSystem->stages.Num(); i++ ) {
			total += sizeof( particleSystem->stages[i] );
		}
	}
	return total;
}