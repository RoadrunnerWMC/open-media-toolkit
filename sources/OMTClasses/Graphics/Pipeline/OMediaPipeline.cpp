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
 
#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OMTRENDERER

#include "OMediaPipeline.h"
#include "OMediaRect.h"
#include "OMediaError.h"
#include "OMedia3DShape.h"
#include "OMediaOMTCanvas.h"

long OMediaPipeline::buffer_user;

OMediaPipePointBuffer 		*OMediaPipeline::point_buffer;
OMediaPipePolygonBuffer 	*OMediaPipeline::poly_buffer;

OMediaPipePoint			**OMediaPipeline::clip_tab_buffer;
unsigned long			OMediaPipeline::clip_tab_buffer_size;


OMediaPipeline::OMediaPipeline(OMediaPipe2RendererInterface *p2rdr_i)
{
	light_enabled = false;
	shade_mode = omshademc_Flat;
	fill_mode = omfillmc_Solid;
	blend_enabled = false;
	blend_src = omblendfc_One;
	blend_dest = omblendfc_Zero;
	culling_enabled = false;
	override_mat_fillmode = false;
	texture = NULL;
	picking_mode = false;
	texture_color_mode = omtcoc_Modulate;

	pipe2rnd_interface = p2rdr_i;

	if (buffer_user == 0) 
	{
		point_buffer 		= new OMediaPipePointBuffer;
		poly_buffer			= new OMediaPipePolygonBuffer;
		clip_tab_buffer_size = 32;
		clip_tab_buffer		= new OMediaPipePoint*[clip_tab_buffer_size];
	}

	buffer_user++;

	model_view.set_identity();
	projection.set_identity();
	model_view.invert(inv_model_view);
}

OMediaPipeline::~OMediaPipeline()
{
	buffer_user--;
	if (buffer_user == 0)
	{
		delete point_buffer;
		delete poly_buffer;
		delete [] clip_tab_buffer;
		clip_tab_buffer_size = 0;
		
		point_buffer = NULL;
		poly_buffer = NULL;
	}
}

void OMediaPipeline::begin(omt_PipelineFlags fl)
{
	flags = fl;
	blend_enabled = false;
	texture = NULL;
	picking_mode = false;

	point_buffer->reset_pointer();
	poly_buffer->reset_pointer();
}

void OMediaPipeline::end()
{
}

