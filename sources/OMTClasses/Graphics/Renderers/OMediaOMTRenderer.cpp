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
#include "OMediaOMTRenderer.h"
#include "OMediaBlitter.h"
#include "OMediaRastBlitter.h"
#include "OMediaRendererInterface.h"
#include "OMediaVideoEngine.h"
#include "OMediaError.h"
#include "OMediaWindow.h"
#include "OMediaPipeline.h"
#include "OMediaOffscreenBuffer.h"
#include "OMediaTriangleRasterizer.h"
#include "OMediaTriangleRasterLine.h"
#include "OMediaOMTCanvas.h"
#include "OMediaCanvas.h"
#include "OMediaEndianSupport.h"

#define omd_ShrinkTCoord(x) {if (x>0.0) x-=0.0001f; else if (x<0.0) x +=0.0001f;}

unsigned short		*OMediaOMTRenderPort::zbuffer;
unsigned long		OMediaOMTRenderPort::zbuffer_length;
unsigned long		OMediaOMTRenderPort::port_counter;


OMediaOMTRenderPort::OMediaOMTRenderPort(OMediaRenderTarget *target):
					  OMediaRenderPort(target)
{
	port_counter++;

	bounds.set(0,0,0,0);
	current_bounds.set(0,0,0,0);
	
	cbuffer = NULL;

	pipeline = new OMediaPipeline(this);	

	zb_write = omzbwc_Disabled;
	zb_test = omzbtc_Disabled;
	zb_func = omzbfc_Never;
	zmodulo = 0;
	
	half_vpw = half_vph = 0.0f;
	
}

OMediaOMTRenderPort::~OMediaOMTRenderPort()
{
	port_counter--;

	delete cbuffer;
	delete pipeline;

	if (port_counter==0)
	{
		delete [] zbuffer;
		zbuffer_length = 0;
		zbuffer = NULL;
	}
}

void OMediaOMTRenderPort::capture_frame(OMediaCanvas &canv)
{
	unsigned char	*dest_pix;
	long			w,h;
	void				*p;
	long		rowbytes;

	prepare_context();
	if (!cbuffer) return;

	w = cbuffer->get_width();
	h = cbuffer->get_height();

	canv.create(w,h);
	canv.lock(omlf_Write);
	cbuffer->lock();

	dest_pix = (unsigned char*)canv.get_pixels();

	if (cbuffer->get_pixel_format()==omobfc_ARGB8888)
	{
		long				x,y;
		unsigned char		*src_pix,*srcl_pix;
		unsigned long		pix;

		cbuffer->get_pixmap(p, rowbytes);
		srcl_pix = (unsigned char*)p;

		for(y=0;y<h;y++)
		{		
			src_pix = srcl_pix;
			for(x=0;x<w;x++)
			{
				pix = *((unsigned long*)src_pix);

				dest_pix[0] = (pix>>16L)&0xFFL;
				dest_pix[1] = (pix>>8L)&0xFFL;
				dest_pix[2] = (pix)&0xFFL;
				dest_pix[3] = (pix>>24L)&0xFFL;
				src_pix+=4;
				dest_pix+=4;
			}
			srcl_pix +=rowbytes;
		}
	}
	else if (cbuffer->get_pixel_format()==omobfc_ARGB1555)
	{
		long				x,y;
		unsigned char		*src_pix,*srcl_pix;
		unsigned short		pix;

		cbuffer->get_pixmap(p, rowbytes);
		srcl_pix = (unsigned char*)p;

		for(y=0;y<h;y++)
		{		
			src_pix = srcl_pix;
			for(x=0;x<w;x++)
			{
				pix = *((unsigned short*)src_pix);

				dest_pix[0] = (((pix>>10L)&0x1F) * 0xFFL) / 0x1FL;
				dest_pix[1] = (((pix>>5L)&0x1F) * 0xFFL) / 0x1FL;
				dest_pix[2] = (((pix)&0x1F) * 0xFFL) / 0x1FL;
				dest_pix[3] = (pix&(1<<15))?0xFF:0;
				src_pix+=2;
				dest_pix+=4;
			}
			srcl_pix +=rowbytes;
		}
	}


	cbuffer->unlock();
	canv.unlock();
}


void OMediaOMTRenderPort::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_OffscreenBufferDeleted:
		cbuffer = NULL;
		break;
	}
}

void OMediaOMTRenderPort::set_bounds(OMediaRect &rect)
{
	bounds = rect;
}

