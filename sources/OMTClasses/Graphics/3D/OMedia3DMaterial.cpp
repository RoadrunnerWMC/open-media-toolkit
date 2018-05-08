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


#include "OMedia3DMaterial.h"
#include "OMediaError.h"
#include "OMediaDataBase.h"


OMedia3DMaterial::OMedia3DMaterial(void)
{
	fill_mode = omfillmc_Solid;
	shade_mode = omshademc_Flat;
	light_mode = ommlmc_Color;
	src_blend = omblendfc_One;
	dest_blend = omblendfc_Zero;

	emission.set(1.0f,0,0,0);
	ambient.set(1.0f,0.2f,0.2f,0.2f);
	diffuse.set(1.0f, 0.8f, 0.8f, 0.8f);
	specular.set(1.0f,0,0,0);
	
	shininess = 1.0f;
	flags = 0;
	texture = NULL;
	texture_address_mode = omtamc_Wrap;
	texture_color_operation = omtcoc_Modulate;
}

OMedia3DMaterial::~OMedia3DMaterial(void) 
{
	db_update();
	
	if (texture) texture->db_unlock();
}

unsigned long OMedia3DMaterial::db_get_type(void) const
{
	return OMedia3DMaterial::db_type;
}

void OMedia3DMaterial::purge(void)
{
	fill_mode = omfillmc_Solid;
	shade_mode = omshademc_Flat;
	light_mode = ommlmc_Color;
	src_blend = omblendfc_One;
	dest_blend = omblendfc_Zero;

	emission.set(1.0f,0,0,0);
	ambient.set(1.0f,0.2f,0.2f,0.2f);
	diffuse.set(1.0f, 0.8f, 0.8f, 0.8f);
	specular.set(1.0f,0,0,0);
	
	shininess = 1.0f;
	flags = 0;

	set_texture(NULL);	
}


OMedia3DMaterial::OMedia3DMaterial(const OMedia3DMaterial &x)
{
	fill_mode = omfillmc_Solid;
	shade_mode = omshademc_Flat;
	light_mode = ommlmc_Color;
	src_blend = omblendfc_One;
	dest_blend = omblendfc_Zero;

	emission.set(1.0f,0,0,0);
	ambient.set(1.0f,0.2f,0.2f,0.2f);
	diffuse.set(1.0f, 0.8f, 0.8f, 0.8f);
	specular.set(1.0f,0,0,0);
	
	shininess = 0.0f;
	flags = 0;
	texture = NULL;

	assign(x);
}

void OMedia3DMaterial::set_color(const OMediaFRGBColor &color,
						 float femissive,			// 0.0f - 1.0f
						 float fambient,
						 float fdiffuse,
						 float fspecular)
{
	emission.set(color.red*femissive,color.green*femissive,color.blue*femissive);
	ambient.set(color.red*fambient,color.green*fambient,color.blue*fambient);
	diffuse.set(color.red*fdiffuse,color.green*fdiffuse,color.blue*fdiffuse);
	specular.set(color.red*fspecular,color.green*fspecular,color.blue*fspecular);
}

void OMedia3DMaterial::set_texture(OMediaCanvas *canvas)
{
	if (texture) texture->db_unlock();
	texture = canvas;
	if (texture) texture->db_lock();
}

void OMedia3DMaterial::assign(const OMedia3DMaterial &m)
{
	fill_mode = m.get_fill_mode();
	shade_mode = m.get_shade_mode();
	light_mode = m.get_light_mode();
	src_blend = m.get_blend_src();
	dest_blend = m.get_blend_dest();

	m.get_emission(emission);
	m.get_ambient(ambient);
	m.get_diffuse(diffuse);
	m.get_specular(specular);
	
	shininess = m.get_shininess();
	flags = m.get_flags();
	
	set_texture(m.get_texture());	

	texture_address_mode = m.get_texture_address_mode();
	texture_color_operation	= m.get_texture_color_operation();
}