bool OMediaPipeline::clip_polygon(OMediaPipePolygon *poly, omt_ClippingPlane plane)
{
	OMediaPipePoint		*ptr1,*ptr2,*last_p,*ptr1nv;
	bool				clip_current, clip_next,one_clipped;
    float 				para,d,dw;
	OMediaPipePoint	 	**ptab;
	short				np = 0;

    ptr1 = poly->points;
	ptr2 = point_buffer->reserve_points(poly->npoints<<2UL);
	last_p = ptr1 + poly->npoints;

	if ((poly->npoints<<2UL)>(long)clip_tab_buffer_size) enlarge_clip_tab_buffer(poly->npoints<<2UL);

	ptab = clip_tab_buffer;
    one_clipped = false;

	for (;ptr1!=last_p;ptr1++)
    {
		ptr1nv = ptr1;
		ptr1nv++;
		if (ptr1nv==last_p) ptr1nv = poly->points;  

#define x xyzw[0]
#define y xyzw[1]
#define z xyzw[2]
#define w xyzw[3]
	
		switch(plane)
		{
			case omclippc_Left:
       		clip_current = (ptr1->x < -ptr1->w);
        	clip_next    = (ptr1nv->x < -ptr1nv->w);
			break;

			case omclippc_Right:
       		clip_current = (ptr1->x > ptr1->w);
        	clip_next    = (ptr1nv->x > ptr1nv->w);
			break;

			case omclippc_Top:
       		clip_current = (ptr1->y > ptr1->w);
        	clip_next    = (ptr1nv->y > ptr1nv->w);
			break;

			case omclippc_Bottom:
       		clip_current = (ptr1->y < -ptr1->w);
        	clip_next    = (ptr1nv->y < -ptr1nv->w);
			break;

			case omclippc_Near:
       		clip_current = (ptr1->z < -ptr1->w);
        	clip_next    = (ptr1nv->z < -ptr1nv->w);
			break;

			case omclippc_Far:
       		clip_current = (ptr1->z > ptr1->w);
        	clip_next    = (ptr1nv->z > ptr1nv->w);
			break;
		}


         if (!clip_current)
         {
			*ptab = ptr1;
			ptab++;
			np++;
			ptr2++;
         }
         
		 if (clip_current != clip_next)
         {
       		one_clipped = true;


			switch(plane)
			{
				case omclippc_Left:
				d = (ptr1nv->x -  ptr1->x);
				dw = (ptr1nv->w -  ptr1->w);
				para = -((ptr1->x + ptr1->w) / (dw+d));
				
				ptr2->x = ptr1->x + para * d;
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omclippc_Right:
				d = (ptr1nv->x -  ptr1->x);
				dw = (ptr1nv->w -  ptr1->w);
				para = (ptr1->x - ptr1->w) / (dw-d);
				
				ptr2->x = ptr1->x + para * d;
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omclippc_Top:
				d = (ptr1nv->y -  ptr1->y);
				dw = (ptr1nv->w -  ptr1->w);
				para = ((ptr1->y - ptr1->w) / (dw-d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * d;
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omclippc_Bottom:
				d = (ptr1nv->y -  ptr1->y);
				dw = (ptr1nv->w -  ptr1->w);
				para = -((ptr1->y + ptr1->w) / (dw+d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * d;
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omclippc_Near:
				d = (ptr1nv->z -  ptr1->z);
				dw = (ptr1nv->w -  ptr1->w);
				para = -((ptr1->z + ptr1->w) / (dw+d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * d;
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omclippc_Far:
				d = (ptr1nv->z -  ptr1->z);
				dw = (ptr1nv->w -  ptr1->w);
				para = ((ptr1->z - ptr1->w) / (dw-d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * d;
				ptr2->w = ptr1->w + para * dw;
				break;
			}
            
           	ptr2->a = (ptr1->a + para * (ptr1nv->a - ptr1->a));
           	ptr2->dr = (ptr1->dr + para * (ptr1nv->dr - ptr1->dr));
           	ptr2->dg = (ptr1->dg + para * (ptr1nv->dg - ptr1->dg));
           	ptr2->db = (ptr1->db + para * (ptr1nv->db - ptr1->db));
           	ptr2->sr = (ptr1->sr + para * (ptr1nv->sr - ptr1->sr));
           	ptr2->sg = (ptr1->sg + para * (ptr1nv->sg - ptr1->sg));
           	ptr2->sb = (ptr1->sb + para * (ptr1nv->sb - ptr1->sb));	
           	ptr2->u = (ptr1->u + para * (ptr1nv->u - ptr1->u));
           	ptr2->v = (ptr1->v + para * (ptr1nv->v - ptr1->v));
		
			
			*ptab = NULL;
			ptab++;
			ptr2++; 
			np++;
        }
	}

#undef x
#undef y
#undef z
#undef w
    
        
    if (np==0) return true;
        
    if (one_clipped) move_clipped_points(np,poly,clip_tab_buffer); 

  	return false;
}


void OMediaPipeline::move_clipped_points(short np, 
								OMediaPipePolygon *poly,
								OMediaPipePoint **ptab)
{
	OMediaPipePoint	*ptr1,*ptr2;

	poly->npoints = np;
	ptr2 = poly->points = point_buffer->get_points(np);
	while(np--)
	{
		ptr1 = *ptab;
		if (ptr1)
		{
			ptr2->xyzw[0] = ptr1->xyzw[0];
			ptr2->xyzw[1] = ptr1->xyzw[1];
			ptr2->xyzw[2] = ptr1->xyzw[2];
			ptr2->xyzw[3] = ptr1->xyzw[3];
			
			ptr2->a = ptr1->a;
			ptr2->dr = ptr1->dr;
			ptr2->dg = ptr1->dg;
			ptr2->db = ptr1->db;
			ptr2->sr = ptr1->sr;
			ptr2->sg = ptr1->sg;
			ptr2->sb = ptr1->sb;
			
			ptr2->u = ptr1->u;
			ptr2->v = ptr1->v;
		}

		ptr2++;
		ptab++;
	}
}



//-------------------------------
// Buffers

OMediaPipePoint *OMediaPipePointBuffer::get_points(long n)
{	
	current_seg_offset+=n;
	if (current_seg_offset<=omd_PipePointSegmentSize)
	{
		OMediaPipePoint	*p = current_point;
		current_point+=n;
		return p;
	}

	if (n>omd_PipePointSegmentSize) omd_EXCEPTION(omcerr_PointBufferFull);

	current_seg++;
	if (current_seg==segments.end()) new_segment();
	else
	{
		current_seg_offset = 0;
		current_point = (*current_seg).points;
	}
	
	return get_points(n);
}

OMediaPipePoint *OMediaPipePointBuffer::do_reserve_points(long n)
{
	if (n>omd_PipePointSegmentSize) omd_EXCEPTION(omcerr_PointBufferFull);

	current_seg++;
	if (current_seg==segments.end()) new_segment();
	else
	{
		current_seg_offset = 0;
		current_point = (*current_seg).points;
	}

	return current_point;
}


OMediaPipePolygon *OMediaPipePolygonBuffer::get_polygon(void)
{
	current_seg_offset++;
	if (current_seg_offset<=omd_PipePolygonSegmentSize)
	{
		OMediaPipePolygon	*p = current_polygon;
		current_polygon++;
		return p;
	}

	current_seg++;
	if (current_seg==segments.end()) new_segment();
	else
	{
		current_seg_offset = 0;
		current_polygon = (*current_seg).polygons;
	}

	return get_polygon();
}

//-------------------------------
// Rendering interface


void OMediaPipeline::enable_clipping_rect(bool enable)
{
}

void OMediaPipeline::set_clipping_rect(const OMediaRect &rect)
{
}

omt_RendererRequirementFlags OMediaPipeline::get_requirement_flags(void)
{
	return 0;
}

void OMediaPipeline::get_view_bounds(OMediaRect &r)
{
	pipe2rnd_interface->get_view_bounds(r);	
}

void OMediaPipeline::clear_all_buffers(OMediaFARGBColor &rgb)
{
	pipe2rnd_interface->clear_colorbuffer(rgb);
	pipe2rnd_interface->clear_zbuffer();
}
	
void OMediaPipeline::clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *area)
{
	pipe2rnd_interface->clear_colorbuffer(rgb,area);
}

void OMediaPipeline::clear_zbuffer(OMediaRect *area)
{
	pipe2rnd_interface->clear_zbuffer(area);
}

void OMediaPipeline::set_zbuffer_write(omt_ZBufferWrite zb)
{
	pipe2rnd_interface->set_zbuffer_write(zb);
}

void OMediaPipeline::set_zbuffer_test(omt_ZBufferTest zb)
{
	pipe2rnd_interface->set_zbuffer_test(zb);
}

void OMediaPipeline::set_zbuffer_func(omt_ZBufferFunc zb)
{
	pipe2rnd_interface->set_zbuffer_func(zb);
}

void OMediaPipeline::set_model_view(OMediaMatrix_4x4 &m)
{
	model_view = m;
	model_view.invert(inv_model_view);
}

void OMediaPipeline::set_projection(OMediaMatrix_4x4 &m) 
{
	projection = m;
}


// Light

void OMediaPipeline::enable_lighting(void)
{
	light_enabled = true;
}

void OMediaPipeline::disable_lighting(void)
{
	light_enabled = false;
}
	
long OMediaPipeline::get_max_lights(void)
{
	return omd_MAX_PIPELINE_LIGHTS;
} 
		
void OMediaPipeline::start_light_edit(void) {}
void OMediaPipeline::end_light_edit(void) {}
	
void OMediaPipeline::enable_light(long index)
{
	lights[index].enabled = true;
}

void OMediaPipeline::disable_light(long index)
{
	lights[index].enabled = false;
}
	
void OMediaPipeline::set_light_type(long index, omt_LightType type)
{
	lights[index].type = type;
}

void OMediaPipeline::set_light_pos(long index, OMedia3DPoint &p)
{
	lights[index].pos = p;
}

void OMediaPipeline::set_light_dir(long index, OMedia3DVector &v)
{
	lights[index].vect = v;
}
	
void OMediaPipeline::set_light_ambient(long index, OMediaFARGBColor &argb)
{
	lights[index].ambient = argb;
}

void OMediaPipeline::set_light_diffuse(long index, OMediaFARGBColor &argb)
{
	lights[index].diffuse = argb;
}

void OMediaPipeline::set_light_specular(long index, OMediaFARGBColor &argb)
{
	lights[index].specular = argb;

}
	
void OMediaPipeline::set_light_attenuation(long index, float range, float constant, float linear, float quadratic)
{
	lights[index].range = 1.0f/range;
	lights[index].constant_attenuation = constant;
	lights[index].linear_attenuation = linear; 
	lights[index].quadratic_attenuation = quadratic;
}
	
void OMediaPipeline::set_light_spot_cutoff(long index, float cutoff)
{
	lights[index].spot_cutoff = omt_Angle(cutoff);
}

void OMediaPipeline::set_light_spot_exponent(long index, float expo)
{
	lights[index].spot_exponent = expo;
}
	
void OMediaPipeline::set_light_global_ambient(OMediaFARGBColor &argb)
{
	global_ambient = argb;
}

void OMediaPipeline::set_material(OMediaFARGBColor &emission,
											OMediaFARGBColor &diffuse,
											OMediaFARGBColor &specular,
											OMediaFARGBColor &ambient,
											float			 shininess)
{
	material.emission=emission;
	material.diffuse=diffuse;
	material.specular=specular;
	material.ambient=ambient;
	material.shininess=shininess;
}

void OMediaPipeline::set_blend(omt_Blend blend)
{
	blend_enabled = (blend==omblendc_Enabled);
}

void OMediaPipeline::set_blend_func(omt_BlendFunc src_func, omt_BlendFunc dest_func)
{
	blend_src = src_func;
	blend_dest = dest_func;
}	
	
void OMediaPipeline::set_override_material_fill_mode(bool m)
{
	override_mat_fillmode = m;
}

void OMediaPipeline::set_shade_mode(omt_ShadeMode mode)
{
	shade_mode = mode;
}

void OMediaPipeline::set_fill_mode(omt_FillMode mode)
{
	fill_mode = mode;
}
	
void OMediaPipeline::enable_faceculling(void)
{
	culling_enabled = true;
}

void OMediaPipeline::disable_faceculling(void)
{
	culling_enabled = false;
}
	
void OMediaPipeline::set_texture(OMediaCanvas *canvas)
{
	if (canvas)
	{
		OMediaRenderTarget	*rtarget = pipe2rnd_interface->get_target();
		OMediaOMTCanvas		*omt_texture;

		omt_texture = (OMediaOMTCanvas*)canvas->find_implementation(rtarget, omcskf_Scaled|omcskf_Exact, 0, true);
		
		if (!omt_texture) omt_texture = new OMediaOMTCanvas((OMediaOMTRenderTarget*)rtarget,canvas,omcskf_Scaled|omcskf_Exact);
		else omt_texture->render_prepare();	

		texture = omt_texture->texture_grid;
	}
	else
		texture = NULL;
}

void OMediaPipeline::set_texture_address_mode(const omt_TextureAddressMode am)
{
	// Only wrap is supported in software
}

void OMediaPipeline::set_texture_color_operation(const omt_TextureColorOperation cm)
{
	texture_color_mode = cm;
}

void OMediaPipeline::set_extra_texture_passes(omt_ExtraTexturePassList *pass_list)
{
}

long OMediaPipeline::get_max_texture_passes(void)
{
	return 0;	// No multi-texturing in software
}
	
void OMediaPipeline::draw(omt_RenderVertexList &vertices, omt_RenderDrawMode mode)
{
	long	n,c;
	omt_RenderVertexList::iterator i,ni,ni2;

	
	switch(mode)
	{
		case omrdmc_Points:	
		begin_gen_polygon(omrfmc_PointList,vertices.size());
		for(i=vertices.begin();
			i!=vertices.end();
			i++)
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);
		end_gen_polygon();
		break;
		
		case omrdmc_Lines:
		n=vertices.size()>>1;
		i=vertices.begin();
		while(n--)
		{
			begin_gen_polygon(omrfmc_LineLoop,2);
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);	i++;
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);	i++;
			end_gen_polygon();
		}		
		break;
		
		case omrdmc_LineStrip:
		n=vertices.size()-1;
		i=vertices.begin();
		ni = i; ni++;
		while(n--)
		{
			begin_gen_polygon(omrfmc_LineLoop,2);
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);
			gen_vertex((*ni),&(*ni).normal, &(*ni).diffuse, (*ni).u,(*ni).v);
			end_gen_polygon();
			
			ni++;	i++;
		}		
		break;

		case omrdmc_LineLoop:	
		begin_gen_polygon(omrfmc_LineLoop,vertices.size());
		for(i=vertices.begin();
			i!=vertices.end();
			i++)
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);
		end_gen_polygon();
		break;
		
		case omrdmc_Triangles:
		n=vertices.size()/3;
		i=vertices.begin();
		while(n--)
		{
			begin_gen_polygon(omrfmc_TriangleFan,3);
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);	i++;
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);	i++;
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);	i++;
			end_gen_polygon();
		}				
		break;
		
		case omrdmc_TriangleStrip:
		n=vertices.size()-2;
		i=vertices.begin();
		ni = i; 	ni++;
		ni2 = ni;	ni2++;
		for(c=0; c<n; c++)
		{
			begin_gen_polygon(omrfmc_TriangleFan,3);
			if (c&1)
			{
				gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);
				gen_vertex((*ni2),&(*ni2).normal, &(*ni2).diffuse, (*ni2).u,(*ni2).v);
				gen_vertex((*ni),&(*ni).normal, &(*ni).diffuse, (*ni).u,(*ni).v);
			}
			else
			{
				gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);
				gen_vertex((*ni),&(*ni).normal, &(*ni).diffuse, (*ni).u,(*ni).v);
				gen_vertex((*ni2),&(*ni2).normal, &(*ni2).diffuse, (*ni2).u,(*ni2).v);
			}
			end_gen_polygon();
			
			ni2++;	ni++;	i++;
		}		
		break;

		case omrdmc_TriangleFan:	
		case omrdmc_Polygon:
		begin_gen_polygon(omrfmc_TriangleFan,vertices.size());
		for(i=vertices.begin();
			i!=vertices.end();
			i++)
			gen_vertex((*i),&(*i).normal, &(*i).diffuse, (*i).u,(*i).v);
		end_gen_polygon();
		break;


	}
}


