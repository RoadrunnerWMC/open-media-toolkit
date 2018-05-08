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

#ifndef GLH_GLUT_H
#define GLH_GLUT_H

// some helper functions and object to
// make writing simple glut programs even easier! :-)

#include <algorithm>
#include <vector>
#include <GL/glut.h>
#include <glh_convenience.h>

namespace glh
{

  class glut_interactor
  {
  public:
	glut_interactor() {}
	virtual void keyboard(unsigned char key, int x, int y) {}
	virtual void mouse(int button, int state, int x, int y) {}
	virtual void motion(int x, int y) {}

	virtual void reshape(int w, int h) {}
  };

  std::vector<glut_interactor *> interactors;

  void glut_motion_function(int x, int y)
  {
	for(int i=0; i < interactors.size(); i++)
		interactors[i]->motion(x, y);
  }

  void glut_mouse_function(int button, int state, int x, int y)
  {
	for(int i=0; i < interactors.size(); i++)
	  interactors[i]->mouse(button, state, x, y);
  }

  void glut_keyboard_function(unsigned char k, int x, int y)
  {
	for(int i=0; i < interactors.size(); i++)
	  interactors[i]->keyboard(k, x, y);
  }


  void glut_reshape_function(int w, int h)

  {

	for(int i=0; i < interactors.size(); i++)

	  interactors[i]->reshape(w,h);

  }
  void glut_helpers_initialize()
  {
	glutMouseFunc(glut_mouse_function);
	glutMotionFunc(glut_motion_function);
	glutKeyboardFunc(glut_keyboard_function);

	glutReshapeFunc(glut_reshape_function);
  }

  void glut_remove_interactor(glut_interactor *gi)
  {
	  std::vector<glut_interactor *>::iterator it = 
		  std::find(interactors.begin(), interactors.end(), gi);
	if(it != interactors.end())
	  interactors.erase(it);
  }

  void glut_add_interactor(glut_interactor *gi)
  {
	glut_remove_interactor(gi);
	interactors.push_back(gi);
  }



  class glut_callbacks : public glut_interactor
  {
  public:
	glut_callbacks()
	  : keyboard_function(0),
		motion_function(0),
		mouse_function(0),

		reshape_function(0)
	{}

	virtual void keyboard(unsigned char k, int x, int y)
	{ if(keyboard_function) keyboard_function(k, x, y); }
	virtual void motion(int x, int y)
	{ if(motion_function) motion_function(x,y); }
	virtual void mouse(int button, int state, int x, int y)
	{ if(mouse_function) mouse_function(button, state, x, y); }

	virtual void reshape(int w, int h)

	{ if(reshape_function) reshape_function(w, h); }

	void (*keyboard_function) (unsigned char, int, int);
	void (*motion_function) (int, int);
	void (*mouse_function)(int, int, int, int);

	void (*reshape_function)(int, int);

  };



  class glut_perspective_reshaper : public glut_interactor
  {
  public:
	  glut_perspective_reshaper(float infovy = 60.f, float inzNear = .1f, float inzFar = 10.f)
		  : fovy(infovy), zNear(inzNear), zFar(inzFar) {}
	  
	  void reshape(int w, int h)
	  {
		  width = w; height = h;
		  aspect = float(w)/float(h);
		  glViewport(0,0,w,h);
		  glMatrixMode(GL_PROJECTION);
		  glLoadIdentity();
		  gluPerspective(fovy, aspect, zNear, zFar);
		  glMatrixMode(GL_MODELVIEW);
	  }
	  int width, height;
	  float fovy, aspect, zNear, zFar;
  };

  // activates/deactivates on a particular mouse button
  // and calculates deltas while active
  class glut_simple_interactor : public glut_interactor
  {
  public:
	glut_simple_interactor()
	{
	  activate_on = GLUT_LEFT_BUTTON;
	  active = false;
	  use_modifiers = true;
	  modifiers = 0;
	  x0 = y0 = x = y = dx = dy = 0;
	}

	virtual void mouse(int button, int state, int X, int Y)
	{
	  if(button == activate_on && state == GLUT_DOWN && 
		 (! use_modifiers || (modifiers == glutGetModifiers())) )
	  {
		  active = true;
		  x = x0 = X;
		  y = y0 = Y;
		  dx = dy = 0;
	  }
	  else if (button == activate_on && state == GLUT_UP)
	  {
		active = false;
		dx = dy = 0;
	  }
	}

	virtual void motion(int X, int Y)
	{
	  if(active)
	  {
		dx = X - x;   dy = y - Y;
		x = X;   y = Y;
		update();
	  }
	}

	virtual void apply_transform() = 0; 
	virtual void apply_inverse_transform() = 0;
	virtual matrix4f get_transform() = 0;
	virtual matrix4f get_inverse_transform() = 0;

	virtual void update() {}

	int activate_on;
	bool use_modifiers;
	int modifiers;
	bool active;
	int x0, y0;
	int x, y;
	int dx, dy;
  };