void OMedia3DMaterial::read_class(OMediaStreamOperators &stream)
{
	short	es;
	short	version;

	#define read_enum(t,x) stream>>es; x = t(es) 
	#define read_argb(x) stream>>x.alpha; stream>>x.red; stream>>x.green; stream>>x.blue

	OMediaDBObject::read_class(stream);

	purge();

	stream>>version;
	if (version>3) omd_EXCEPTION(omcerr_BadFormat);

	if (version>1)
	{	
		read_enum(omt_FillMode,fill_mode);
		read_enum(omt_ShadeMode,shade_mode);
		read_enum(omt_MaterialLightMode,light_mode);
		read_enum(omt_BlendFunc,src_blend);
		read_enum(omt_BlendFunc,dest_blend);
		
		read_argb(emission);
		read_argb(ambient);
		read_argb(diffuse);
		read_argb(specular);

		stream>>shininess;
		stream>>flags;
		
		OMediaDBObjectStreamLink	slink;
			
		stream>>slink;
		set_texture((OMediaCanvas*)slink.get_object());

		if (version>2)
		{
			read_enum(omt_TextureAddressMode,texture_address_mode);
			read_enum(omt_TextureColorOperation,texture_color_operation);
		}
	}
	else
	{
		// OMT 1.x material	format
		
		short	mtype;
		bool	nofog,gouraud;
		float	alpha_channel;
		
		OMediaRGBColor	emission,diffuse,specular,ambient;

		
		stream>>mtype;
		
		if (mtype>2) return;
		stream>>alpha_channel;
		stream>>nofog;

		if (nofog) flags |= ommatf_DisableFog;

		if (mtype==0)
		{
			stream>>diffuse.red;
			stream>>diffuse.green;
			stream>>diffuse.blue;		
			light_mode = ommlmc_Color;
			shade_mode = omshademc_Flat;
		}
		else
		{
			light_mode = ommlmc_Light;
			bool		shaded;

			stream>>emission.red;	stream>>emission.green;	stream>>emission.blue;
			stream>>diffuse.red;	stream>>diffuse.green;	stream>>diffuse.blue;
			stream>>specular.red;	stream>>specular.green;	stream>>specular.blue;
			stream>>ambient.red;	stream>>ambient.green;	stream>>ambient.blue;

			stream>>gouraud;
			if (gouraud) shade_mode = omshademc_Gouraud;
			else shade_mode = omshademc_Flat;
			
			if (mtype==2)
			{
				short contain;

				stream>>shaded;
				if (shaded) light_mode = ommlmc_Light;
				else 
				{
					light_mode = ommlmc_Color;
					diffuse.red = diffuse.green = diffuse.blue = 0xFFFF;
				}

				stream>>contain;

				if (contain==1)
				{
					OMediaDBObjectStreamLink	slink;
		
					stream>>slink;
					set_texture((OMediaCanvas*)slink.get_object());
				}
			}
		}

		OMedia3DMaterial::emission.set(emission);
		OMedia3DMaterial::diffuse.set(diffuse);
		OMedia3DMaterial::specular.set(specular);
		OMedia3DMaterial::ambient.set(ambient);		
		
		OMedia3DMaterial::diffuse.alpha = 1.0f - alpha_channel;
	}
}

void OMedia3DMaterial::write_class(OMediaStreamOperators &stream)
{
	short	es;
	short	version = 3;
	OMediaDBObjectStreamLink	slink;

	#define write_enum(x) es = short(x); stream<<es
	#define write_argb(x) stream<<x.alpha; stream<<x.red; stream<<x.green; stream<<x.blue

	OMediaDBObject::write_class(stream);

	stream<<version;

	write_enum(fill_mode);
	write_enum(shade_mode);
	write_enum(light_mode);
	write_enum(src_blend);
	write_enum(dest_blend);
	
	write_argb(emission);
	write_argb(ambient);
	write_argb(diffuse);
	write_argb(specular);
	
	stream<<shininess;
	stream<<flags;	

		
	slink.set_object(texture);
	stream<<slink;

	write_enum(texture_address_mode);
	write_enum(texture_color_operation);
}

unsigned long OMedia3DMaterial::get_approximate_size(void)
{
	return sizeof(*this);
}

OMediaDBObject *OMedia3DMaterial::db_builder(void)
{
	return new OMedia3DMaterial;
}



