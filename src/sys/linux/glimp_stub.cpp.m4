// glimp_stub.cpp.m4
// stub gl/glX APIs

#include "idlib/precompiled.h"
#include "renderer/tr_local.h"
#pragma hdrstop

dnl =====================================================
dnl glX stubs
dnl =====================================================

GLenum glGetError(void){return 0;}

GLuint glGenLists(GLsizei range){return 0;}

void glGetIntegerv(GLenum pname, GLint *params){
	switch( pname ) {
		case GL_MAX_TEXTURE_SIZE: *params = 1024; break;
		case GL_MAX_TEXTURE_UNITS_ARB: *params = 2; break;
		default: *params = 0; break;
	}
}

const GLubyte * glGetString(GLenum name){
	switch( name ) {
		case GL_EXTENSIONS: return (GLubyte *)"GL_ARB_multitexture GL_ARB_texture_env_combine GL_ARB_texture_cube_map GL_ARB_texture_env_dot3";
	}
	return (const GLubyte *)"";
}
