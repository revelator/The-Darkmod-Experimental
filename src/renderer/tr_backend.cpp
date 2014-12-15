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

frameData_t		*frameData;
backEndState_t	backEnd;

/*
======================
RB_SetDefaultGLState

This should initialize all GL state that any part of the entire program
may touch, including the editor.
======================
*/
void RB_SetDefaultGLState( void ) {
	RB_LogComment( "--- R_SetDefaultGLState ---\n" );
	glClearDepth( 1.0f );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	// the vertex array is always enabled
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	// make sure our GL state vector is set correctly
	memset( &backEnd.glState, 0, sizeof( backEnd.glState ) );
	backEnd.glState.forceGlState = true;
	glColorMask( 1, 1, 1, 1 );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glEnable( GL_SCISSOR_TEST );
	glEnable( GL_CULL_FACE );
	glDisable( GL_LIGHTING );
	glDisable( GL_LINE_STIPPLE );
	glDisable( GL_STENCIL_TEST );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_ALWAYS );
	glCullFace( GL_FRONT_AND_BACK );
	glShadeModel( GL_SMOOTH );
	if( r_useScissor.GetBool() ) {
		GL_Scissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	}
	for( int i = glConfig.maxTextureUnits - 1; i >= 0; i-- ) {
		GL_SelectTexture( i );
		// object linear texgen is our default
		glTexGenf( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGenf( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGenf( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGenf( GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		GL_TexEnv( GL_MODULATE );
		glDisable( GL_TEXTURE_2D );
		if( glConfig.texture3DAvailable ) {
			glDisable( GL_TEXTURE_3D );
		}
		if( glConfig.cubeMapAvailable ) {
			glDisable( GL_TEXTURE_CUBE_MAP_EXT );
		}
	}
}

/*
====================
RB_LogComment
====================
*/
void RB_LogComment( const char *comment, ... ) {
	if( !tr.logFile ) {
		return;
	}
	va_list marker;
	fprintf( tr.logFile, "// " );
	va_start( marker, comment );
	vfprintf( tr.logFile, comment, marker );
	va_end( marker );
}

//=============================================================================

/*
====================
GL_SelectTexture
====================
*/
void GL_SelectTexture( const int unit ) {
	if( backEnd.glState.currenttmu == unit ) {
		return;
	}
	if( unit < 0 || ( unit >= glConfig.maxTextureUnits && unit >= glConfig.maxTextureImageUnits ) ) {
		common->Warning( "GL_SelectTexture: unit = %i", unit );
		return;
	}
	glActiveTextureARB( GL_TEXTURE0_ARB + unit );
	glClientActiveTextureARB( GL_TEXTURE0_ARB + unit );
	RB_LogComment( "glActiveTextureARB( %i );\nglClientActiveTextureARB( %i );\n", unit, unit );
	backEnd.glState.currenttmu = unit;
}

/*
====================
GL_Cull

This handles the flipping needed when the view being
rendered is a mirored view.
====================
*/
void GL_Cull( const int cullType ) {
	if( backEnd.glState.faceCulling == cullType ) {
		return;
	}
	if( cullType == CT_TWO_SIDED ) {
		glDisable( GL_CULL_FACE );
	} else {
		if( backEnd.glState.faceCulling == CT_TWO_SIDED ) {
			glEnable( GL_CULL_FACE );
		}
		if( cullType == CT_BACK_SIDED ) {
			if( backEnd.viewDef->isMirror ) {
				glCullFace( GL_FRONT );
			} else {
				glCullFace( GL_BACK );
			}
		} else {
			if( backEnd.viewDef->isMirror ) {
				glCullFace( GL_BACK );
			} else {
				glCullFace( GL_FRONT );
			}
		}
	}
	backEnd.glState.faceCulling = cullType;
}

/*
====================
GL_Scissor
====================
*/
void GL_Scissor( int x /* left*/, int y /* bottom */, int w, int h ) {
	glScissor( x, y, w, h );
}

/*
====================
GL_Viewport
====================
*/
void GL_Viewport( int x /* left */, int y /* bottom */, int w, int h ) {
	glViewport( x, y, w, h );
}

/*
====================
GL_TexEnv
====================
*/
void GL_TexEnv( int env ) {
	tmu_t *tmu = &backEnd.glState.tmu[backEnd.glState.currenttmu];
	if( env == tmu->texEnv ) {
		return;
	}
	tmu->texEnv = env;
	if( env & ( GL_COMBINE_EXT | GL_MODULATE | GL_REPLACE | GL_DECAL | GL_ADD ) ) {
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, env );
	} else {
		common->Error( "GL_TexEnv: invalid env '%d' passed\n", env );
	}
}

/*
=================
GL_ClearStateDelta

Clears the state delta bits, so the next GL_State
will set every item
=================
*/
void GL_ClearStateDelta( void ) {
	backEnd.glState.forceGlState = true;
}

/*
====================
GL_State

This routine is responsible for setting the most commonly changed state
====================
*/
void GL_State( const int stateBits ) {
#if 1
	int diff;
	if( !r_useStateCaching.GetBool() || backEnd.glState.forceGlState ) {
		// make sure everything is set all the time, so we
		// can see if our delta checking is screwing up
		diff = -1;
		backEnd.glState.forceGlState = false;
	} else {
		diff = stateBits ^ backEnd.glState.glStateBits;
		if( !diff ) {
			return;
		}
	}
#else
	// angua: this caused light gem problems (lg changed based on view angle)
	// it's important to set diff to -1 if force gl state is true
	const int diff = stateBits ^ backEnd.glState.glStateBits;
	if( !diff ) {
		return;
	}
	if( backEnd.glState.forceGlState ) {
		backEnd.glState.forceGlState = false;
	}
#endif
	// check depthFunc bits
	if( diff & ( GLS_DEPTHFUNC_EQUAL | GLS_DEPTHFUNC_LESS | GLS_DEPTHFUNC_ALWAYS ) ) {
		if( stateBits & GLS_DEPTHFUNC_EQUAL ) {
			glDepthFunc( GL_EQUAL );
		} else if( stateBits & GLS_DEPTHFUNC_ALWAYS ) {
			glDepthFunc( GL_ALWAYS );
		} else {
			glDepthFunc( GL_LEQUAL );
		}
	}
	// check blend bits
	if( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) {
		GLenum srcFactor, dstFactor;
		switch( stateBits & GLS_SRCBLEND_BITS ) {
		case GLS_SRCBLEND_ONE:
			srcFactor = GL_ONE;
			break;
		case GLS_SRCBLEND_ZERO:
			srcFactor = GL_ZERO;
			break;
		case GLS_SRCBLEND_DST_COLOR:
			srcFactor = GL_DST_COLOR;
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
			srcFactor = GL_ONE_MINUS_DST_COLOR;
			break;
		case GLS_SRCBLEND_SRC_ALPHA:
			srcFactor = GL_SRC_ALPHA;
			break;
		case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
			srcFactor = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case GLS_SRCBLEND_DST_ALPHA:
			srcFactor = GL_DST_ALPHA;
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
			srcFactor = GL_ONE_MINUS_DST_ALPHA;
			break;
		case GLS_SRCBLEND_ALPHA_SATURATE:
			srcFactor = GL_SRC_ALPHA_SATURATE;
			break;
		default:
			srcFactor = GL_ONE;		// to get warning to shut up
			common->Error( "GL_State: invalid src blend state bits\n" );
			break;
		}
		switch( stateBits & GLS_DSTBLEND_BITS ) {
		case GLS_DSTBLEND_ZERO:
			dstFactor = GL_ZERO;
			break;
		case GLS_DSTBLEND_ONE:
			dstFactor = GL_ONE;
			break;
		case GLS_DSTBLEND_SRC_COLOR:
			dstFactor = GL_SRC_COLOR;
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
			dstFactor = GL_ONE_MINUS_SRC_COLOR;
			break;
		case GLS_DSTBLEND_SRC_ALPHA:
			dstFactor = GL_SRC_ALPHA;
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
			dstFactor = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case GLS_DSTBLEND_DST_ALPHA:
			dstFactor = GL_DST_ALPHA;
			break;
		case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
			dstFactor = GL_ONE_MINUS_DST_ALPHA;
			break;
		default:
			dstFactor = GL_ONE;		// to get warning to shut up
			common->Error( "GL_State: invalid dst blend state bits\n" );
			break;
		}
		glBlendFunc( srcFactor, dstFactor );
	}
	// check depthmask
	if( diff & GLS_DEPTHMASK ) {
		if( stateBits & GLS_DEPTHMASK ) {
			glDepthMask( GL_FALSE );
		} else {
			glDepthMask( GL_TRUE );
		}
	}
	// check colormask
	if( diff & ( GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK | GLS_ALPHAMASK ) ) {
		glColorMask(
			!( stateBits & GLS_REDMASK ),
			!( stateBits & GLS_GREENMASK ),
			!( stateBits & GLS_BLUEMASK ),
			!( stateBits & GLS_ALPHAMASK )
		);
	}
	// fill/line mode
	if( diff & GLS_POLYMODE_LINE ) {
		if( stateBits & GLS_POLYMODE_LINE ) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		} else {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}
	// alpha test
	if( diff & GLS_ATEST_BITS ) {
		switch( stateBits & GLS_ATEST_BITS ) {
		case 0:
			glDisable( GL_ALPHA_TEST );
			break;
		case GLS_ATEST_EQ_255:
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_EQUAL, 1 );
			break;
		case GLS_ATEST_LT_128:
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_LESS, 0.5 );
			break;
		case GLS_ATEST_GE_128:
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GEQUAL, 0.5 );
			break;
		default:
			assert( 0 );
			break;
		}
	}
	backEnd.glState.glStateBits = stateBits;
}

