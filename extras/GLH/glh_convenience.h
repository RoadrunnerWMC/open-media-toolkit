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

#ifndef GLH_CONVENIENCE_H
#define GLH_CONVENIENCE_H

// Convenience methods for using glh_linear objects
// with opengl...



// debugging hack...
#include <iostream>

using namespace std;

#include <glh_linear.h>
#include <GL/gl.h>
#include <glh_ext.h>


namespace glh
{

  // per-vertex helpers

  inline void glColor(const vec3f & c) { glColor3fv(&c[0]); }
  inline void glColor(const vec4f & c) { glColor4fv(&c[0]); }

#ifdef GL_EXT_secondary_color

  inline void glSecondaryColor(const vec3f & c) { glSecondaryColor3fvEXT(&c[0]); }

#endif


  inline void glNormal(const vec3f & n) { glNormal3fv(&n[0]); }

  inline void glTexCoord(const GLfloat f) { glTexCoord1f (f);     }
  inline void glTexCoord(const vec2f & t) { glTexCoord2fv(&t[0]); }
  inline void glTexCoord(const vec3f & t) { glTexCoord3fv(&t[0]); }
  inline void glTexCoord(const vec4f & t) { glTexCoord4fv(&t[0]); }


#ifdef GL_ARB_multitexture

  inline void glMultiTexCoord(GLenum unit, const GLfloat f) { glMultiTexCoord1fARB (unit, f);     }
  inline void glMultiTexCoord(GLenum unit, const vec2f & t) { glMultiTexCoord2fvARB(unit, &t[0]); }
  inline void glMultiTexCoord(GLenum unit, const vec3f & t) { glMultiTexCoord3fvARB(unit, &t[0]); }
  inline void glMultiTexCoord(GLenum unit, const vec4f & t) { glMultiTexCoord4fvARB(unit, &t[0]); }

#endif


  inline void glVertex(const vec2f & v) { glVertex2fv(&v[0]); }
  inline void glVertex(const vec3f & v) { glVertex3fv(&v[0]); }
  inline void glVertex(const vec4f & v) { glVertex4fv(&v[0]); }

  // lighting helpers

  inline void glMaterial(GLenum face, GLenum pname, GLint i)
  { glMateriali(face, pname, i); }

  inline void glMaterial(GLenum face, GLenum pname, GLfloat f)
  { glMaterialf(face, pname, f); }

  inline void glMaterial(GLenum face, GLenum pname, const vec4f & v)
  { glMaterialfv(face, pname, &v[0]); }

  inline void glLight(GLenum light, GLenum pname, GLint i)
  { glLighti(light, pname, i); } 

  inline void glLight(GLenum light, GLenum pname, GLfloat f)
  { glLightf(light, pname, f); } 

  inline void glLight(GLenum light, GLenum pname, const vec4f & v)
  { glLightfv(light, pname, &v[0]); } 

  inline void glLight(GLenum light, GLenum pname, const vec3f & v, float wa = 1)
  {   vec4f v4(v[0], v[1], v[2], wa); glLightfv(light, pname, &v4[0]); } 


  // matrix helpers

  inline matrix4f get_matrix(GLenum matrix) 
  {
	GLfloat m[16];
	glGetFloatv(matrix, m);
	return matrix4f(m);
  }

  inline void glLoadMatrix(const matrix4f & m)
  {
	glLoadMatrixf(m.get_value());	
  }

  inline void glMultMatrix(const matrix4f & m)
  {
	glMultMatrixf(m.get_value());	
  }

  // transform helpers

  inline void glRotate(const quaternionf & r)
  {
	float angle;
	vec3f axis;
	r.get_value(axis, angle);
	glRotatef(to_degrees(angle), axis[0], axis[1], axis[2]);
  }

  inline void glTranslate(const vec3f & t) { glTranslatef(t[0], t[1], t[2]); }

  inline void glScale    (const vec3f & s) { glScalef(s[0], s[1], s[2]); }



  // inverse of camera_lookat
  inline matrix4f object_lookat(const vec3f & from, const vec3f & to, const vec3f & Up)
  {
	  vec3f look = to - from;
	  look.normalize();
	  vec3f up(Up);
	  up -= look * look.dot(up);
	  up.normalize();
	  
	  quaternionf r(vec3f(0,0,-1), vec3f(0,1,0), look, up);
	  matrix4f m;
	  r.get_value(m);
	  m.set_translate(from);
	  return m;
  }


