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

/*
=========================================================================================

GENERAL INTERACTION RENDERING

=========================================================================================
*/

/*
====================
GL_SelectTextureNoClient
====================
*/
static void GL_SelectTextureNoClient( int unit ) {
	backEnd.glState.currenttmu = unit;
	glActiveTextureARB( GL_TEXTURE0_ARB + unit );
	RB_LogComment( "glActiveTextureARB( %i )\n", unit );
}
/*
==================
RB_ARB2_DrawInteraction
==================
*/
void	RB_ARB2_DrawInteraction( const drawInteraction_t *din ) {
	// load all the vertex program parameters
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_ORIGIN, din->localLightOrigin.ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_VIEW_ORIGIN, din->localViewOrigin.ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_S, din->lightProjection[0].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_T, din->lightProjection[1].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_Q, din->lightProjection[2].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_FALLOFF_S, din->lightProjection[3].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_BUMP_MATRIX_S, din->bumpMatrix[0].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_BUMP_MATRIX_T, din->bumpMatrix[1].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_DIFFUSE_MATRIX_S, din->diffuseMatrix[0].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_DIFFUSE_MATRIX_T, din->diffuseMatrix[1].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_S, din->specularMatrix[0].ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_T, din->specularMatrix[1].ToFloatPtr() );
	// testing fragment based normal mapping
	if( r_testARBProgram.GetBool() ) {
		glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 2, din->localLightOrigin.ToFloatPtr() );
		glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 3, din->localViewOrigin.ToFloatPtr() );
	}
	static const float zero[4] = { 0, 0, 0, 0 };
	static const float one[4] = { 1, 1, 1, 1 };
	static const float negOne[4] = { -1, -1, -1, -1 };
	switch( din->vertexColor ) {
	case SVC_IGNORE:
		glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, zero );
		glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, one );
		break;
	case SVC_MODULATE:
		glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, one );
		glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, zero );
		break;
	case SVC_INVERSE_MODULATE:
		glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, negOne );
		glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, one );
		break;
	}
	// set the constant colors
	glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, din->diffuseColor.ToFloatPtr() );
	glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 1, din->specularColor.ToFloatPtr() );
	// set the textures
	// texture 1 will be the per-surface bump map
	GL_SelectTextureNoClient( 1 );
	din->bumpImage->Bind();
	// texture 2 will be the light falloff texture
	GL_SelectTextureNoClient( 2 );
	din->lightFalloffImage->Bind();
	// texture 3 will be the light projection texture
	GL_SelectTextureNoClient( 3 );
	din->lightImage->Bind();
	// texture 4 is the per-surface diffuse map
	GL_SelectTextureNoClient( 4 );
	din->diffuseImage->Bind();
	// texture 5 is the per-surface specular map
	GL_SelectTextureNoClient( 5 );
	din->specularImage->Bind();
	// draw it
	RB_DrawElementsWithCounters( din->surf->geo );
}