bool OMediaPipeline::flat_draw_surface(OMediaCanvas 		*canv, 
									float x, 			float y, 		float z, 
									float width,		float height,
									OMediaFARGBColor	&diffuse)
{
	float		xyzw[4],xyzw_h[4],xyzw_v[4],xyzw_hv[4];


	if (fill_mode!=omfillmc_Solid) return false;
	if (diffuse.red!=1.0f || diffuse.green!=1.0f || diffuse.blue!=1.0f) return false;

	xyzw[0] = x;
	xyzw[1] = y;
	xyzw[2] = z;
	xyzw[3] = 1.0f;

	xyzw_h[0] = x+width;
	xyzw_h[1] = y;
	xyzw_h[2] = z;
	xyzw_h[3] = 1.0f;

	model_view.multiply(xyzw);	
	projection.multiply(xyzw);			

	model_view.multiply(xyzw_h);	
	projection.multiply(xyzw_h);			
	if (xyzw[1]!=xyzw_h[1] || xyzw[2]!=xyzw_h[2]) return false;

	xyzw_v[0] = x;
	xyzw_v[1] = y+height;
	xyzw_v[2] = z;
	xyzw_v[3] = 1.0f;

	model_view.multiply(xyzw_v);	
	projection.multiply(xyzw_v);			
	if (xyzw[0]!=xyzw_v[0] || xyzw[2]!=xyzw_v[2]) return false;

	xyzw_hv[0] = x+width;
	xyzw_hv[1] = y+height;
	xyzw_hv[2] = z;
	xyzw_hv[3] = 1.0f;

	model_view.multiply(xyzw_hv);	
	projection.multiply(xyzw_hv);			

	if (xyzw_h[0]!=xyzw_hv[0] || 
		xyzw_v[1]!=xyzw_hv[1] ||
		xyzw[2]!=xyzw_v[2]) return false;

	if (xyzw[0]		> xyzw[3] ||
		xyzw_hv[0]	< -xyzw_hv[3] ||
		xyzw[1]		> xyzw[3] ||
		xyzw_hv[1]	< -xyzw_hv[3]) return false;

	transform_homogenous_coord(xyzw);
	transform_homogenous_coord(xyzw_hv);

	if (xyzw[0] > xyzw_hv[0] || xyzw[1] > xyzw_hv[1]) return false; 

	gen_surface(canv,xyzw,xyzw_hv,diffuse.alpha);
	
	return true;
}
	