  // inverse of object_lookat
  inline matrix4f camera_lookat(const vec3f & eye, const vec3f & lookpoint, const vec3f & Up)
  {
	  vec3f look = lookpoint - eye;
	  look.normalize();
	  vec3f up(Up);
	  up -= look * look.dot(up);
	  up.normalize();

	  matrix4f t;
	  t.set_translate(-eye);

	  quaternionf r(vec3f(0,0,-1), vec3f(0,1,0), look, up);
	  r.invert();
	  matrix4f rm;
	  r.get_value(rm);
	  return rm*t;	  
  }


  inline matrix4f frustum(float left, float right,
				   float bottom, float top,
				   float zNear, float zFar)
  {
	matrix4f m;
	m.make_identity();

	m(0,0) = (2*zNear) / (right - left);
	m(0,2) = (right + left) / (right - left);
	
	m(1,1) = (2*zNear) / (top - bottom);
	m(1,2) = (top + bottom) / (top - bottom);
	
	m(2,2) = -(zFar + zNear) / (zFar - zNear);
	m(2,3) = -2*zFar*zNear / (zFar - zNear);
   
	m(3,2) = -1;
	m(3,3) = 0;

	return m;
  }

  inline matrix4f frustum_inverse(float left, float right,
						   float bottom, float top,
						   float zNear, float zFar)
  {
	matrix4f m;
	m.make_identity();

	m(0,0) = (right - left) / (2 * zNear);
	m(0,3) = (right + left) / (2 * zNear);
	
	m(1,1) = (top - bottom) / (2 * zNear);
	m(1,3) = (top + bottom) / (2 * zNear);

	m(2,2) = 0;
	m(2,3) = -1;
	
	m(3,2) = -(zFar - zNear) / (2 * zFar * zNear);
	m(3,3) = (zFar + zNear) / (2 * zFar * zNear);

	return m;
  }

  inline matrix4f perspective(float fovy, float aspect, float zNear, float zFar)
  {
	double tangent = tan(to_radians(fovy/2.0));
	float y = tangent * zNear;
	float x = aspect * y;
	return frustum(-x, x, -y, y, zNear, zFar);
  }

  inline matrix4f perspective_inverse(float fovy, float aspect, float zNear, float zFar)
  {
	double tangent = tan(to_radians(fovy/2.0));
	float y = tangent * zNear;
	float x = aspect * y;
	return frustum_inverse(-x, x, -y, y, zNear, zFar);
  }



  // are these names ok?

  inline void set_texgen_planes(GLenum plane_type, const matrix4f & m)
  {
	  GLenum coord[] = {GL_S, GL_T, GL_R, GL_Q };
	  for(int i = 0; i < 4; i++)
          {
          vec4f row;
          m.get_row(i,row);
		  glTexGenfv(coord[i], plane_type, &row[0]);
          }
  }

	
  inline void texgen_mode(GLenum coord, GLint param)
  { glTexGeni(coord, GL_TEXTURE_GEN_MODE, param); }

  inline void texenv_mode(GLint m)
  { glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m); }





  // "get" (state query) helpers



  inline GLfloat glGetFloat(GLenum pname)
  {
	  GLfloat v;
	  glGetFloatv(pname, &v);
	  return v;
  }



  inline GLint glGetInteger(GLenum pname)

  {
	  GLint i;
	  glGetIntegerv(pname, &i);
	  return i;
  }



  inline vec4f get_vec4f(GLenum pname)
  {
	  vec4f v;
	  glGetFloatv(pname, &v[0]);
	  return v;
  }


  // handy for register combiners
  inline vec3f range_compress(const vec3f & v)
  { vec3f vret(v); vret *= .5f; vret += .5f; return vret; }
  
  inline vec3f range_uncompress(const vec3f & v)
  { vec3f vret(v); vret -= .5f; vret *= 2.f; return vret; }

  
#ifdef GL_NV_register_combiners

	inline void glCombinerParameterNV(GLenum param, GLfloat * v)
	{ glCombinerParameterfvNV(param, v); }
	inline void glCombinerParameterNV(GLenum param, const vec4f & v)
	{ glCombinerParameterfvNV(param, & v[0]); }
	inline void glCombinerParameterNV(GLenum param, const vec3f & v, float a)
	{ vec4f v4(v[0], v[1], v[2], a); glCombinerParameterNV(param, v4); }
	inline void glCombinerParameterNV(GLenum param, GLfloat f)
	{ glCombinerParameterfNV(param, f); }

#endif



} // namespace glh

#endif