/*
============================================================================

RENDER BACK END COLOR WRAPPERS

============================================================================
*/

/*
====================
GL_Color
====================
*/
void GL_Color( const idVec3 &color ) {
	GLfloat parm[4];
	parm[0] = idMath::ClampFloat( 0.0f, 1.0f, color[0] );
	parm[1] = idMath::ClampFloat( 0.0f, 1.0f, color[1] );
	parm[2] = idMath::ClampFloat( 0.0f, 1.0f, color[2] );
	GL_Color( parm[0], parm[1], parm[2] );
}

/*
====================
GL_Color
====================
*/
void GL_Color( const idVec4 &color ) {
	GLfloat parm[4];
	parm[0] = idMath::ClampFloat( 0.0f, 1.0f, color[0] );
	parm[1] = idMath::ClampFloat( 0.0f, 1.0f, color[1] );
	parm[2] = idMath::ClampFloat( 0.0f, 1.0f, color[2] );
	parm[3] = idMath::ClampFloat( 0.0f, 1.0f, color[3] );
	glColor4f( parm[0], parm[1], parm[2], parm[3] );
}

/*
====================
GL_Color
====================
*/
void GL_Color( float r, float g, float b ) {
	GLfloat parm[3];
	parm[0] = idMath::ClampFloat( 0.0f, 1.0f, r );
	parm[1] = idMath::ClampFloat( 0.0f, 1.0f, g );
	parm[2] = idMath::ClampFloat( 0.0f, 1.0f, b );
	glColor3f( parm[0], parm[1], parm[2] );
}