void OMediaPipeline::draw_surface(OMediaCanvas 		*canv, 
									float x, 			float y, 		float z, 
									float width,		float height,
									OMediaFARGBColor	&diffuse)
{
	if (flat_draw_surface(canv,x,y,z,width,height,diffuse)) return;

	OMediaOMTCanvas				*omtcanv;
	omt_CanvasSlaveKeyFlags		kflags;
	OMediaRenderTarget			*rtarget = pipe2rnd_interface->get_target();
	OMedia3DVector				n;
	OMedia3DPoint				p;

	kflags = omcskf_SubDivided|omcskf_Exact;

	omtcanv = (OMediaOMTCanvas*) canv->find_implementation(rtarget, kflags, 0, true);
	if (!omtcanv) omtcanv = new OMediaOMTCanvas((OMediaOMTRenderTarget*)rtarget,canv,omcskf_SubDivided);
	else omtcanv->render_prepare();

	if (omtcanv->get_slave_key_long()&omcskf_Exact)
	{
		texture = omtcanv->texture_grid;		

		n.set(0,0,-1.0f);

		switch(fill_mode)
		{
			case omfillmc_Point:
			begin_gen_polygon(omrfmc_PointList,4);
			break;
			
			case omfillmc_Line:
			begin_gen_polygon(omrfmc_LineLoop,4);
			break;

			case omfillmc_Solid:
			begin_gen_polygon(omrfmc_TriangleFan,4);
			break;
		}

		p.set(x,y,z);				gen_vertex(p,&n, &diffuse, 0.0f,	0.9999f);
		p.set(x+width,y,z);			gen_vertex(p,&n, &diffuse, 0.9999f, 0.9999f);
		p.set(x+width,y+height,z);	gen_vertex(p,&n, &diffuse, 0.9999f, 0.0f);
		p.set(x,y+height,z);		gen_vertex(p,&n, &diffuse, 0.0f,   	0.0f);

		end_gen_polygon();
	}
	else
	{
		long					tx,ty;
		OMediaOMTCanvasText		*textid = omtcanv->texture_grid;
		float					canv_w = float(canv->get_width()), canv_h = float(canv->get_height());
		float					scalex = width  / canv_w,
								scaley = height / canv_h;

		float		incx = omtcanv->subdiv_w * scalex,
					incy = omtcanv->subdiv_h * scaley;
		
		float		ix,iy,x2,y2,ih = canv_h * scaley;
		
		float		u2,v2;

		n.set(0,0,-1.0f);

		for(ty =0, iy = 0; ty < omtcanv->n_texth; ty++, iy +=incy)
		{
			v2 = (ty==omtcanv->n_texth-1)?omtcanv->last_v:0.9999f;
		
			for(tx =0, ix = 0; tx < omtcanv->n_textw; tx++,textid++,ix+=incx)
			{
				u2 = (tx==omtcanv->n_textw-1)?omtcanv->last_u:0.9999f;
	
				texture = textid;		
					
				switch(fill_mode)
				{
					case omfillmc_Point:
					begin_gen_polygon(omrfmc_PointList,4);
					break;
					
					case omfillmc_Line:
					begin_gen_polygon(omrfmc_LineLoop,4);
					break;

					case omfillmc_Solid:
					begin_gen_polygon(omrfmc_TriangleFan,4);
					break;
				}
	
				x2 = ix+incx;	if (x2>width) x2 = width;
				y2 = iy+incy;	if (y2>height) y2 = height;

				p.set(x+ix,y+(ih-iy),z);	gen_vertex(p,&n, &diffuse, 0.0f,	0.0f);
				p.set(x+ix,y+(ih-y2),z);	gen_vertex(p,&n, &diffuse, 0.0f,	v2);
				p.set(x+x2,y+(ih-y2),z);	gen_vertex(p,&n, &diffuse, u2, 		v2);
				p.set(x+x2,y+(ih-iy),z);	gen_vertex(p,&n, &diffuse, u2, 		0.0f);

				end_gen_polygon();
			}	
		}
	}

	texture = NULL;
}
		