void OMediaOMTRenderPort::prepare_context(void)
{
	OMediaRect				final_bounds,target_bounds,device_bounds;
	OMediaOMTRenderer		*renderer = (OMediaOMTRenderer*) target->get_renderer();
	OMediaOMTRenderTarget	*target = (OMediaOMTRenderTarget*)this->target;
	OMediaWindow			*window = target->get_window();
	OMediaVideoMode			*vmode = renderer->get_video_engine()->get_current_video_mode();
	
	if (window) 
	{
		device_bounds.left = device_bounds.right = vmode->its_card->positionx;
		device_bounds.top = device_bounds.bottom = vmode->its_card->positiony;
		device_bounds.right += vmode->width;
		device_bounds.bottom += vmode->height;

		target_bounds.left = target_bounds.right = window->get_x();
		target_bounds.top = target_bounds.bottom = window->get_y();
		target_bounds.right += window->get_width();
		target_bounds.bottom += window->get_height();

		if (!device_bounds.touch(&target_bounds))
		{
			delete cbuffer;
			cbuffer = NULL;
			return;
		}

		target_bounds.offset(-target_bounds.left,-target_bounds.top);
	}
	else 
	{
		delete cbuffer;
		cbuffer = NULL;
		return;
	}
	
	final_bounds = bounds;
	final_bounds.offset(target_bounds.left,target_bounds.top);

	if (final_bounds.right>target_bounds.right) final_bounds.right = target_bounds.right;
	if (final_bounds.bottom>target_bounds.bottom) final_bounds.bottom = target_bounds.bottom;

	if (current_bounds.is_equal(&final_bounds) && 
		!target->rebuild_contexts && 
		!final_bounds.empty() &&
		cbuffer) return;

	current_bounds = final_bounds;

	delete cbuffer;
	cbuffer = NULL;

	if (current_bounds.empty()) return;

	OMediaVideoEngine *vengine = renderer->get_video_engine();

	if (vengine->get_current_video_mode()->depth<=16) 
		cbuffer = vengine->create_offscreen_buffer(current_bounds.get_width(),
													current_bounds.get_height(),omobfc_ARGB1555);
	else
		cbuffer = vengine->create_offscreen_buffer(current_bounds.get_width(),
													current_bounds.get_height(),omobfc_ARGB8888);

	cbuffer->addlistener(this);

	cbuff_fw = float(current_bounds.get_width());
	cbuff_fh = float(current_bounds.get_height());

	half_vpw = (cbuff_fw*0.5f)-0.1f;
	half_vph = (cbuff_fh*0.5f)-0.1f;

	if (renderer->zbuffer)
	{
		long	zbsize;

		zbsize = current_bounds.get_width() * current_bounds.get_height();
		zmodulo = current_bounds.get_width()<<1;
	
		if (zbsize>(long)zbuffer_length)
		{
			delete [] zbuffer;
			zbuffer = new unsigned short[zbsize];
			if (!zbuffer) omd_EXCEPTION(omcerr_OutOfMemory);
			zbuffer_length = zbsize;
		}
	}
}


void OMediaOMTRenderPort::render(void)
{
	if (begin_render())
	{
 		OMediaRendererInterface	*iface = pipeline; 
		broadcast_message(omsg_RenderPort,iface);		
		end_render();

		if (!get_target()->get_renderer()->get_picking_mode())
		{	
			cbuffer->draw(get_target()->get_window(), current_bounds.left, current_bounds.top);
		}
	}
}

bool OMediaOMTRenderPort::begin_render(void)
{
	prepare_context();
	if (!cbuffer) return false;

	pipeline->begin(0);

	return true;
}

void OMediaOMTRenderPort::send_polygons(void)
{
	if (!get_target()->get_renderer()->get_picking_mode())
	{	
		OMediaPipePolygonBuffer	*buf = pipeline->get_polygons();
		OMediaPipePolygon		*poly;
	
		void	*vb;

		cbuffer->lock();
		cbuffer->get_pixmap(vb, cb_rowbytes);
		cb_pixels = (char*)vb;
		
		for(buf->scan_start(); !buf->scan_end(); buf->scan_next())
		{
			poly = buf->scan_polygon();
		
			if (poly->flags&omppf_FlatSurface)
				render_polygon_surface(poly);	
			else if (poly->flags&omppf_Points)
				render_polygon_points(poly);	
			else if (poly->flags&omppf_Lines)
				render_polygon_lines(poly);	
			else if (poly->texture)
			{
				if (poly->texture->argb1555)
					render_polygon_texture16(poly);	
				else
					render_polygon_texture32(poly);	
			}
			else 
				render_polygon_shaded(poly);	
		}
	
		cbuffer->unlock();
	}
}

void OMediaOMTRenderPort::end_render(void)
{
	pipeline->end();
	send_polygons();
}

//----------------------------------
// Render

unsigned long OMediaOMTRenderPort::point_to_directcolor(OMediaPipePoint	*p)
{
	unsigned long	a,r,g,b;

	a = (unsigned long)((p->a * float(0xFF)));
	r = (unsigned long)((p->dr * float(0xFF)) + (p->sr * float(0xFF)));
	g = (unsigned long)((p->dg * float(0xFF)) + (p->sg * float(0xFF)));
	b = (unsigned long)((p->db * float(0xFF)) + (p->sb * float(0xFF)));

	if (a>0xFF) a=0xFF;
	if (r>0xFF) r=0xFF;
	if (g>0xFF) g=0xFF;
	if (b>0xFF) b=0xFF;

	return ((a<<24UL)|(r<<16UL)|(g<<8UL)|b);
}

