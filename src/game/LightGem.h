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
#ifndef __LIGHTGEM_H__
#define __LIGHTGEM_H__

//----------------------------------
// Preprocessor Constants
//----------------------------------
// Number of passes that we can do at most. This is 6 because it's simply a cube that is rendered
// from all sides. This is not needed though, because a top and a bottom render with a pyramidic
// shape would be sufficient to cover all lighting situations. For silouhette detection we might
// consider more stages though.
#define DARKMOD_LG_MAX_RENDERPASSES			2
#define DARKMOD_LG_MAX_IMAGESPLIT			4
#define DARKMOD_LG_RENDER_MODEL				"models/darkmod/misc/system/lightgem.lwo"
#define DARKMOD_LG_ENTITY_NAME				"lightgem_surface"
// The lightgem viewid defines the viewid that is to be used for the lightgem surfacetestmodel
#define DARKMOD_LG_VIEWID					-1

#define DARKMOD_LG_RENDER_WIDTH				64 // LG render resolution - keep it a power-of-two!
#define DARKMOD_LG_RENDER_FOV				70.0f
#define DARKMOD_LG_BPP						3 // 3 Channels of 8 bits

// The colour is converted to a grayscale value which determines the state of the lightgem.
// LightGem = (0.29900*R+0.58700*G+0.11400*B) * 0.0625

#define DARKMOD_LG_MIN						1
#define DARKMOD_LG_MAX						32
#define DARKMOD_LG_FRACTION					(1.0f/32.0f)		// Do we use this?
#define DARKMOD_LG_RED						0.29900f
#define DARKMOD_LG_GREEN					0.58700f
#define DARKMOD_LG_BLUE						0.11400f
#define DARKMOD_LG_SCALE					(1.0f/255.0f)			// scaling factor for grayscale value
#define DARKMOD_LG_TRIRATIO					(1.0f/((DARKMOD_LG_RENDER_WIDTH*DARKMOD_LG_RENDER_WIDTH)/4.0f))

//----------------------------------
// Class Declarations.
//----------------------------------

class LightGem {
private:
	int						m_LightgemShotSpot;
	float					m_LightgemShotValue[DARKMOD_LG_MAX_RENDERPASSES];
	idEntityPtr<idEntity>	m_LightgemSurface;

	unsigned char			*m_LightgemImgBuffer;
	renderView_t			m_Lightgem_rv;
	float 					m_fColVal[DARKMOD_LG_MAX_IMAGESPLIT];

public:
	//---------------------------------
	// Construction/Destruction
	//---------------------------------
	LightGem();
	~LightGem();

	//---------------------------------
	// Initialization
	//---------------------------------
	void Initialize();
	void Deinitialize();
	void Clear();

	//---------------------------------
	// Persistence
	//---------------------------------
	void Save( idSaveGame 		&a_saveGame );
	void Restore( idRestoreGame 	&a_savedGame );
	//---------------------------------

	//---------------------------------
	// SpawnlightgemEntity will create exactly one lightgem entity for the map and ensures
	//  that no multiple copies of it will exist.
	//---------------------------------
	void SpawnLightGemEntity( idMapFile 	*a_mapFile );
	void InitializeLightGemEntity();

	//---------------------------------
	// Calculation
	//---------------------------------
	float	Calculate( idPlayer 	*a_pPlayer );

private:
	void AnalyzeRenderImage();
	void ToggleShadows( bool noShadows );
};

#endif // __LIGHTGEM_H__