void OMediaPipeline::draw_shape(OMedia3DShape		*shape,
								bool			&inout_second_pass,
								OMediaRendererOverrideVertexList	*ovlist)
{

	shape->lock(omlf_Read);

	omt_PolygonList *polygons = shape->get_polygons();
	omt_VertexList *vlist  = shape->get_vertices(); 
	omt_NormalList *nlist = shape->get_normals();
	omt_ColorList	*clist = shape->get_colors();

	if (ovlist)
	{
		// Note: overrided vertex list can be bigger than the vertex list of the shape
		if (ovlist->vertex_list) vlist = ovlist->vertex_list;
		if (ovlist->normal_list) nlist = ovlist->normal_list;
		if (ovlist->color_list) clist = ovlist->color_list;
	}
	
	OMedia3DMaterial					*mat = NULL,*newmat;
	omt_PolygonList::iterator			poly_i;
	omt_3DPolygonVertexList::iterator	i,vend;
	bool								second_pass_found = false;

	for(poly_i=polygons->begin();
		poly_i!=polygons->end();
		poly_i++)
	{
		if (picking_mode) set_picking_id(poly_i-polygons->begin());	
	
		newmat = (*poly_i).get_material();
		if (!newmat || ((*poly_i).get_flags()&om3pf_Hide)) continue;
	
		if (newmat!=mat)
		{
			if (!inout_second_pass)
			{
				if (newmat->get_flags()&ommatf_SecondPass)
				{
					second_pass_found = true;
					continue;
				}
			}
			else
			{
				if (!(newmat->get_flags()&ommatf_SecondPass)) continue;
			}
		
			mat = newmat;

			set_texture(mat->get_texture());
			texture_color_mode=mat->get_texture_color_operation();

			if (mat->get_blend_src()==omblendfc_One &&
				mat->get_blend_dest()==omblendfc_Zero) 
			{
				blend_enabled = false;
			}
			else
			{
				blend_enabled = true;
				blend_src = mat->get_blend_src();
				blend_dest = mat->get_blend_dest();
			}
			
			if (mat->get_light_mode()==ommlmc_Light)
			{
				light_enabled = true;

				set_material(mat->get_emission_ref(),
							 mat->get_diffuse_ref(),
							 mat->get_specular_ref(),
							 mat->get_ambient_ref(),
							 mat->get_shininess());					
			}
			else	
			{
				light_enabled = false;			
			}

			if (!override_mat_fillmode) fill_mode = mat->get_fill_mode();

			shade_mode = mat->get_shade_mode();
		}
		
		if ( ((*poly_i).get_flags()&om3pf_TwoSided) && culling_enabled) 		
		{
			culling_enabled = false;		
		}
		else if ( !((*poly_i).get_flags()&om3pf_TwoSided) && !culling_enabled)
		{
			culling_enabled = true;		
		}		
	
		switch(fill_mode)
		{
			case omfillmc_Point:
			begin_gen_polygon(omrfmc_PointList, (*poly_i).get_vertices().size());
			break;
			
			case omfillmc_Line:
			begin_gen_polygon(omrfmc_LineLoop, (*poly_i).get_vertices().size());
			break;

			case omfillmc_Solid:
			begin_gen_polygon(omrfmc_TriangleFan, (*poly_i).get_vertices().size());
			break;
			
			default:
			continue;
		}
		
		vend = (*poly_i).get_vertices().end();


		switch(mat->get_light_mode())
		{
			case ommlmc_Color:
			{
				OMediaFARGBColor	argb;
				
				mat->get_diffuse(argb);

				for(i=(*poly_i).get_vertices().begin();
					i!=vend;
					i++)
				{
					OMedia3DPoint	*p = &(*(vlist->begin() + (*i).vertex_index));
					gen_vertex(*p,NULL,&argb,(*i).u,(*i).v);
				}
			}
			break;

			case ommlmc_VertexColor:
			for(i=(*poly_i).get_vertices().begin();
				i!= vend;
				i++)
			{
				OMediaFARGBColor	*argb = &(*(clist->begin() + (*i).color_index));
				OMedia3DPoint		*p = &(*(vlist->begin() + (*i).vertex_index));	

				gen_vertex(*p,NULL,argb,(*i).u,(*i).v);
			}
			break;		

			case ommlmc_Light:
			if (shade_mode==omshademc_Flat)
			{
				for(i=(*poly_i).get_vertices().begin();
					i!=vend;
					i++)
				{
					OMedia3DPoint		*p = &(*(vlist->begin() + (*i).vertex_index));	
					
					gen_vertex(*p,&((*poly_i).get_normal()),NULL,(*i).u,(*i).v);
				}
			}
			else
			{
				for(i=(*poly_i).get_vertices().begin();
					i!=vend;
					i++)
				{
					OMedia3DPoint		*p = &(*(vlist->begin() + (*i).vertex_index));
					OMedia3DVector		*n = &(*(nlist->begin() + (*i).normal_index));

					gen_vertex(*p,n,NULL,(*i).u,(*i).v);	
				}			
			}
			break;
		}
	
		end_gen_polygon();		
	}
	
	inout_second_pass = second_pass_found;

	shape->unlock();
}								