/*
=============
RB_ARB2_CreateDrawInteractions
=============
*/
static void RB_ARB2_CreateDrawInteractions( const drawSurf_t *surf ) {
	if( !surf ) {
		return;
	}
	// perform setup here that will be constant for all interactions
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );
	// bind the vertex program
	// rebb: support dedicated ambient - CVar and direct interactions can probably be removed, they're there mainly for performance testing
	if( r_dedicatedAmbient.GetBool() ) {
		if( backEnd.vLight->lightShader->IsAmbientLight() ) {
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_AMBIENT );
			glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_AMBIENT );
		} else {
			if( r_testARBProgram.GetBool() ) {
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_TEST_DIRECT );
				glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_TEST_DIRECT );
			} else {
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_INTERACTION_DIRECT );
				glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_INTERACTION_DIRECT );
			}
		}
	} else {
		if( r_testARBProgram.GetBool() ) {
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_TEST );
			glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_TEST );
		} else {
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_INTERACTION );
			glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_INTERACTION );
		}
	}
	glEnable( GL_VERTEX_PROGRAM_ARB );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	// enable the vertex arrays
	glEnableVertexAttribArrayARB( 8 );
	glEnableVertexAttribArrayARB( 9 );
	glEnableVertexAttribArrayARB( 10 );
	glEnableVertexAttribArrayARB( 11 );
	glEnableClientState( GL_COLOR_ARRAY );
	// texture 0 is the normalization cube map for the vector towards the light
	GL_SelectTextureNoClient( 0 );
	if( backEnd.vLight->lightShader->IsAmbientLight() ) {
		globalImages->ambientNormalMap->Bind();
	} else {
		globalImages->normalCubeMapImage->Bind();
	}
	// texture 6 is the specular lookup table
	GL_SelectTextureNoClient( 6 );
	if( r_testARBProgram.GetBool() ) {
		globalImages->specular2DTableImage->Bind();	// variable specularity in alpha channel
	} else {
		globalImages->specularTableImage->Bind();
	}
	for( /**/; surf; surf = surf->nextOnLight ) {
		// perform setup here that will not change over multiple interaction passes
		// set the vertex pointers
		idDrawVert	*ac = ( idDrawVert * )vertexCache.Position( surf->geo->ambientCache );
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
		glVertexAttribPointerARB( 11, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
		glVertexAttribPointerARB( 10, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
		glVertexAttribPointerARB( 9, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
		glVertexAttribPointerARB( 8, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
		glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
		// this may cause RB_ARB2_DrawInteraction to be executed multiple
		// times with different colors and images if the surface or light have multiple layers
		RB_CreateSingleDrawInteractions( surf, RB_ARB2_DrawInteraction );
	}
	glDisableVertexAttribArrayARB( 8 );
	glDisableVertexAttribArrayARB( 9 );
	glDisableVertexAttribArrayARB( 10 );
	glDisableVertexAttribArrayARB( 11 );
	glDisableClientState( GL_COLOR_ARRAY );
	// disable features
	GL_SelectTextureNoClient( 6 );
	globalImages->BindNull();
	GL_SelectTextureNoClient( 5 );
	globalImages->BindNull();
	GL_SelectTextureNoClient( 4 );
	globalImages->BindNull();
	GL_SelectTextureNoClient( 3 );
	globalImages->BindNull();
	GL_SelectTextureNoClient( 2 );
	globalImages->BindNull();
	GL_SelectTextureNoClient( 1 );
	globalImages->BindNull();
	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );
	glDisable( GL_VERTEX_PROGRAM_ARB );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );
}

/*
==================
RB_ARB2_DrawInteractions
==================
*/
void RB_ARB2_DrawInteractions( bool noshadows ) {
	viewLight_t			*vLight;
	const idMaterial	*lightShader;
	GL_SelectTexture( 0 );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	//
	// for each light, perform adding and shadowing
	//
	for( vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next ) {
		backEnd.vLight = vLight;
		// do fogging and custom light types later
		if( vLight->lightShader->IsFogLight() ) {
			continue;
		}
		if( vLight->lightShader->IsBlendLight() ) {
			continue;
		}
		if( !vLight->localInteractions && !vLight->globalInteractions
				&& !vLight->translucentInteractions ) {
			continue;
		}
		lightShader = vLight->lightShader;
		// clear the stencil buffer if needed
		if( vLight->globalShadows || vLight->localShadows ) {
			backEnd.currentScissor = vLight->scissorRect;
			if( r_useScissor.GetBool() ) {
				GL_Scissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
							backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
							backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
							backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
			}
			glClear( GL_STENCIL_BUFFER_BIT );
		} else {
			// no shadows, so no need to read or write the stencil buffer
			// we might in theory want to use GL_ALWAYS instead of disabling
			// completely, to satisfy the invarience rules
			glStencilFunc( GL_ALWAYS, 128, 255 );
		}
		if( r_useShadowVertexProgram.GetBool() ) {
			//common->Warning("%s", renderSystem->getDrawShadows() ? "true" : "false");
			if( noshadows ) {
				glEnable( GL_VERTEX_PROGRAM_ARB );
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
				RB_ARB2_CreateDrawInteractions( vLight->localInteractions );
				glEnable( GL_VERTEX_PROGRAM_ARB );
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
				RB_ARB2_CreateDrawInteractions( vLight->globalInteractions );
				glDisable( GL_VERTEX_PROGRAM_ARB );	// if there weren't any globalInteractions, it would have stayed on
			} else {
				glEnable( GL_VERTEX_PROGRAM_ARB );
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
				RB_StencilShadowPass( vLight->globalShadows );
				RB_ARB2_CreateDrawInteractions( vLight->localInteractions );
				glEnable( GL_VERTEX_PROGRAM_ARB );
				glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
				RB_StencilShadowPass( vLight->localShadows );
				RB_ARB2_CreateDrawInteractions( vLight->globalInteractions );
				glDisable( GL_VERTEX_PROGRAM_ARB );	// if there weren't any globalInteractions, it would have stayed on
			}
		} else {
			RB_StencilShadowPass( vLight->globalShadows );
			RB_ARB2_CreateDrawInteractions( vLight->localInteractions );
			RB_StencilShadowPass( vLight->localShadows );
			RB_ARB2_CreateDrawInteractions( vLight->globalInteractions );
		}
		// translucent surfaces never get stencil shadowed
		if( r_skipTranslucent.GetBool() ) {
			continue;
		}
		glStencilFunc( GL_ALWAYS, 128, 255 );
		backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
		RB_ARB2_CreateDrawInteractions( vLight->translucentInteractions );
		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	}
	// disable stencil shadow test
	glStencilFunc( GL_ALWAYS, 128, 255 );
	GL_SelectTexture( 0 );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
}

//===================================================================================

typedef struct {
	GLenum			target;
	GLuint			ident;
	char			name[64];
} progDef_t;

#define MAX_GLPROGS			200

// a single file can have both a vertex program and a fragment program
static progDef_t	progs[MAX_GLPROGS] = {
	{ GL_VERTEX_PROGRAM_ARB, VPROG_TEST, "test.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_TEST, "test.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_INTERACTION, "interaction.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_INTERACTION, "interaction.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_BUMPY_ENVIRONMENT, "bumpyEnvironment.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_BUMPY_ENVIRONMENT, "bumpyEnvironment.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_AMBIENT, "ambientLight.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_AMBIENT, "ambientLight.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW, "shadow.vp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_R200_INTERACTION, "R200_interaction.vp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_NV20_BUMP_AND_LIGHT, "nv20_bumpAndLight.vp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_NV20_DIFFUSE_COLOR, "nv20_diffuseColor.vp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_NV20_SPECULAR_COLOR, "nv20_specularColor.vp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_NV20_DIFFUSE_AND_SPECULAR_COLOR, "nv20_diffuseAndSpecularColor.vp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_ENVIRONMENT, "environment.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_ENVIRONMENT, "environment.vfp" },
	// revelator: this was probably intended to be here when megatexture support was finished.
	{ GL_VERTEX_PROGRAM_ARB, VPROG_MEGATEXTURE, "megaTexture.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_MEGATEXTURE, "megaTexture.vfp" },
	// rebb: direct light interaction files for performance testing
	{ GL_VERTEX_PROGRAM_ARB, VPROG_TEST_DIRECT, "test_direct.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_TEST_DIRECT, "test_direct.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_INTERACTION_DIRECT, "interaction_direct.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_INTERACTION_DIRECT, "interaction_direct.vfp" },
	// additional programs can be dynamically specified in materials
};

/*
=================
R_LoadARBProgram
=================
*/
void R_LoadARBProgram( int progIndex ) {
	idStr	fullPath = "glprogs/";
	fullPath += progs[progIndex].name;
	char	*fileBuffer;
	char	*buffer;
	char	*start, *end;
	// load the program even if we don't support it
	fileSystem->ReadFile( fullPath.c_str(), ( void ** )&fileBuffer, NULL );
	if( !fileBuffer ) {
		common->Warning( "LoadARBProgram: \'%s\' not found", fullPath.c_str() );
		return;
	}
	common->Printf( "%s", fullPath.c_str() );
	// copy to stack memory and free
	buffer = ( char * )_alloca( strlen( fileBuffer ) + 1 );
	strcpy( buffer, fileBuffer );
	fileSystem->FreeFile( fileBuffer );
	if( !glConfig.isInitialized ) {
		return;
	}
	//
	// submit the program string at start to GL
	//
	if( progs[progIndex].ident == 0 ) {
		// allocate a new identifier for this program
		progs[progIndex].ident = PROG_USER + progIndex;
	}
	// vertex and fragment programs can both be present in a single file, so
	// scan for the proper header to be the start point, and stamp a 0 in after the end
	start = NULL;
	if( progs[progIndex].target == GL_VERTEX_PROGRAM_ARB ) {
		if( !glConfig.ARBVertexProgramAvailable ) {
			common->Printf( S_COLOR_RED ": GL_VERTEX_PROGRAM_ARB not available\n" S_COLOR_DEFAULT );
			return;
		}
		start = strstr( ( char * )buffer, "!!ARBvp" );
	}
	if( progs[progIndex].target == GL_FRAGMENT_PROGRAM_ARB ) {
		if( !glConfig.ARBFragmentProgramAvailable ) {
			common->Printf( S_COLOR_RED ": GL_FRAGMENT_PROGRAM_ARB not available\n" S_COLOR_DEFAULT );
			return;
		}
		start = strstr( ( char * )buffer, "!!ARBfp" );
	}
	if( !start ) {
		common->Printf( S_COLOR_RED ": !!ARB not found\n"  S_COLOR_DEFAULT );
		return;
	}
	end = strstr( start, "END" );
	if( !end ) {
		common->Printf( S_COLOR_RED ": END not found\n"  S_COLOR_DEFAULT );
		return;
	}
	end[3] = 0;
	glBindProgramARB( progs[progIndex].target, progs[progIndex].ident );
	glProgramStringARB( progs[progIndex].target, GL_PROGRAM_FORMAT_ASCII_ARB, strlen( start ), ( unsigned char * )start );
	// this is pretty important for quick shader debugging, better have it in always
	int err = glGetError();
	int	ofs;
	glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, ( GLint * )&ofs );
	if( err == GL_INVALID_OPERATION ) {
		const GLubyte *str = glGetString( GL_PROGRAM_ERROR_STRING_ARB );
		common->Printf( "\nGL_PROGRAM_ERROR_STRING_ARB: %s\n", str );
		if( ofs < 0 ) {
			common->Printf( "GL_PROGRAM_ERROR_POSITION_ARB < 0 with error\n" );
		} else if( ofs >= ( int )strlen( ( char * )start ) ) {
			common->Printf( "error at end of program\n" );
		} else {
			common->Printf( "error at %i:\n%s", ofs, start + ofs );
		}
		return;
	}
	if( ofs != -1 ) {
		common->Printf( "\nGL_PROGRAM_ERROR_POSITION_ARB != -1 without error\n" );
		return;
	}
	common->Printf( "\n" );
}

/*
==================
R_FindARBProgram

Returns a GL identifier that can be bound to the given target, parsing
a text file if it hasn't already been loaded.
==================
*/
int R_FindARBProgram( GLenum target, const char *program ) {
	int		i;
	idStr	stripped = program;
	stripped.StripFileExtension();
	// see if it is already loaded
	for( i = 0; progs[i].name[0]; i++ ) {
		if( progs[i].target != target ) {
			continue;
		}
		idStr	compare = progs[i].name;
		compare.StripFileExtension();
		if( !idStr::Icmp( stripped.c_str(), compare.c_str() ) ) {
			return progs[i].ident;
		}
	}
	if( i == MAX_GLPROGS ) {
		common->Error( "R_FindARBProgram: MAX_GLPROGS" );
	}
	// add it to the list and load it
	progs[i].ident = ( program_t )0;	// will be gen'd by R_LoadARBProgram
	progs[i].target = target;
	strncpy( progs[i].name, program, sizeof( progs[i].name ) - 1 );
	R_LoadARBProgram( i );
	return progs[i].ident;
}

/*
==================
R_ReloadARBPrograms_f
==================
*/
void R_ReloadARBPrograms_f( const idCmdArgs &args ) {
	int		i;
	common->Printf( "----- R_ReloadARBPrograms -----\n" );
	for( i = 0; progs[i].name[0]; i++ ) {
		R_LoadARBProgram( i );
	}
	common->Printf( "-------------------------------\n" );
}

/*
==================
R_ARB2_Init

==================
*/
void R_ARB2_Init( void ) {
	common->Printf( "---------- R_ARB2_Init ----------\n" );
	if( !glConfig.ARBVertexProgramAvailable || !glConfig.ARBFragmentProgramAvailable ) {
		common->Printf( "Not available.\n" );
		return;
	}
	common->Printf( "Available.\n" );
	common->Printf( "---------------------------------\n" );
}