/*
====================
GL_Color
====================
*/
void GL_Color( float r, float g, float b, float a ) {
	GLfloat parm[4];
	parm[0] = idMath::ClampFloat( 0.0f, 1.0f, r );
	parm[1] = idMath::ClampFloat( 0.0f, 1.0f, g );
	parm[2] = idMath::ClampFloat( 0.0f, 1.0f, b );
	parm[3] = idMath::ClampFloat( 0.0f, 1.0f, a );
	glColor4f( parm[0], parm[1], parm[2], parm[3] );
}

/*
====================
GL_Color
====================
*/
void GL_Color( byte r, byte g, byte b ) {
	GLubyte parm[3];
	parm[0] = idMath::ClampByte( 0, 255, r );
	parm[1] = idMath::ClampByte( 0, 255, g );
	parm[2] = idMath::ClampByte( 0, 255, b );
	glColor3ub( parm[0], parm[1], parm[2] );
}

/*
====================
GL_Color
====================
*/
void GL_Color( byte r, byte g, byte b, byte a ) {
	GLubyte parm[4];
	parm[0] = idMath::ClampByte( 0, 255, r );
	parm[1] = idMath::ClampByte( 0, 255, g );
	parm[2] = idMath::ClampByte( 0, 255, b );
	parm[3] = idMath::ClampByte( 0, 255, a );
	glColor4ub( parm[0], parm[1], parm[2], parm[3] );
}