void OMediaPipeline::start_picking(unsigned long max_ids)
{
	picking_mode = true;
	hit_list.erase(hit_list.begin(),hit_list.end());
}

vector<OMediaPickHit> *OMediaPipeline::end_picking(void)
{
	picking_mode = false;	
	return &hit_list;
}
	
void OMediaPipeline::set_picking_id(unsigned long id)
{
	pick_newhit.id = id;
}
	
OMediaPickRequest *OMediaPipeline::get_pick_mode(void)
{
	return pipe2rnd_interface->get_target()->get_renderer()->get_picking_mode();
}
		
void OMediaPipeline::flush_pipeline(void)
{
	pipe2rnd_interface->flush_pipeline();
}	

void OMediaPipeline::enable_fog(bool enable) {}
void OMediaPipeline::set_fog_density(float d) {}
void OMediaPipeline::set_fog_color(const OMediaFARGBColor &argb) {}
void OMediaPipeline::set_fog_range(float start, float end) {}
void OMediaPipeline::enable_texture_persp(bool enabled) {}



//--------------

void OMediaPipeline::gen_surface(OMediaCanvas *canv,float *xyzw1, float *xyzw2,float alpha)
{
	OMediaPipePoint			*p,*base_p;
	OMediaPipePolygon		*poly;	
	omt_ZBufferTest 		zt; 
	omt_ZBufferWrite 		zw;	


	point_buffer->mark_temporary_points();

	p = base_p = point_buffer->get_points(2);
	p->xyzw[0] = xyzw1[0];	p->xyzw[1] = xyzw1[1];	p->xyzw[2] = xyzw1[2];	p->xyzw[3] = xyzw1[3];	
	p->a = alpha;
	p++;

	p->xyzw[0] = xyzw2[0];	p->xyzw[1] = xyzw2[1];	p->xyzw[2] = xyzw2[2];	p->xyzw[3] = xyzw2[3];

	

	poly = poly_buffer->get_polygon();
	
	poly->points = base_p;
	poly->npoints = 2;
	poly->surface = canv;
	poly->flags = omppf_FlatSurface;

	pipe2rnd_interface->get_zbuffer_info(zt,zw, poly->zfunc);
	if (zt==omzbtc_Enabled) poly->flags |= omppf_ZTest;
	if (zw==omzbwc_Enabled) poly->flags |= omppf_ZWrite;

	if (get_blend_enabled())
	{
		poly->src_blend = get_src_blend();
		poly->dest_blend =get_dest_blend();
	}
	else
	{
		poly->src_blend = omblendfc_One;
		poly->dest_blend = omblendfc_Zero;
	}		
		
	if (picking_mode)
	{

		if (hit_list.size()==0 ||
			hit_list.back().id!=pick_newhit.id)
		{
			hit_list.push_back(pick_newhit);
			pick_find_zrange(poly, hit_list.back().minz, hit_list.back().maxz,true);
		}
		else
		{
			pick_find_zrange(poly, hit_list.back().minz, hit_list.back().maxz,false);				
		}
		
		poly_buffer->release_polygon();
		point_buffer->release_temporary_points();		
	}		
}

void OMediaPipeline::begin_gen_polygon(omt_RasterizerFillMode fmode, long	nvertices)
{
	gen_fill_mode = fmode;
	point_buffer->mark_temporary_points();
	gen_base_npoints = nvertices;
	gen_base_p = gen_cur_p = point_buffer->get_points(gen_base_npoints);
}

void OMediaPipeline::gen_vertex(OMedia3DPoint		&p,
								OMedia3DVector		*normal,
								OMediaFARGBColor	*color,
								float				u,
								float				v)
{
	gen_cur_p->xyzw[0] = p.x;
	gen_cur_p->xyzw[1] = p.y;
	gen_cur_p->xyzw[2] = p.z; 
	gen_cur_p->xyzw[3] = 1.0f;
	
	if (normal) gen_cur_p->n = *normal;
	
	if (color)
	{
		gen_cur_p->a = color->alpha;
		gen_cur_p->dr = color->red;
		gen_cur_p->dg = color->green;
		gen_cur_p->db = color->blue;
	}
	
	gen_cur_p->u = u;
	gen_cur_p->v = v;
		
	gen_cur_p++;
}
								