void OMediaOMTRenderPort::render_polygon_points(OMediaPipePolygon *poly)
{
	OMediaPipePoint		*pipe_p,*pipe_end;
	char				*pixels = cb_pixels;
	long				rowbytes = cb_rowbytes;
	omt_BlendFunc		src_blend,dest_blend;
	omt_ZBufferWrite	zb_write;
	omt_ZBufferTest		zb_test;
	omt_ZBufferFunc		zb_func;
	
	src_blend = poly->src_blend;
	dest_blend = poly->dest_blend;
	
	zb_write = (poly->flags&omppf_ZWrite)?omzbwc_Enabled:omzbwc_Disabled;
	zb_test = (poly->flags&omppf_ZTest)?omzbtc_Enabled:omzbtc_Disabled;
	zb_func = poly->zfunc;

	// Transform to screen coordinates

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;

	for(; pipe_p!=pipe_end; pipe_p++)
	{
		if (pipe_p->xyzw[0]<-1.0f) pipe_p->xyzw[0] = -1.0f;
		else if (pipe_p->xyzw[0]> 1.0f) pipe_p->xyzw[0] =  1.0f;
		if (pipe_p->xyzw[1]<-1.0f) pipe_p->xyzw[1] = -1.0f;
		else if (pipe_p->xyzw[1]> 1.0f) pipe_p->xyzw[1] =  1.0f;			

		pipe_p->xyzw[0] = ((pipe_p->xyzw[0]*half_vpw) + half_vpw);
		pipe_p->xyzw[1] = ((pipe_p->xyzw[1]*-half_vph) + half_vph);	
		pipe_p->xyzw[2] = ((pipe_p->xyzw[2] + 1.0f) * 0.5f);
			
		if (pipe_p->xyzw[0]<0.0f) pipe_p->xyzw[0]=0.0f;
		else if (pipe_p->xyzw[0]>=cbuff_fw) pipe_p->xyzw[0] = cbuff_fw-1.0f;

		if (pipe_p->xyzw[1]<0.0f) pipe_p->xyzw[1]=0.0f;
		else if (pipe_p->xyzw[1]>=cbuff_fh) pipe_p->xyzw[1] = cbuff_fh-1.0f;	
	}

	// Rasterize

	long	px,py,pz;

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;

	switch(cbuffer->get_pixel_format())
	{
		case omobfc_ARGB1555:
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (src_blend==omblendfc_One && dest_blend==omblendfc_Zero)
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
					pz = (long)(pipe_p->xyzw[2]*float(omd_ZBufferPixScale));
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						OMediaSegmentRasterizer::fill_color_zbuffer16(
									(unsigned short*)(pixels + (px<<1) + (py*rowbytes)),
									point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points),
									(unsigned short*)((char*)zbuffer + (px<<1) + (py*zmodulo)),
									pz,0,1,zb_func,zb_test,zb_write);						
					}
				}	
			}
			else
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
					pz = (long)(pipe_p->xyzw[2]*float(omd_ZBufferPixScale));
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						OMediaSegmentRasterizer::fill_color_blend_zbuffer16(
									(unsigned short*)(pixels + (px<<1) + (py*rowbytes)),
									point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points),
									(unsigned short*)((char*)zbuffer + (px<<1) + (py*zmodulo)),
									pz,0,1,zb_func,zb_test,zb_write,src_blend,dest_blend);
					}
				}	
			}				
		}
		else 
		{
			if (src_blend==omblendfc_One && dest_blend==omblendfc_Zero)
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						unsigned long pix16 = point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points);
						pix16 = omd_ARGB32to16(pix16);
					
						*((unsigned short*)(pixels + (px<<1) + (py*rowbytes))) = (unsigned short)pix16;
					}
				}	
			}
			else
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						OMediaSegmentRasterizer::fill_color_blend16(
									(unsigned char*)(pixels + (px<<1) + (py*rowbytes)),
									point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points),1,
									src_blend,dest_blend);
					}
				}	
			}
		}
		break;

		case omobfc_ARGB8888:			
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (src_blend==omblendfc_One && dest_blend==omblendfc_Zero)
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
					pz = (long)(pipe_p->xyzw[2]*float(omd_ZBufferPixScale));
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						OMediaSegmentRasterizer::fill_color_zbuffer32(
									(unsigned long*)(pixels + (px<<2) + (py*rowbytes)),
									point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points),
									(unsigned short*)((char*)zbuffer + (px<<1) + (py*zmodulo)),
									pz,0,1,zb_func,zb_test,zb_write);						
					}
				}	
			}
			else
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
					pz = (long)(pipe_p->xyzw[2]*float(omd_ZBufferPixScale));
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						OMediaSegmentRasterizer::fill_color_blend_zbuffer32(
									(unsigned long*)(pixels + (px<<2) + (py*rowbytes)),
									point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points),
									(unsigned short*)((char*)zbuffer + (px<<1) + (py*zmodulo)),
									pz,0,1,zb_func,zb_test,zb_write,src_blend,dest_blend);
					}
				}	
			}				
		}
		else
		{
			if (src_blend==omblendfc_One && dest_blend==omblendfc_Zero)
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						*((unsigned long*)(pixels + (px<<2) + (py*rowbytes))) = point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points);
					}
				}	
			}
			else
			{
				for(; pipe_p!=pipe_end; pipe_p++)
				{
					px = (long)pipe_p->xyzw[0];
					py = (long)pipe_p->xyzw[1];
				
					if (!(pipe_p->flags & omppf_Clipped)) 
					{
						OMediaSegmentRasterizer::fill_color_blend32(
									(unsigned char*)(pixels + (px<<2) + (py*rowbytes)),
									point_to_directcolor((poly->flags&omppf_Gouraud)?pipe_p: poly->points),1,
									src_blend,dest_blend);
					}
				}	
			}
		}
		break;
	}

}