/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
=============
RB_SetGL2D

This is not used by the normal game paths, just by some tools
=============
*/
void RB_SetGL2D( void ) {
	// set 2D virtual screen size
	glViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	if( r_useScissor.GetBool() ) {
		GL_Scissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
	}
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, 640, 480, 0, 0, 1 );		// always assume 640x480 virtual coordinates
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	GL_State( GLS_DEPTHFUNC_ALWAYS |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
	GL_Cull( CT_TWO_SIDED );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_STENCIL_TEST );
}

/*
=============
RB_SetBuffer

=============
*/
static void	RB_SetBuffer( const void *data ) {
	const setBufferCommand_t	*cmd;
	// see which draw buffer we want to render the frame to
	cmd = ( const setBufferCommand_t * )data;
	backEnd.frameCount = cmd->frameCount;
	glDrawBuffer( cmd->buffer );
	// clear screen for debugging
	// automatically enable this with several other debug tools
	// that might leave unrendered portions of the screen
	if( r_clear.GetFloat() || idStr::Length( r_clear.GetString() ) != 1 || r_lockSurfaces.GetBool() || r_singleArea.GetBool() || r_showOverDraw.GetBool() ) {
		float c[3];
		if( sscanf( r_clear.GetString(), "%f %f %f", &c[0], &c[1], &c[2] ) == 3 ) {
			glClearColor( c[0], c[1], c[2], 1 );
		} else if( r_clear.GetInteger() == 2 ) {
			glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		} else if( r_showOverDraw.GetBool() ) {
			glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
		} else {
			glClearColor( 0.4f, 0.0f, 0.25f, 1.0f );
		}
		glClear( GL_COLOR_BUFFER_BIT );
	}
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.
===============
*/
void RB_ShowImages( void ) {
	idImage	*image;
	float	x, y, w, h;
	//int		start, end;
	// Serp - Disabled in gpl - draw with grey background
	//RB_SetGL2D();
	//glClearColor( 0.2, 0.2, 0.2, 1 );
	//glClear( GL_COLOR_BUFFER_BIT );
	//glFinish();
	//start = Sys_Milliseconds();
	for( int i = 0; i < globalImages->images.Num(); i++ ) {
		image = globalImages->images[i];
		if( image->texnum == idImage::TEXTURE_NOT_LOADED && image->partialImage == NULL ) {
			continue;
		}
		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;
		// show in proportional size in mode 2
		if( r_showImages.GetInteger() == 2 ) {
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}
		image->Bind();
		glBegin( GL_QUADS );
		glTexCoord2f( 0, 0 );
		glVertex2f( x, y );
		glTexCoord2f( 1, 0 );
		glVertex2f( x + w, y );
		glTexCoord2f( 1, 1 );
		glVertex2f( x + w, y + h );
		glTexCoord2f( 0, 1 );
		glVertex2f( x, y + h );
		glEnd();
	}
	glFinish();
	//end = Sys_Milliseconds();
	//Serp : This was enabled in gpl, it's fairly annoying however.
	// You will need to uncomment the vars above.
	//common->Printf( "%i msec to draw all images\n", end - start );
}

/*
=============
RB_SwapBuffers

=============
*/
const void	RB_SwapBuffers( const void *data ) {
	// texture swapping test
	if( r_showImages.GetInteger() != 0 ) {
		RB_ShowImages();
	}
	// force a gl sync if requested
	if( r_finish.GetBool() ) {
		glFinish();
	}
	RB_LogComment( "***************** RB_SwapBuffers *****************\n" );
	// don't flip if drawing to front buffer
	if( !r_frontBuffer.GetBool() ) {
		GLimp_SwapBuffers();
	}
}

/*
=============
RB_CopyRender

Copy part of the current framebuffer to an image
=============
*/
const void	RB_CopyRender( const void *data ) {
	if( r_skipCopyTexture.GetBool() ) {
		return;
	}
	const copyRenderCommand_t *cmd = ( copyRenderCommand_t * )data;
	RB_LogComment( "***************** RB_CopyRender *****************\n" );
	if( cmd->image ) {
		cmd->image->CopyFramebuffer( cmd->x, cmd->y, cmd->imageWidth, cmd->imageHeight, false );
	}
}

/*
====================
RB_ExecuteBackEndCommands

This function will be called syncronously if running without
smp extensions, or asyncronously by another thread.
====================
*/
void RB_ExecuteBackEndCommands( const emptyCommand_t *cmds ) {
	static int backEndStartTime, backEndFinishTime;
	if( cmds->commandId == RC_NOP && !cmds->next ) {
		return;
	}
	// r_debugRenderToTexture
	int	c_draw3d = 0, c_draw2d = 0, c_setBuffers = 0, c_swapBuffers = 0, c_copyRenders = 0;
	backEndStartTime = Sys_Milliseconds();
	// needed for editor rendering
	RB_SetDefaultGLState();
	// upload any image loads that have completed
	globalImages->CompleteBackgroundImageLoads();
	while( cmds ) {
		switch( cmds->commandId ) {
		case RC_NOP:
			break;
		case RC_DRAW_VIEW:
			RB_DrawView( cmds );
			if( ( ( const drawSurfsCommand_t * )cmds )->viewDef->viewEntitys ) {
				c_draw3d++;
			} else {
				c_draw2d++;
			}
			break;
		case RC_SET_BUFFER:
			RB_SetBuffer( cmds );
			c_setBuffers++;
			break;
		case RC_COPY_RENDER:
			RB_CopyRender( cmds );
			c_copyRenders++;
			break;
		case RC_SWAP_BUFFERS:
			RB_SwapBuffers( cmds );
			c_swapBuffers++;
			break;
		default:
			common->Error( "RB_ExecuteBackEndCommands: bad commandId" );
			break;
		}
		cmds = ( const emptyCommand_t * )cmds->next;
	}
	// go back to the default texture so the editor doesn't mess up a bound image
	glBindTexture( GL_TEXTURE_2D, 0 );
	backEnd.glState.tmu[0].current2DMap = -1;
	// stop rendering on this thread
	backEndFinishTime = Sys_Milliseconds();
	backEnd.pc.msec = backEndFinishTime - backEndStartTime;
	if( r_debugRenderToTexture.GetInteger() ) {
		common->Printf( "3d: %i, 2d: %i, SetBuf: %i, SwpBuf: %i, CpyRenders: %i, CpyFrameBuf: %i\n", c_draw3d, c_draw2d, c_setBuffers, c_swapBuffers, c_copyRenders, backEnd.c_copyFrameBuffer );
		backEnd.c_copyFrameBuffer = 0;
	}
}