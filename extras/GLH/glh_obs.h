/*
    glh - is a platform-indepenedent C++ OpenGL helper library 


    Copyright (c) 2000 Cass Everitt
	Copyright (c) 2000 NVIDIA Corporation
    All rights reserved.

    Redistribution and use in source and binary forms, with or
	without modification, are permitted provided that the following
	conditions are met:

     * Redistributions of source code must retain the above
	   copyright notice, this list of conditions and the following
	   disclaimer.

     * Redistributions in binary form must reproduce the above
	   copyright notice, this list of conditions and the following
	   disclaimer in the documentation and/or other materials
	   provided with the distribution.

     * The names of contributors to this software may not be used
	   to endorse or promote products derived from this software
	   without specific prior written permission. 

       THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	   POSSIBILITY OF SUCH DAMAGE. 


    Cass Everitt - cass@r3.nu
*/

// This is a file for simple GL helper classes...

#ifndef GLH_OBS_H
#define GLH_OBS_H

#ifdef WIN32
# include <windows.h>
#endif

#include <GL/gl.h>
#include <glext.h>

namespace glh
{
	class display_list
	{
	public:
		display_list() 
			: valid(false) {}
		
		virtual ~display_list()
		{ del(); }
		
		void call_list()
		{ if(valid) glCallList(dlist); }
		
		void new_list(GLenum mode)
		{ if(!valid) gen(); glNewList(dlist, mode); }
		
		void end_list()
		{ glEndList(); }
		
		void del()
		{ if(valid) glDeleteLists(dlist, 1); valid = false; }
		
		bool is_valid() const { return valid; }
		
	private:
		
		void gen() { dlist = glGenLists(1); valid=true; }
		
		bool valid;
		GLuint dlist;
	};
	
	
	class tex_object
	{
	public:
		tex_object(GLenum tgt) 
			: target(tgt), valid(false) {}
		
		virtual ~tex_object()
		{ del(); }
		
		void bind()
		{ if(!valid) gen(); glBindTexture(target, texture); }
		
		void unbind()
		{ glBindTexture(target, 0); }


		// convenience methods

		void parameter(GLenum pname, GLint i)

		{ glTexParameteri(target, pname, i); }



		void parameter(GLenum pname, GLfloat f)

		{ glTexParameterf(target, pname, f); }



		void parameter(GLenum pname, GLint * ip)

		{ glTexParameteriv(target, pname, ip); }



		void parameter(GLenum pname, GLfloat * fp)

		{ glTexParameterfv(target, pname, fp); }



		void enable() { glEnable(target); }

		void disable() { glDisable(target); }


		void del()
		{ if(valid) glDeleteTextures(1, &texture); valid = false; }
		
		bool is_valid() const { return valid; }
		
	private:
		
		void gen() { glGenTextures(1, &texture); valid=true; }
		
		bool valid;
		GLuint texture;
		GLenum target;
	};
	
	class tex_object_1D : public tex_object
	{ public: tex_object_1D() : tex_object(GL_TEXTURE_1D) {} };
	
	class tex_object_2D : public tex_object
	{ public: tex_object_2D() : tex_object(GL_TEXTURE_2D) {} };


# ifdef GL_ARB_texture_cube_map
	class tex_object_cube_map : public tex_object
	{ public: tex_object_cube_map() : tex_object(GL_TEXTURE_CUBE_MAP_ARB) {} };
# else if GL_EXT_texture_cube_map
	class tex_object_cube_map : public tex_object
	{ public: tex_object_cube_map() : tex_object(GL_TEXTURE_CUBE_MAP_EXT) {} };
# endif
}
#endif