void OMediaOMTRenderPort::render_polygon_lines(OMediaPipePolygon *poly)
{
	OMediaPipePoint		*pipe_p,*pipe_end;
	char				*pixels = cb_pixels;
	long				rowbytes = cb_rowbytes;
	omt_BlendFunc		src_blend,dest_blend;
	omt_ZBufferWrite	zb_write;
	omt_ZBufferTest		zb_test;
	omt_ZBufferFunc		zb_func;
	
	src_blend = poly->src_blend;
	dest_blend = poly->dest_blend;
	
	zb_write = (poly->flags&omppf_ZWrite)?omzbwc_Enabled:omzbwc_Disabled;
	zb_test = (poly->flags&omppf_ZTest)?omzbtc_Enabled:omzbtc_Disabled;
	zb_func = poly->zfunc;

	// Transform to screen coordinates

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;

	for(; pipe_p!=pipe_end; pipe_p++)
	{
		if (pipe_p->xyzw[0]<-1.0f) pipe_p->xyzw[0] = -1.0f;
		else if (pipe_p->xyzw[0]> 1.0f) pipe_p->xyzw[0] =  1.0f;
		if (pipe_p->xyzw[1]<-1.0f) pipe_p->xyzw[1] = -1.0f;
		else if (pipe_p->xyzw[1]> 1.0f) pipe_p->xyzw[1] =  1.0f;			

		pipe_p->xyzw[0] = ((pipe_p->xyzw[0]*half_vpw) + half_vpw);
		pipe_p->xyzw[1] = ((pipe_p->xyzw[1]*-half_vph) + half_vph);	
		pipe_p->xyzw[2] = ((pipe_p->xyzw[2] + 1.0f) * 0.5f);
		
		if (pipe_p->xyzw[0]<0.0f) pipe_p->xyzw[0]=0.0f;
		else if (pipe_p->xyzw[0]>=cbuff_fw) pipe_p->xyzw[0] = cbuff_fw-1.0f;

		if (pipe_p->xyzw[1]<0.0f) pipe_p->xyzw[1]=0.0f;
		else if (pipe_p->xyzw[1]>=cbuff_fh) pipe_p->xyzw[1] = cbuff_fh-1.0f;	
	}
	

	// Rasterize
	
	OMediaPipePoint	*pipe_np;
	long	 n = poly->npoints;

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;
	
	if (n<=2) n--;

	switch(cbuffer->get_pixel_format())
	{
		case omobfc_ARGB1555:
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{	
			while(n--)
			{
				pipe_np = pipe_p+1;
				if (pipe_np==pipe_end) pipe_np = poly->points;
			
				OMediaSegmentRasterizer::draw_line_zb16((unsigned short*)pixels, rowbytes,
														(short)pipe_p->xyzw[0],
														(short)pipe_p->xyzw[1],
														pipe_p->xyzw[2],
														(short)pipe_np->xyzw[0],
														(short)pipe_np->xyzw[1],
														pipe_np->xyzw[2],
														point_to_directcolor(poly->points),
														src_blend,dest_blend,
														zbuffer,
														zmodulo,
														zb_func,zb_test,zb_write);
								
				pipe_p++;
			}	
		}
		else 
		{
			while(n--)
			{
				pipe_np = pipe_p+1;
				if (pipe_np==pipe_end) pipe_np = poly->points;
			
				OMediaSegmentRasterizer::draw_line_16((unsigned short*)pixels, rowbytes,
														(short)pipe_p->xyzw[0],
														(short)pipe_p->xyzw[1],
														(short)pipe_np->xyzw[0],
														(short)pipe_np->xyzw[1],
														point_to_directcolor(poly->points),
														src_blend,dest_blend);
								
				pipe_p++;
			}	
		
		}
		break;

		case omobfc_ARGB8888:			
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			while(n--)
			{
				pipe_np = pipe_p+1;
				if (pipe_np==pipe_end) pipe_np = poly->points;
			
				OMediaSegmentRasterizer::draw_line_zb32((unsigned long*)pixels, rowbytes,
														(short)pipe_p->xyzw[0],
														(short)pipe_p->xyzw[1],
														pipe_p->xyzw[2],
														(short)pipe_np->xyzw[0],
														(short)pipe_np->xyzw[1],
														pipe_np->xyzw[2],
														point_to_directcolor(poly->points),
														src_blend,dest_blend,
														zbuffer,
														zmodulo,
														zb_func,zb_test,zb_write);
								
				pipe_p++;
			}	
							
		}
		else
		{

			while(n--)
			{
				pipe_np = pipe_p+1;
				if (pipe_np==pipe_end) pipe_np = poly->points;
			
				OMediaSegmentRasterizer::draw_line_32((unsigned long*)pixels, rowbytes,
														(short)pipe_p->xyzw[0],
														(short)pipe_p->xyzw[1],
														(short)pipe_np->xyzw[0],
														(short)pipe_np->xyzw[1],
														point_to_directcolor(poly->points),
														src_blend,dest_blend);									
				pipe_p++;
			}	
		}
		break;
	}
}