void OMediaPipeline::end_gen_polygon(void)
{
	long				c,base_c;
	OMediaPipePoint		*p,*base_p;
	bool				do_culling, do_light = light_enabled && !picking_mode;
	float				xyzw[3][4];
	OMedia3DVector		flat_n;
	bool				texture_replace = (texture && texture_color_mode==omtcoc_Replace);

	base_c = gen_base_npoints;
	base_p = gen_base_p;
	
	do_culling = culling_enabled && base_c>=3;
	if (base_c<3 || texture_replace) do_light = false;

	if (do_culling && do_light && shade_mode==omshademc_Gouraud)
	{
		float				pxyzw[3][4];	// Projected
		float				nh_xyzw[3][4];	// Non-homogenous coordinates
	
		p = base_p;
		p->flags = 0;	copy_xyzw(p->xyzw,xyzw[0]);	transform_xyzw(xyzw[0],pxyzw[0],nh_xyzw[0]);	p++;
		p->flags = 0;	copy_xyzw(p->xyzw,xyzw[1]);	transform_xyzw(xyzw[1],pxyzw[1],nh_xyzw[1]);	p++;
		p->flags = 0;	copy_xyzw(p->xyzw,xyzw[2]);	transform_xyzw(xyzw[2],pxyzw[2],nh_xyzw[2]);
	
		if (backface_culled(nh_xyzw[0], nh_xyzw[1], nh_xyzw[2])) 
		{
			point_buffer->release_temporary_points();
			return;
		}

		p = base_p;
		copy_xyzw(xyzw[0],p->xyzw); compute_light(p);	p++;
		copy_xyzw(xyzw[1],p->xyzw); compute_light(p);	p++;
		copy_xyzw(xyzw[2],p->xyzw); compute_light(p);			
		
		p = base_p;
		copy_xyzw(pxyzw[0],p->xyzw);	p++;
		copy_xyzw(pxyzw[1],p->xyzw);	p++;
		copy_xyzw(pxyzw[2],p->xyzw);	p++;

		base_c -= 3;
		base_p = p;	
	}

	p = base_p;
	c = base_c;
	
	while(c--)
	{
		model_view.multiply(p->xyzw);
		
		p->flags = 0;

		if (texture_replace)
		{
			p->sr = 0.0f;
			p->sg = 0.0f;
			p->sb = 0.0f;
			p->a = 1.0f;
			p->dr = 1.0f;
			p->dg = 1.0f;
			p->db = 1.0f;
		}
		else if (!do_light)
		{
			p->sr = 0.0f;
			p->sg = 0.0f;
			p->sb = 0.0f;
		}
		else
		{
			if (shade_mode==omshademc_Gouraud) compute_light(p);
			else if (p==base_p) 
			{
				copy_xyzw(p->xyzw,xyzw[0]);
				flat_n = p->n;
			}
		}
	
		projection.multiply(p->xyzw);			
		p++;
	}
	
	// Get a polygon
	
	OMediaPipePolygon		*poly;	
	omt_ZBufferTest 		zt; 
	omt_ZBufferWrite 		zw;	
	
	poly = poly_buffer->get_polygon();
	
	poly->points = gen_base_p;
	poly->npoints = gen_base_npoints;
	poly->texture = texture;
	poly->flags = 0;

	pipe2rnd_interface->get_zbuffer_info(zt,zw, poly->zfunc);
	if (zt==omzbtc_Enabled) poly->flags |= omppf_ZTest;
	if (zw==omzbwc_Enabled) poly->flags |= omppf_ZWrite;

	if (get_blend_enabled())
	{
		poly->src_blend = get_src_blend();
		poly->dest_blend =get_dest_blend();
	}
	else
	{
		poly->src_blend = omblendfc_One;
		poly->dest_blend = omblendfc_Zero;
	}

	if (shade_mode==omshademc_Gouraud && !texture_replace) poly->flags |= omppf_Gouraud;
	if (gen_fill_mode==omrfmc_LineLoop) poly->flags |= omppf_Lines;

	// Quick clipping

	bool	all_clipped = true;
	bool	all_inside = true;

	p = poly->points;
	c = poly->npoints;
	while(c--)
	{
		if (p->xyzw[0] < -p->xyzw[3] || p->xyzw[0] > p->xyzw[3] ||
			p->xyzw[1] < -p->xyzw[3] || p->xyzw[1] > p->xyzw[3] ||
			p->xyzw[2] < -p->xyzw[3] || p->xyzw[2] > p->xyzw[3])
		{
			p->flags |= omppf_Clipped;
			all_inside = false;
		}
		else all_clipped = false;
		
		p++;
	}
		
	if (gen_fill_mode==omrfmc_PointList)
	{
		if (all_clipped)
		{
			poly_buffer->release_polygon();
			point_buffer->release_temporary_points();
			return;		
		}
		
		all_inside = true;
	
		poly->flags |= omppf_Points;
	}

	// Clipping

	if (all_inside ||
		(!clip_polygon(poly, omclippc_Left) 		&&
		!clip_polygon(poly, omclippc_Right) 	&&
		!clip_polygon(poly, omclippc_Top) 		&&
		!clip_polygon(poly, omclippc_Bottom) 	&&
		!clip_polygon(poly, omclippc_Near) 		&&
		!clip_polygon(poly, omclippc_Far)))
	{
		p = poly->points;
		c = poly->npoints;
		while(c--)
		{
			p->inv_w = transform_homogenous_coord(p->xyzw);	
			p++;
		}
		
		if (do_culling && (!do_light || shade_mode==omshademc_Flat))
		{
			p = poly->points;
			if (backface_culled(p[0].xyzw, p[1].xyzw, p[2].xyzw)) 
			{
				poly_buffer->release_polygon();
				point_buffer->release_temporary_points();
				return;
			}
			else
			{
				if (do_light)
				{
					copy_xyzw(p->xyzw,xyzw[1]);
					copy_xyzw(xyzw[0],p->xyzw);
					p->n = flat_n;
					compute_light(p);
					copy_xyzw(xyzw[1],p->xyzw);				
				}			
			}
		}
		
		if (picking_mode)
		{
			if (hit_list.size()==0 ||
				hit_list.back().id!=pick_newhit.id)
			{
				hit_list.push_back(pick_newhit);
				pick_find_zrange(poly, hit_list.back().minz, hit_list.back().maxz,true);
			}
			else
			{
				pick_find_zrange(poly, hit_list.back().minz, hit_list.back().maxz,false);				
			}
		
			poly_buffer->release_polygon();
			point_buffer->release_temporary_points();		
			return;	
		}		
	}
	else
	{
		poly_buffer->release_polygon();
		point_buffer->release_temporary_points();
	}
}

