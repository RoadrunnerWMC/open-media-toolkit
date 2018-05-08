/*****************************************************************

        O P E N      M E D I A     T O O L K I T              V2.5    
 
        Copyright Yves Schmid 1996-2003
 
        See www.garagecube.com for more informations about this library.
        
        Author(s): Yves Schmid
 
        OMT is provided under LGPL:
 
          This library is free software; you can redistribute it and/or
          modify it under the terms of the GNU Lesser General Public
          License as published by the Free Software Foundation; either
          version 2.1 of the License, or (at your option) any later version.

          This library is distributed in the hope that it will be useful,
          but WITHOUT ANY WARRANTY; without even the implied warranty of
          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
          Lesser General Public License for more details.

          You should have received a copy of the GNU Lesser General Public
          License along with this library; if not, write to the Free Software
          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

          The full text of the license can be found in lgpl.txt          

******************************************************************/
 

#include "OMediaLight.h"
#include "OMediaWorld.h"

#include "OMediaViewPort.h" 

unsigned short OMediaLight::current_render_count;
unsigned long OMediaLight::render_layer_key;


OMediaLight::OMediaLight() 
{
	light_type = omclt_Directional;
	layer_key = 0xFFFFFFFF;
	
	const_attenuation = 1.0f;
	linear_attenuation = 0.0f;
	quadratic_attenuation = 0.0f;
	range = 128.0f;

	spot_cutoff = omd_Deg2Angle(45);
	spot_exponent = 0.0f;

	ambient.set(1.0f,0.0f,0.0f,0.0f);
	diffuse.set(1.0f,1.0f,1.0f,1.0f);
	specular.set(1.0f,1.0f,1.0f,1.0f);

	render_count = 0;
}

OMediaLight::~OMediaLight() 
{
	db_update();
	unlink();
}	

void OMediaLight::reset(void)
{
	light_type = omclt_Directional;
	layer_key = 0xFFFFFFFF;
	
	const_attenuation = 1.0f;
	linear_attenuation = 0.0f;
	quadratic_attenuation = 0.0f;
	range = 128.0f;

	spot_cutoff = omd_Deg2Angle(45);
	spot_exponent = 0.0f;

	ambient.set(1.0f,0.0f,0.0f,0.0f);
	diffuse.set(1.0f,1.0f,1.0f,1.0f);
	specular.set(1.0f,1.0f,1.0f,1.0f);

	render_count = 0;
}
	
void OMediaLight::unlink(void)
{	
	OMediaElement::unlink();
}
	
void OMediaLight::update_world_ptr(OMediaWorld *world)
{
	if (world!=its_world)
	{
		if (world && !its_world)
		{
			OMediaWorld	*w3d = ((OMediaWorld*)world);
		
			w3d->get_light_sources()->push_back(this);
			light_node = --(w3d->get_light_sources()->end());
		}
		
		if (!world && its_world)
		{
			((OMediaWorld*)its_world)->get_light_sources()->erase(light_node);
		}
	}

	OMediaElement::update_world_ptr(world);
}


void OMediaLight::render_compute_light(OMediaMatrix_4x4 &viewmatrix)
{
	if (render_count==0 || current_render_count!=render_count)
	{
		OMediaRenderHTransform		hxform;
		OMediaMatrix_4x4			matrix;

		render_count = current_render_count;
	
		hxform.trans_x = hxform.trans_y = hxform.trans_z = 0;
		compute_hxform_hierarchy(hxform);
		compute_model_matrix(hxform, viewmatrix, matrix);

		render_position.set(0,0,0);
		matrix.multiply(render_position);

		if (light_type!=omclt_Point)
		{
			render_direction.set(0,0,1.0);
			matrix.multiply(render_direction);
			render_direction.x -= render_position.x;
			render_direction.y -= render_position.y;
			render_direction.z -= render_position.z;
			render_direction.normalize();
		}
	}
}

void OMediaLight::render_light(long index, OMediaRendererInterface *rdr_i)
{

	rdr_i->set_light_type(index,light_type);
	rdr_i->set_light_ambient(index,ambient);
	rdr_i->set_light_diffuse(index,diffuse);
	rdr_i->set_light_specular(index,specular);

	switch(light_type)
	{
		case omclt_Directional:
		rdr_i->set_light_dir(index,render_direction);
		break;

		case omclt_Point:
		rdr_i->set_light_pos(index,render_position);
		rdr_i->set_light_attenuation(index, range, const_attenuation, linear_attenuation, quadratic_attenuation);
		break;

		case omclt_Spot:
		rdr_i->set_light_pos(index,render_position);
		rdr_i->set_light_dir(index,render_direction);
		rdr_i->set_light_spot_cutoff(index, spot_cutoff);
		rdr_i->set_light_spot_exponent(index, spot_exponent);
		rdr_i->set_light_attenuation(index, range, const_attenuation, linear_attenuation, quadratic_attenuation);
		break;
	}
}

OMediaDBObject *OMediaLight::db_builder(void)
{
	return new OMediaLight;
}

void OMediaLight::read_class(OMediaStreamOperators &stream)
{
	short	s;

	OMediaElement::read_class(stream);

	stream>>s; light_type = omt_LightType(s);

	stream>>layer_key;	
	stream>>const_attenuation;
	stream>>linear_attenuation;
	stream>>quadratic_attenuation;
	stream>>range;

	stream>>spot_cutoff;
	stream>>spot_exponent;

	stream>>ambient.alpha;
	stream>>ambient.red;
	stream>>ambient.green;
	stream>>ambient.blue;
	stream>>diffuse.alpha;
	stream>>diffuse.red;
	stream>>diffuse.green;
	stream>>diffuse.blue;
	stream>>specular.alpha;
	stream>>specular.red;
	stream>>specular.green;
	stream>>specular.blue;
}

void OMediaLight::write_class(OMediaStreamOperators &stream)
{
	short	s;

	OMediaElement::write_class(stream);

	s = short(light_type); stream<<s;

	stream<<layer_key;	
	stream<<const_attenuation;
	stream<<linear_attenuation;
	stream<<quadratic_attenuation;
	stream<<range;

	stream<<spot_cutoff;
	stream<<spot_exponent;

	stream<<ambient.alpha;
	stream<<ambient.red;
	stream<<ambient.green;
	stream<<ambient.blue;
	stream<<diffuse.alpha;
	stream<<diffuse.red;
	stream<<diffuse.green;
	stream<<diffuse.blue;
	stream<<specular.alpha;
	stream<<specular.red;
	stream<<specular.green;
	stream<<specular.blue;

}

unsigned long OMediaLight::get_approximate_size(void)
{
	return OMediaElement::get_approximate_size();
}

unsigned long OMediaLight::db_get_type(void)
{
	return OMediaLight::db_type;
}