void OMediaOMTRenderPort::render_polygon_surface(OMediaPipePolygon *poly)
{
	OMediaPipePoint		*pipe_p,*pipe_end;
	char				*pixels = cb_pixels;
	long				rowbytes = cb_rowbytes;
	omt_BlendFunc		src_blend,dest_blend;
	omt_ZBufferWrite	zb_write;
	omt_ZBufferTest		zb_test;
	omt_ZBufferFunc		zb_func;
	
	src_blend = poly->src_blend;
	dest_blend = poly->dest_blend;
	
	zb_write = (poly->flags&omppf_ZWrite)?omzbwc_Enabled:omzbwc_Disabled;
	zb_test = (poly->flags&omppf_ZTest)?omzbtc_Enabled:omzbtc_Disabled;
	zb_func = poly->zfunc;

	// Transform to screen coordinates

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;

	for(; pipe_p!=pipe_end; pipe_p++)
	{
		pipe_p->xyzw[0] = ((pipe_p->xyzw[0]*half_vpw) + half_vpw);
		pipe_p->xyzw[1] = ((pipe_p->xyzw[1]*-half_vph) + half_vph);	
		pipe_p->xyzw[2] = ((pipe_p->xyzw[2] + 1.0f) * 0.5f);
	}

	// Rasterize
	
	OMediaRect	src,dest;
	pipe_p = poly->points;	

	src.set(0,0,poly->surface->get_width(),poly->surface->get_height());
	dest.set(long(pipe_p[0].xyzw[0]), long(pipe_p[1].xyzw[1]),
			 long(pipe_p[1].xyzw[0]), long(pipe_p[0].xyzw[1]));

	poly->surface->lock(omlf_Read);

	switch(cbuffer->get_pixel_format())
	{
		case omobfc_ARGB1555:
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		OMediaRastBlitter::draw_16zb(poly->surface->get_pixels(), 
								src.right,src.bottom,
								(unsigned short*)pixels,
								cbuffer->get_width(),cbuffer->get_height(),
								rowbytes,
								&src,&dest,
								src_blend,dest_blend,
								(unsigned long)(pipe_p[0].a * float(0xFF)),
								pipe_p[0].xyzw[0],
								zbuffer,
								zmodulo,zb_func,zb_test,zb_write);
		else
		OMediaRastBlitter::draw_16(poly->surface->get_pixels(), 
								src.right,src.bottom,
								(unsigned short*)pixels,
								cbuffer->get_width(),cbuffer->get_height(),
								rowbytes,
								&src,&dest,
								src_blend,dest_blend,
								(unsigned long)(pipe_p[0].a * float(0xFF)));
		break;

		case omobfc_ARGB8888:			
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		OMediaRastBlitter::draw_32zb(poly->surface->get_pixels(), 
								src.right,src.bottom,
								(unsigned long*)pixels,
								cbuffer->get_width(),cbuffer->get_height(),
								rowbytes,
								&src,&dest,
								src_blend,dest_blend,
								(unsigned long)(pipe_p[0].a * float(0xFF)),
								pipe_p[0].xyzw[0],
								zbuffer,
								zmodulo,zb_func,zb_test,zb_write);
		else
		OMediaRastBlitter::draw_32(poly->surface->get_pixels(), 
								src.right,src.bottom,
								(unsigned long*)pixels,
								cbuffer->get_width(),cbuffer->get_height(),
								rowbytes,
								&src,&dest,
								src_blend,dest_blend,
								(unsigned long)(pipe_p[0].a * float(0xFF)));
		break;
	}

	poly->surface->unlock();	
}