void OMediaPipeline::pick_find_zrange(	OMediaPipePolygon *poly, 
											float &min_z, float &max_z,
											bool	newhit)
{
	long 	i;
	float	z;

	for(i=0;i<poly->npoints;i++)
	{
		z = poly->points[i].xyzw[2];

		if (newhit) 
		{
			newhit = false;
			min_z = z; 
			max_z = z; 
		}
		else
		{
			if (z<min_z) min_z = z;
			if (z>max_z) max_z = z;
		}
	}
}


//----------------------------------------------

void OMediaPipeline::compute_light(OMediaPipePoint *p)
{
	OMediaPipeLight	*l,*light_end;
	float			a,r,g,b,sr,sg,sb;
	bool			only_constant,do_specular;
	OMedia3DVector	dv;
	float			d,spec_f;
	float			shininess;
	
	
	l = lights;
	light_end = l + omd_MAX_PIPELINE_LIGHTS;

	inv_model_view.normal_transform(p->n,p->n);

	// emission
	
	a = material.emission.alpha;
	r = material.emission.red;
	g = material.emission.green;
	b = material.emission.blue;

	// ambient

	a += material.ambient.alpha * global_ambient.alpha;
	r += material.ambient.red * global_ambient.red;
	g += material.ambient.green * global_ambient.green;
	b += material.ambient.blue * global_ambient.blue;


	// Specular
	
	do_specular = 	material.specular.red!=0.0f ||
					material.specular.green!=0.0f ||
					material.specular.blue!=0.0f;
					
	sr = sg = sb = 0.0f;
	shininess = material.shininess;

	for(;l!=light_end;l++)
	{
		float	af;
	
		if (!l->enabled) continue;

		float			cat=l->constant_attenuation,
						lat=l->linear_attenuation,
						qat=l->quadratic_attenuation;
	
		if (l->type==omclt_Directional) only_constant = true;
		else
		{
			only_constant = (lat==0.0f && qat==0.0f);		
			if (only_constant && cat==0.0f) continue;
		}

		af = cat;

		if (!only_constant || do_specular) 
		{			
			dv.x = p->xyzw[0] - l->pos.x;
			dv.y = p->xyzw[1] - l->pos.y;
			dv.z = p->xyzw[2] - l->pos.z;
			
			d = dv.quick_magnitude();

		
			if (!only_constant)
			{
				af += lat * d;
				af += qat * d * d;				
			}	
		}
		
		af = 1.0f / af;
	
		if (do_specular)
		{
			if (dv.dot_product(p->n)<0.0f)
			{
				OMedia3DVector		eye_v;
				float	id = 1.0f/d;
			
				dv.x *= id;		dv.y *= id;		dv.z *= id;
			
				eye_v.x = p->xyzw[0];
				eye_v.y = p->xyzw[1];
				eye_v.z = p->xyzw[2];
				eye_v.normalize();
				
				eye_v.x += dv.x;
				eye_v.y += dv.y;
				eye_v.z += dv.z;
				eye_v.normalize();
				
				spec_f = -eye_v.dot_product(p->n);
				if (spec_f<0.0f) spec_f = 0.0f;
				
				if (shininess!=1.0f) spec_f = pow(spec_f, shininess);
			}
			else spec_f = 0.0f;
		}
	
		switch(l->type)
		{
			case omclt_Directional:
			af  *= -p->n.dot_product(l->vect);
			break;

			case omclt_Spot:
			break;
                        
                        default:
                        break;
		}
		
		a += material.diffuse.alpha * af;	
		r += material.diffuse.red 	* af;	
		g += material.diffuse.green * af;	
		b += material.diffuse.blue 	* af;
		
		if (do_specular)
		{
			spec_f *= af;
			sr += material.specular.red * spec_f;
			sg += material.specular.green * spec_f;
			sb += material.specular.blue * spec_f;		
		}	
	}
	
	if (a>1.0f) a = 1.0f;	else if (a<0.0f) a = 0.0f;
	if (r>1.0f) r = 1.0f;	else if (r<0.0f) r = 0.0f;
	if (g>1.0f) g = 1.0f;	else if (g<0.0f) g = 0.0f;
	if (b>1.0f) b = 1.0f;	else if (b<0.0f) b = 0.0f;
	if (sr>1.0f) sr = 1.0f;	else if (sr<0.0f) sr = 0.0f;
	if (sg>1.0f) sg = 1.0f;	else if (sg<0.0f) sg = 0.0f;
	if (sb>1.0f) sb = 1.0f;	else if (sb<0.0f) sb = 0.0f;
	
	p->a = a;
	p->dr = r;
	p->dg = g;
	p->db = b;
	p->sr = sr;
	p->sg = sg;
	p->sb = sb;
}

#endif