  class glut_pan : public glut_simple_interactor
  {
  public:
	glut_pan()
	{
	  glScale = .01f;
	  invert_increment = false;
	  parent_rotation = 0;
	}
	void update()
	{
	  vec3f v(dx, dy, 0);
	  if(parent_rotation != 0) parent_rotation->mult_vec(v);

	  if(invert_increment)
		  pan -= v * glScale;
	  else
		  pan += v * glScale;
	  glutPostRedisplay();
	}

	void apply_transform()
	{
	  //cerr << "Applying transform: " << (x - x0) << ", " << (y - y0) << endl;
	  glTranslate(pan);
	}
	
	void apply_inverse_transform()
	{
	  //cerr << "Applying transform: " << (x - x0) << ", " << (y - y0) << endl;
	  glTranslate(-pan);
	}

	matrix4f get_transform()
	{
		matrix4f m;
		m.make_identity();
		m.set_translate(pan);
		return m;
	}	

	matrix4f get_inverse_transform()
	{
		matrix4f m;
		m.make_identity();
		m.set_translate(-pan);
		return m;
	}	


	bool invert_increment;
	const rotationf * parent_rotation;
	vec3f pan;
	float glScale;
  };

  
  class glut_dolly : public glut_simple_interactor
  {
  public:
	glut_dolly()
	{
	  glScale = .01f;
	  invert_increment = false;
	  parent_rotation = 0;
	}
	void update()
	{
	  vec3f v(0,0,dy);
	  if(parent_rotation != 0) parent_rotation->mult_vec(v); 

	  if(invert_increment)
		  dolly += v * glScale;
	  else
		  dolly -= v * glScale;
	  glutPostRedisplay();
	}

	void apply_transform()
	{
	  //cerr << "Applying transform: " << (x - x0) << ", " << (y - y0) << endl;
	  glTranslate(dolly);
	}

	void apply_inverse_transform()
	{
	  //cerr << "Applying transform: " << (x - x0) << ", " << (y - y0) << endl;
	  glTranslate(-dolly);
	}
	
	matrix4f get_transform()
	{
		matrix4f m;
		m.make_identity();
		m.set_translate(dolly);
		return m;
	}	

	matrix4f get_inverse_transform()
	{
		matrix4f m;
		m.make_identity();
		m.set_translate(-dolly);
		return m;
	}	


	bool invert_increment;
	const rotationf * parent_rotation;
	vec3f dolly;
	float glScale;
  };


  class glut_trackball : public glut_simple_interactor
  {
  public:
	glut_trackball()
	{
		r = rotationf(vec3f(0, 1, 0), 0);
		centroid = vec3f(0,0,0);
		glScale = -.01f;
		invert_increment = false;
		parent_rotation = 0;
	}

	void update()
	{
        if(dx == 0 && dy == 0) return;
        vec3f v(dy, -dx, 0);
        float len = v.normalize();
        if(parent_rotation != 0) parent_rotation->mult_vec(v);
        //r.mult_dir(vec3f(v), v);
        rotationf increment;
        if(invert_increment)
          increment.set_value(v, -len * glScale);
        else
          increment.set_value(v, len * glScale);
        r = increment * r; 
        glutPostRedisplay();
	}

	void apply_transform()
	{
		glTranslate(centroid);
		glRotate(r);
		glTranslate(-centroid);
	}

	void apply_inverse_transform()
	{
		glTranslate(centroid);
		glRotate(r.inverse());
		glTranslate(-centroid);
	}


	matrix4f get_transform()
	{
		matrix4f mt, mr, minvt;
		mt.set_translate(centroid);
		r.get_value(mr);
		minvt.set_translate(-centroid);
		return mt * mr * minvt;
	}

	matrix4f get_inverse_transform()
	{
		matrix4f mt, mr, minvt;
		mt.set_translate(centroid);
		r.inverse().get_value(mr);
		minvt.set_translate(-centroid);
		return mt * mr * minvt;
	}

	bool invert_increment;
	const rotationf * parent_rotation;
	rotationf r;
	vec3f centroid;
	float glScale;
  }; 

  
  class glut_rotate : public glut_simple_interactor
  {
  public:
	glut_rotate()
	{
	  rotate_x = rotate_y = 0;
	  glScale = 1;
	}
	void update()
	{
	  rotate_x += dx * glScale;
	  rotate_y += dy * glScale;
	  glutPostRedisplay();
	}

	void apply_transform()
	{
	  glRotatef(rotate_x, 0, 1, 0);
	  glRotatef(rotate_y, -1, 0, 0);
	}

	void apply_inverse_transform()
	{
	  glRotatef(-rotate_y, -1, 0, 0);
	  glRotatef(-rotate_x, 0, 1, 0);
	}

	matrix4f get_transform()
	{
		rotationf rx(to_radians(rotate_x), 0, 1, 0);
		rotationf ry(to_radians(rotate_y), -1, 0, 0);
		matrix4f mx, my;
		rx.get_value(mx);
		ry.get_value(my);
		return mx * my;
	}	