void OMediaOMTRenderPort::render_polygon_shaded(OMediaPipePolygon *poly)
{
	OMediaPipePoint		*pipe_p,*pipe_end;
	char				*pixels = cb_pixels;
	long				rowbytes = cb_rowbytes;
	OMediaPipePoint		*p1, *p2, *p3;
	long				nvertex;
	omt_BlendFunc		src_blend,dest_blend;
	omt_ZBufferWrite	zb_write;
	omt_ZBufferTest		zb_test;
	omt_ZBufferFunc		zb_func;
	
	src_blend = poly->src_blend;
	dest_blend = poly->dest_blend;
	
	zb_write = (poly->flags&omppf_ZWrite)?omzbwc_Enabled:omzbwc_Disabled;
	zb_test = (poly->flags&omppf_ZTest)?omzbtc_Enabled:omzbtc_Disabled;
	zb_func = poly->zfunc;


	// Transform to screen coordinates

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;

	for(; pipe_p!=pipe_end; pipe_p++)
	{
		if (pipe_p->xyzw[0]<-1.0f) pipe_p->xyzw[0] = -1.0f;
		else if (pipe_p->xyzw[0]> 1.0f) pipe_p->xyzw[0] =  1.0f;
		if (pipe_p->xyzw[1]<-1.0f) pipe_p->xyzw[1] = -1.0f;
		else if (pipe_p->xyzw[1]> 1.0f) pipe_p->xyzw[1] =  1.0f;			

		pipe_p->xyzw[0] = ((pipe_p->xyzw[0]*half_vpw) + half_vpw);
		pipe_p->xyzw[1] = ((pipe_p->xyzw[1]*-half_vph) + half_vph);	
		pipe_p->xyzw[2] = ((pipe_p->xyzw[2] + 1.0f) * 0.5f);			
	}

	// Rasterize

	nvertex = poly->npoints-2;	
	p1 = poly->points;
	p2 = p1+1;
	p3 = p2+1;
	
	if (cbuffer->get_pixel_format()==omobfc_ARGB1555)
	{
		// 16bits
	
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (poly->flags&omppf_Gouraud)
			{
				OMediaTriangleRasterLineZB_gouraudcopy_16 rasterline(pixels,rowbytes, src_blend,dest_blend,
																	zbuffer,zmodulo,
																	zb_func,zb_test,zb_write);
				
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZGouraudSegment,
										 OMediaTriangleRasterLineZB_gouraudcopy_16>									 
										 rasterizer(&rasterline);	
		
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else
			{
				OMediaTriangleRasterLineZB_flatcopy_16 rasterline(pixels,rowbytes,src_blend,dest_blend,
																	zbuffer,zmodulo,
																	zb_func,zb_test,zb_write,
																	point_to_directcolor(p1));
				
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZSegment,
										 OMediaTriangleRasterLineZB_flatcopy_16>									 
										 rasterizer(&rasterline);	
		
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);				
			
			}
		}
		else 
		{
			if (poly->flags&omppf_Gouraud)
			{
				OMediaTriangleRasterLine_gouraudcopy_16 rasterline(pixels,rowbytes,src_blend,dest_blend);
			
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleGouraudSegment,
										 OMediaTriangleRasterLine_gouraudcopy_16>	rasterizer(&rasterline);
			
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);		
			}
			else 
			{
						
				OMediaTriangleRasterLine_flatcopy_16 rasterline(pixels,rowbytes,src_blend,dest_blend,
																point_to_directcolor(p1));
			
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleSegment,
										 OMediaTriangleRasterLine_flatcopy_16>	rasterizer(&rasterline);
			
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
		}
	}
	else
	{
		// 32bits
	
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (poly->flags&omppf_Gouraud)
			{
				OMediaTriangleRasterLineZB_gouraudcopy_32 rasterline(pixels,rowbytes, src_blend,dest_blend,
																	zbuffer,zmodulo,
																	zb_func,zb_test,zb_write);
				
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZGouraudSegment,
										 OMediaTriangleRasterLineZB_gouraudcopy_32>									 
										 rasterizer(&rasterline);	
		
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else
			{
				OMediaTriangleRasterLineZB_flatcopy_32 rasterline(pixels,rowbytes,src_blend,dest_blend,
																	zbuffer,zmodulo,
																	zb_func,zb_test,zb_write,
																	point_to_directcolor(p1));
				
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZSegment,
										 OMediaTriangleRasterLineZB_flatcopy_32>									 
										 rasterizer(&rasterline);	
		
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);				
			
			}
		}
		else 
		{
			if (poly->flags&omppf_Gouraud)
			{
				OMediaTriangleRasterLine_gouraudcopy_32 rasterline(pixels,rowbytes,src_blend,dest_blend);
			
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleGouraudSegment,
										 OMediaTriangleRasterLine_gouraudcopy_32>	rasterizer(&rasterline);
			
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);		
			}
			else
			{
						
				OMediaTriangleRasterLine_flatcopy_32 rasterline(pixels,rowbytes,src_blend,dest_blend,
																point_to_directcolor(p1));
			
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleSegment,
										 OMediaTriangleRasterLine_flatcopy_32>	rasterizer(&rasterline);
			
				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
		}	
	}
}