	matrix4f get_inverse_transform()
	{
		rotationf rx(to_radians(-rotate_x), 0, 1, 0);
		rotationf ry(to_radians(-rotate_y), -1, 0, 0);
		matrix4f mx, my;
		rx.get_value(mx);
		ry.get_value(my);
		return my * mx; 
	}	

	float rotate_x, rotate_y, glScale;
  };

  class glut_mouse_to_keyboard : public glut_simple_interactor
  {
  public:
	  glut_mouse_to_keyboard()
	  {
		  keyboard_function = 0;
		  pos_dx_key = neg_dx_key = pos_dy_key = neg_dy_key = 0;
	  }
	  void apply_transform() {}

	  void apply_inverse_transform() {}

	  matrix4f get_transform() {return matrix4f();}

	  matrix4f get_inverse_transform() {return matrix4f();}

	  void update()
	  {
		  if(!keyboard_function) return;
		  if(dx > 0)
			keyboard_function(pos_dx_key, x, y);
		  else if(dx < 0)
			keyboard_function(neg_dx_key, x, y);
		  if(dy > 0)
			keyboard_function(pos_dy_key, x, y);
		  else if(dy < 0)
			keyboard_function(neg_dy_key, x, y);		
	  }
	  unsigned char pos_dx_key, neg_dx_key, pos_dy_key, neg_dy_key;
	  void (*keyboard_function)(unsigned char, int, int);
  };

  inline void glut_exit_on_escape(unsigned char k, int x = 0, int y = 0)
  { if(k==27) exit(0); }

  struct glut_simple_mouse_interactor : public glut_interactor
  {

  public:
	glut_simple_mouse_interactor(int num_buttons_to_use=3)
	{
	  configure_buttons(num_buttons_to_use);
	  camera_mode = false;
	}
	void set_camera_mode(bool cam)
	{
		camera_mode = cam;
		if(camera_mode)
		{
			trackball.invert_increment = true;
			pan.invert_increment = true;
			dolly.invert_increment = true;
			pan.parent_rotation = & trackball.r;
			dolly.parent_rotation = & trackball.r;
		}
		else
		{
			trackball.invert_increment = false;
			pan.invert_increment = false;
			dolly.invert_increment = false;
			if(pan.parent_rotation == &trackball.r) pan.parent_rotation = 0;
			if(dolly.parent_rotation == &trackball.r) dolly.parent_rotation = 0;
		}
	}
	void configure_buttons(int num_buttons_to_use = 3)
	{
	  switch(num_buttons_to_use)
	  {
	  case 1:
		trackball.activate_on = GLUT_LEFT_BUTTON;

		trackball.modifiers = 0;
		pan.activate_on = GLUT_LEFT_BUTTON;
		pan.modifiers = GLUT_ACTIVE_SHIFT;
		dolly.activate_on = GLUT_LEFT_BUTTON;
		dolly.modifiers = GLUT_ACTIVE_CTRL;
		break;

	  case 2:
		trackball.activate_on = GLUT_LEFT_BUTTON;
		trackball.modifiers = 0;

		pan.activate_on = GLUT_MIDDLE_BUTTON;
		pan.modifiers = 0;

		dolly.activate_on = GLUT_LEFT_BUTTON;
		dolly.modifiers = GLUT_ACTIVE_CTRL;
		break;

	  case 3:
	  default:
		trackball.activate_on = GLUT_LEFT_BUTTON;
		trackball.modifiers = 0;

		pan.activate_on = GLUT_MIDDLE_BUTTON;
		pan.modifiers = 0;

		dolly.activate_on = GLUT_RIGHT_BUTTON;
		dolly.modifiers = 0;

		break;
	  }
	}

	virtual void motion(int x, int y)
	{ 

		trackball.motion(x,y);

		pan.motion(x,y);

		dolly.motion(x,y);
	}
	virtual void mouse(int button, int state, int x, int y)
	{
	  trackball.mouse(button, state, x, y);
	  pan.mouse(button, state, x, y);
	  dolly.mouse(button, state, x, y);
	}

	void apply_transform()
	{
	  pan.apply_transform();
	  dolly.apply_transform();
	  trackball.apply_transform();
	}

	void apply_inverse_transform()
	{
	  trackball.apply_inverse_transform();
	  dolly.apply_inverse_transform();
	  pan.apply_inverse_transform();
	}

	matrix4f get_transform()
	{
	  return ( pan.get_transform()       *
			   dolly.get_transform()     *
			   trackball.get_transform() );
	}

	matrix4f get_inverse_transform()
	{
	  return ( trackball.get_inverse_transform() *
			   dolly.get_inverse_transform()     *
			   pan.get_inverse_transform()       );
	}

	void set_parent_rotation(rotationf *rp)
	{
		trackball.parent_rotation = rp;
		dolly.parent_rotation = rp;
		pan.parent_rotation = rp;
	}

	bool camera_mode;
	glut_trackball trackball;
	glut_pan  pan;
	glut_dolly dolly;
  };

}

#endif