void OMediaOMTRenderPort::render_polygon_texture32(OMediaPipePolygon *poly)
{
	OMediaPipePoint		*pipe_p,*pipe_end;
	char				*pixels = cb_pixels;
	long				rowbytes = cb_rowbytes;
	OMediaPipePoint		*p1, *p2, *p3;
	long				nvertex;
	omt_BlendFunc		src_blend,dest_blend;
	omt_ZBufferWrite	zb_write;
	omt_ZBufferTest		zb_test;
	omt_ZBufferFunc		zb_func;
	
	src_blend = poly->src_blend;
	dest_blend = poly->dest_blend;
	
	zb_write = (poly->flags&omppf_ZWrite)?omzbwc_Enabled:omzbwc_Disabled;
	zb_test = (poly->flags&omppf_ZTest)?omzbtc_Enabled:omzbtc_Disabled;
	zb_func = poly->zfunc;


	// Transform to screen coordinates

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;

	for(; pipe_p!=pipe_end; pipe_p++)
	{
		if (pipe_p->xyzw[0]<-1.0f) pipe_p->xyzw[0] = -1.0f;
		else if (pipe_p->xyzw[0]> 1.0f) pipe_p->xyzw[0] =  1.0f;
		if (pipe_p->xyzw[1]<-1.0f) pipe_p->xyzw[1] = -1.0f;
		else if (pipe_p->xyzw[1]> 1.0f) pipe_p->xyzw[1] =  1.0f;			

		pipe_p->xyzw[0] = ((pipe_p->xyzw[0]*half_vpw) + half_vpw);
		pipe_p->xyzw[1] = ((pipe_p->xyzw[1]*-half_vph) + half_vph);	
		pipe_p->xyzw[2] = ((pipe_p->xyzw[2] + 1.0f) * 0.5f);
		
		omd_ShrinkTCoord(pipe_p->u);
		omd_ShrinkTCoord(pipe_p->v);
		pipe_p->u *= poly->texture->fwidth * pipe_p->inv_w;
		pipe_p->v *= poly->texture->fheight * pipe_p->inv_w;		
	}


	// Rasterize
	
	nvertex = poly->npoints-2;	
	p1 = poly->points;
	p2 = p1+1;
	p3 = p2+1;
		
	if (cbuffer->get_pixel_format()==omobfc_ARGB1555)
	{
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLineZB_TextGen16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZGouraudTextureSegment,
										 OMediaTriangleRasterLineZB_TextGen16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLineZB_TextFlat16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZTextureSegment,
										 OMediaTriangleRasterLineZB_TextFlat16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);			
			
			}
		}
		else 
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLine_TextGen16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleGouraudTextureSegment,
										 OMediaTriangleRasterLine_TextGen16>
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLine_TextFlat16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleTextureSegment,
										 OMediaTriangleRasterLine_TextFlat16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			
			}	
		}
	}
	else
	{			
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLineZB_TextGen32 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZGouraudTextureSegment,
										 OMediaTriangleRasterLineZB_TextGen32>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLineZB_TextFlat32 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZTextureSegment,
										 OMediaTriangleRasterLineZB_TextFlat32>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);			
			
			}
		}
		else
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLine_TextGen32 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleGouraudTextureSegment,
										 OMediaTriangleRasterLine_TextGen32>
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLine_TextFlat32 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleTextureSegment,
										 OMediaTriangleRasterLine_TextFlat32>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			
			}	
		}
	}
}


void OMediaOMTRenderPort::render_polygon_texture16(OMediaPipePolygon *poly)
{
	OMediaPipePoint		*pipe_p,*pipe_end;
	char				*pixels = cb_pixels;
	long				rowbytes = cb_rowbytes;
	OMediaPipePoint		*p1, *p2, *p3;
	long				nvertex;
	omt_BlendFunc		src_blend,dest_blend;
	omt_ZBufferWrite	zb_write;
	omt_ZBufferTest		zb_test;
	omt_ZBufferFunc		zb_func;
	
	src_blend = poly->src_blend;
	dest_blend = poly->dest_blend;
	
	zb_write = (poly->flags&omppf_ZWrite)?omzbwc_Enabled:omzbwc_Disabled;
	zb_test = (poly->flags&omppf_ZTest)?omzbtc_Enabled:omzbtc_Disabled;
	zb_func = poly->zfunc;


	// Transform to screen coordinates

	pipe_p = poly->points;	
	pipe_end = pipe_p + poly->npoints;

	for(; pipe_p!=pipe_end; pipe_p++)
	{
		if (pipe_p->xyzw[0]<-1.0f) pipe_p->xyzw[0] = -1.0f;
		else if (pipe_p->xyzw[0]> 1.0f) pipe_p->xyzw[0] =  1.0f;
		if (pipe_p->xyzw[1]<-1.0f) pipe_p->xyzw[1] = -1.0f;
		else if (pipe_p->xyzw[1]> 1.0f) pipe_p->xyzw[1] =  1.0f;			
		
		pipe_p->xyzw[0] = ((pipe_p->xyzw[0]*half_vpw) + half_vpw);
		pipe_p->xyzw[1] = ((pipe_p->xyzw[1]*-half_vph) + half_vph);	
		pipe_p->xyzw[2] = ((pipe_p->xyzw[2] + 1.0f) * 0.5f);
		
		omd_ShrinkTCoord(pipe_p->u);
		omd_ShrinkTCoord(pipe_p->v);
		pipe_p->u *= poly->texture->fwidth * pipe_p->inv_w;
		pipe_p->v *= poly->texture->fheight * pipe_p->inv_w;		
	}


	// Rasterize
	
	nvertex = poly->npoints-2;	
	p1 = poly->points;
	p2 = p1+1;
	p3 = p2+1;
		
	if (cbuffer->get_pixel_format()==omobfc_ARGB1555)
	{
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLineZB_TextGen16T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZGouraudTextureSegment,
										 OMediaTriangleRasterLineZB_TextGen16T16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLineZB_TextFlat16T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZTextureSegment,
										 OMediaTriangleRasterLineZB_TextFlat16T16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);			
			
			}
		}
		else 
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLine_TextGen16T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleGouraudTextureSegment,
										 OMediaTriangleRasterLine_TextGen16T16>
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLine_TextFlat16T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleTextureSegment,
										 OMediaTriangleRasterLine_TextFlat16T16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			
			}	
		}
	}
	else
	{			
		if (zbuffer && (zb_write==omzbwc_Enabled || zb_test==omzbtc_Enabled))
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLineZB_TextGen32T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZGouraudTextureSegment,
										 OMediaTriangleRasterLineZB_TextGen32T16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLineZB_TextFlat32T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb,
															zbuffer,zmodulo,
															zb_func,zb_test,zb_write);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleZTextureSegment,
										 OMediaTriangleRasterLineZB_TextFlat32T16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);			
			
			}
		}
		else
		{
			if (poly->flags&omppf_Gouraud)
			{
	
				OMediaTriangleRasterLine_TextGen32T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleGouraudTextureSegment,
										 OMediaTriangleRasterLine_TextGen32T16>
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			}
			else 
			{
				OMediaTriangleRasterLine_TextFlat32T16 rasterline(pixels,rowbytes, 
															(unsigned char*)poly->texture->texture,
															poly->texture->xmod_mask,
															poly->texture->ymod_mask,
															poly->texture->rowbytes_shift,
															src_blend,dest_blend,
															p1->a,p1->dr,p1->dg,p1->db,
															p1->sr,p1->sg,p1->sb);
					
				OMediaTriangleRasterizer<OMediaPipePoint,
										 OMediaTriangleTextureSegment,
										 OMediaTriangleRasterLine_TextFlat32T16>									 
										 rasterizer(&rasterline);	

				while(nvertex--) 	rasterizer.draw(p1,p2++,p3++);
			
			}	
		}
	}
}



//----------------------------------
// Pipeline interface

void OMediaOMTRenderPort::flush_pipeline(void)
{
	pipeline->end();
	send_polygons();
	pipeline->begin(0);	
}
	
void OMediaOMTRenderPort::get_view_bounds(OMediaRect &r)
{
	r = current_bounds;
}

void OMediaOMTRenderPort::clear_colorbuffer(OMediaFARGBColor &argb, OMediaRect *area)
{
	OMediaRGBColor	rgb;
	
	rgb.red 	= (unsigned short)(argb.red * float(0xFFFF));
	rgb.green 	= (unsigned short)(argb.green * float(0xFFFF));
	rgb.blue 	= (unsigned short)(argb.blue * float(0xFFFF));

	cbuffer->fill(rgb,area); 
}
		
void OMediaOMTRenderPort::clear_zbuffer(OMediaRect *dest)
{
	OMediaRect		full,bounds;
	char 			*bits;
	short 			w,h;
	long 			rowbytes = zmodulo;

	full.set(0,0,cbuffer->get_width(),cbuffer->get_height());

	if (!dest) bounds = full;
	else bounds = *dest;
	

	if (full.find_intersection(&bounds,&bounds))
	{
		bits = (char*)zbuffer;
		bits += (bounds.top* rowbytes) + (bounds.left<<1);

		w = bounds.get_width()<<1;
		h = bounds.get_height();
		
		while(h--) {OMediaBlitter::raw_fill((unsigned char*)bits, 0xFFFFFFFFUL,w); bits+=w;}
	}
}

void OMediaOMTRenderPort::set_zbuffer_write(omt_ZBufferWrite zb)
{
	zb_write = zb;
}

void OMediaOMTRenderPort::set_zbuffer_test(omt_ZBufferTest zb)
{
	zb_test = zb;
}

void OMediaOMTRenderPort::set_zbuffer_func(omt_ZBufferFunc zb)
{
	zb_func = zb;
}

void OMediaOMTRenderPort::get_zbuffer_info(omt_ZBufferTest &zt, omt_ZBufferWrite &zw, omt_ZBufferFunc &zf)
{
	zt = zb_test;
	zw = zb_write;
	zf = zb_func;
}

OMediaRenderTarget *OMediaOMTRenderPort::get_target(void)
{
	return target;
}

//----------------------------------
// Render target

OMediaOMTRenderTarget::OMediaOMTRenderTarget(OMediaRenderer *renderer) :
						OMediaRenderTarget(renderer,ommeic_OMT,renderer->get_video_engine()->get_supervisor_window())
{
	rebuild_contexts = false;	
}

OMediaOMTRenderTarget::~OMediaOMTRenderTarget()
{
}
	
OMediaRenderPort *OMediaOMTRenderTarget::new_port(void)
{
	return new OMediaOMTRenderPort(this);
}

void OMediaOMTRenderTarget::render(void)
{
	OMediaRenderTarget::render();
	if(!get_renderer()->get_picking_mode()) renderer->get_video_engine()->flip_page();
}

//----------------------------------
// Renderer

OMediaOMTRenderer::OMediaOMTRenderer(OMediaVideoEngine *video,OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer_flags):
					OMediaRenderer(video,def,zbuffer)
{
	zbuffer = zbuffer_flags!=omfzbc_NoZBuffer;
}

OMediaOMTRenderer::~OMediaOMTRenderer() {}


OMediaRenderTarget *OMediaOMTRenderer::new_target(void)
{
	return new OMediaOMTRenderTarget(this);
}


#endif

