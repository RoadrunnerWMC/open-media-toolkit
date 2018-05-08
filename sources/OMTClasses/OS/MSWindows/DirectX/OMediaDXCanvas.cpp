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

#ifdef omd_ENABLE_DIRECT3D

#include "OMediaCanvas.h"
#include "OMediaDXCanvas.h"
#include "OMediaDXRenderer.h"
#include "OMediaEngineImplementation.h"
#include "OMediaError.h"

#include "OMediaMemTools.h"

#include "OMediaMipmapGenerator.h"


#define omd_ARGB32toA16_4444(p) ( (((p)>>16)&(0xF<<12)) | (((p)>>12)&(0xF<<8)) | (((p)>>8)&(0xF<<4)) | (((p)>>4)&0xF) )

enum omt_DXFindTextParam
{
	omdxftpc_Exact,
	omdxftpc_Higher,
	omdxftpc_Lower,
	omdxftpc_Any
};



class OMediaDXFindTextureFormat
{
public:

	// In

	omt_PixelFormat		omtpixelformat;
	short				screen_depth;


	// Out

	DDPIXELFORMAT		dxpixelformat;
	omt_DXFindTextParam	param;

	void find(LPDIRECT3DDEVICE7 d3d_device);

	static HRESULT CALLBACK enum_textures(LPDDPIXELFORMAT lpDDPixFmt,  
														LPVOID          lpContext); 


	static WORD get_bits( DWORD dwMask )
	{
		WORD wBits = 0;

		while( dwMask )
		{
			dwMask = dwMask & ( dwMask - 1 );  
			wBits++;
		}

		return wBits;
	}

};

static void omf_UploadTexture(LPDIRECTDRAWSURFACE7 src, LPDIRECTDRAWSURFACE7 dest)
{
	HDC src_hdc,dest_hdc;
	DDSURFACEDESC2	desc;
	short			nb_a,nb_r,nb_g,nb_b;
	HRESULT			hr;

	desc.dwSize = sizeof(DDSURFACEDESC2);

	if (SUCCEEDED(dest->GetSurfaceDesc(&desc)))
	{
		if(desc.dwFlags&DDPF_ALPHAPIXELS )
		{
			DDSURFACEDESC2 src_desc;

			// Alpha
			nb_a = OMediaDXFindTextureFormat::get_bits(desc.ddpfPixelFormat.dwRGBAlphaBitMask);
			nb_r = OMediaDXFindTextureFormat::get_bits(desc.ddpfPixelFormat.dwRBitMask);
			nb_g = OMediaDXFindTextureFormat::get_bits(desc.ddpfPixelFormat.dwGBitMask);
			nb_b = OMediaDXFindTextureFormat::get_bits(desc.ddpfPixelFormat.dwBBitMask);

			// I convert to 4444, 1555 for the Voodoo 3 that does not support
			// remapping throught the Blt fonction

			if (nb_a==4 && nb_r==4 && nb_g==4 && nb_b==4)
			{
				short	x,y;
				char	*plsrc,*pldest,*ps,*pd;

				desc.dwSize = sizeof(DDSURFACEDESC2);
				src_desc.dwSize = sizeof(DDSURFACEDESC2);

			    hr = src->Lock( NULL, &src_desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL );
				if( FAILED(hr) ) return;
			    hr = dest->Lock( NULL, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL );
				if( FAILED(hr) ) 
				{
					src->Unlock(NULL);
					return;
				}

				plsrc = (char*)src_desc.lpSurface;
				pldest = (char*)desc.lpSurface;
				for(y=0;y<desc.dwHeight;y++)
				{
					ps = plsrc;
					pd = pldest;

					for(x=0;x<desc.dwWidth;x++)
					{
						unsigned long	spix;

						spix = *((unsigned long*)ps);
						
						*((unsigned short*)pd) = omd_ARGB32toA16_4444(spix);												

						ps += 4;
						pd += 2;
					}

					plsrc += src_desc.lPitch;
					pldest += desc.lPitch;
				}

				dest->Unlock(NULL);
				src->Unlock(NULL);
			}
			else if (nb_a==1 && nb_r==5 && nb_g==5 && nb_b==5)
			{
				short x,y;
				char	*plsrc,*pldest,*ps,*pd;

				desc.dwSize = sizeof(DDSURFACEDESC2);
				src_desc.dwSize = sizeof(DDSURFACEDESC2);

			    hr = src->Lock( NULL, &src_desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL );
				if( FAILED(hr) ) return;
			    hr = dest->Lock( NULL, &desc, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL );
				if( FAILED(hr) ) 
				{
					src->Unlock(NULL);
					return;
				}

				plsrc = (char*)src_desc.lpSurface;
				pldest = (char*)desc.lpSurface;
				for(y=0;y<desc.dwHeight;y++)
				{
					ps = plsrc;
					pd = pldest;

					for(x=0;x<desc.dwWidth;x++)
					{
						unsigned long	spix;

						spix = *((unsigned long*)ps);
						
						*((unsigned short*)pd) = omd_ARGB32toA16(spix);												

						ps += 4;
						pd += 2;
					}

					plsrc += src_desc.lPitch;
					pldest += desc.lPitch;
				}

				dest->Unlock(NULL);
				src->Unlock(NULL);
			}
			else
			{
				// Use the DX remapper

				dest->Blt(NULL,src,NULL,DDBLT_WAIT,NULL);
			}
		}
		else
		{
			// No alpha, use the most supported way

			if( SUCCEEDED( src->GetDC( &src_hdc ) ) )
			{
				if( SUCCEEDED( dest->GetDC( &dest_hdc ) ) )
				{
					BitBlt( dest_hdc, 0, 0, desc.dwWidth, desc.dwHeight, src_hdc,
						0, 0, SRCCOPY );

					dest->ReleaseDC( dest_hdc );
				}
				
				src->ReleaseDC( src_hdc );
			}
		}
	}
}


OMediaDXCanvas::OMediaDXCanvas(	OMediaDXRenderTarget *atarget,
							    OMediaCanvas *master,
							    omt_CanvasSlaveKeyFlags key_flags)
								:OMediaCanvasSlave(atarget,master)
{

	OMediaDXFindTextureFormat	find_format;

	find_format.omtpixelformat = master->get_internal_pixel_format();
	find_format.screen_depth = atarget->screen_depth;
	find_format.find(atarget->d3d_device);

	dxpixelformat = find_format.dxpixelformat;


	slave_key_long = 0;
	slave_key_ptr = 0;
	slave_solid = false;	//FIXME...??


	texture = NULL;
	texturegrid = NULL;

	target = atarget;

	create_texture(true, key_flags);

	dirty = false;
}

OMediaDXCanvas::~OMediaDXCanvas()
{
	if (texturegrid)
	{
		for(long i=0;i<n_texture;i++) texturegrid[i]->Release();

		delete [] texturegrid;
	}
	else texture->Release();
}



static unsigned long getnearestpow(unsigned long n)
{
	unsigned long bit;

	for(bit= 32;;)
	{
		bit--;
		
		if (n&(1<<bit)) return (1<<bit);
		if (bit==0) break;
	}
	
	return 0;
}

static unsigned long getnearestpow_shift(unsigned long n)
{
	unsigned long bit;

	for(bit= 32;;)
	{
		bit--;
		
		if (n&(1<<bit)) return bit;
		if (bit==0) break;
	}
	
	return 0;
}


LPDIRECTDRAWSURFACE7 OMediaDXCanvas::create_mipmaptext(LPDIRECTDRAW7 dx_draw, OMediaMipmapGenerator *source)
{
    HRESULT					hr;
    LPDIRECTDRAWSURFACE7	surface  = NULL;
    DDSURFACEDESC2			ddsd2;



	// Let's build the surface now

	OMediaMemTools::zero(&ddsd2,sizeof(DDSURFACEDESC2));



    ddsd2.dwSize = sizeof(DDSURFACEDESC2);
    ddsd2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE |
                    DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_CAPS;



    ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
                           DDSCAPS_SYSTEMMEMORY;

    ddsd2.dwWidth = source->cur_width;
    ddsd2.dwHeight= source->cur_height;
    ddsd2.lPitch  = source->full_width<<2;
    ddsd2.lpSurface = source->pixels;

 

    ddsd2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd2.ddpfPixelFormat.dwFlags= DDPF_RGB | DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwRGBBitCount			= 32;
    ddsd2.ddpfPixelFormat.dwRBitMask			= 0x00FF0000UL;
    ddsd2.ddpfPixelFormat.dwGBitMask			= 0x0000FF00UL;
    ddsd2.ddpfPixelFormat.dwBBitMask			= 0x000000FFUL;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask		= 0xFF000000UL;


    // Create the surface

    hr = dx_draw->CreateSurface(&ddsd2, &surface, NULL);

	if ( FAILED(hr) ) omd_OSEXCEPTION(hr);	

    return surface;
}



LPDIRECTDRAWSURFACE7 OMediaDXCanvas::create_wraptext(LPDIRECTDRAW7 dx_draw, OMediaCanvas *source)
{
    HRESULT					hr;
    LPDIRECTDRAWSURFACE7	surface  = NULL;
    DDSURFACEDESC2			ddsd2;


	source->lock(omlf_Read|omlf_Write);


	// I need to rearrange the pixels because DX does not support the RGBA format

	long			n= source->get_width() * source->get_height();
	unsigned long	*pix = source->get_pixels(),tp,dp;


	while(n--)
	{
		tp = *pix;
		tp = omd_ReverseLong(tp);
		dp =	((tp&0x000000FFUL)<<24L);		// alpha
		dp |= 	((tp&0x0000FF00UL)>>8L);		// red
		dp |=	((tp&0x00FF0000UL)>>8L);		// green
		dp |=	((tp&0xFF000000UL)>>8L);		// blue

		(*pix++) = dp;
	}


	// Let's build the surface now


	OMediaMemTools::zero(&ddsd2,sizeof(DDSURFACEDESC2));

    ddsd2.dwSize = sizeof(DDSURFACEDESC2);
    ddsd2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE |
                    DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_CAPS;


    ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
                           DDSCAPS_SYSTEMMEMORY;

    ddsd2.dwWidth = source->get_width();
    ddsd2.dwHeight= source->get_height();
    ddsd2.lPitch  = ddsd2.dwWidth<<2;
    ddsd2.lpSurface = source->get_pixels();

 
    ddsd2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd2.ddpfPixelFormat.dwFlags= DDPF_RGB | DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwRGBBitCount			= 32;
    ddsd2.ddpfPixelFormat.dwRBitMask			= 0x00FF0000UL;
    ddsd2.ddpfPixelFormat.dwGBitMask			= 0x0000FF00UL;
    ddsd2.ddpfPixelFormat.dwBBitMask			= 0x000000FFUL;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask		= 0xFF000000UL;


    // Create the surface

    hr = dx_draw->CreateSurface(&ddsd2, &surface, NULL);

	source->unlock();

	if ( FAILED(hr) ) omd_OSEXCEPTION(hr);	

    return surface;
}


void OMediaDXCanvas::create_texture(bool new_texture, omt_CanvasSlaveKeyFlags key_flags)
{
	long			w,nw,h,nh;
	OMediaCanvas	*master_canv,temp_canv;
	DDSURFACEDESC2	ddsd;
	HRESULT			hr;

	LPDIRECTDRAWSURFACE7	temp_surface;


	master_canv = (OMediaCanvas*)master;

	nw = w = master_canv->get_width();
	nh = h = master_canv->get_height();

	target->check_texture_size(nw,nh);

	if ((w==nw && h==nh) || !(key_flags&omcskf_SubDivided))		// Exact size or scaled
	{
		n_texture = 1;

		slave_key_long = (w==nw && h==nh)?omcskf_Exact:omcskf_Scaled;



		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
		ddsd.dwWidth         = nw;
		ddsd.dwHeight        = nh;

		if (target->square_texture && ddsd.dwWidth!=ddsd.dwHeight)
		{
			if (ddsd.dwWidth>ddsd.dwHeight) ddsd.dwHeight = ddsd.dwWidth;
			else ddsd.dwWidth = ddsd.dwHeight;
		}


		if (new_texture)
		{
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
                           DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;

			ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;

			if (master_canv->get_filtering_min()>omtfc_Linear)
			{
				ddsd.ddsCaps.dwCaps |=  DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
			}


			if( ((OMediaDXRenderer*)target->get_renderer())->accelerated )
				ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
			else
				ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

			ddsd.ddpfPixelFormat = dxpixelformat;

			hr = target->dx_draw->CreateSurface( &ddsd, &texture, NULL ); 

			if (FAILED(hr)) omd_OSEXCEPTION(hr);
		}

		if (master_canv->get_filtering_min()>omtfc_Linear)
		{
			OMediaMipmapGenerator	*mm_gen;
			LPDIRECTDRAWSURFACE7	mip_level, next_mip_level; 
			DDSCAPS2				ddsCaps; 
			OMediaRect				src_rect,dest_rect;

			ZeroMemory( &ddsCaps, sizeof(DDSCAPS2) );

			src_rect.set(0,0,master_canv->get_width(),master_canv->get_height());
			dest_rect.set(0,0,ddsd.dwWidth,ddsd.dwHeight);

			master_canv->lock(omlf_Read);
			mm_gen = new OMediaMipmapGenerator(master_canv,src_rect,dest_rect);
			mm_gen->flip_alpha();

			mip_level = texture; 
			mip_level->AddRef();
			ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP; 

			for(;;)
			{
				temp_surface = create_mipmaptext(target->dx_draw, mm_gen);

				omf_UploadTexture(temp_surface,mip_level);

				temp_surface->Release();
				mip_level->Release(); 

				if (!mm_gen->generate_next_mipmap()) break;

				hr = mip_level->GetAttachedSurface( &ddsCaps, &next_mip_level); 
				if (FAILED(hr)) omd_OSEXCEPTION(hr);

				mip_level = next_mip_level; 
			}

			delete mm_gen;
			master_canv->unlock();

		}
		else
		{
			temp_canv.create(nw,nh,master_canv);

			temp_surface = create_wraptext(target->dx_draw, &temp_canv);

			omf_UploadTexture(temp_surface,texture);

			temp_surface->Release();
		}
	}
	else // Sub divised
	{
		create_subimage(master_canv,w,h,new_texture,nw,nh);	
	}
}

void OMediaDXCanvas::master_modified(void)
{
	dirty = true;
}

void OMediaDXCanvas::create_subimage(OMediaCanvas *canv, long w, long h, bool new_texture, long nw, long nh)
{
	HRESULT			hr;
	DDSURFACEDESC2	ddsd;
	OMediaCanvas	temp_canv;
	OMediaRect		r,dr;
	LPDIRECTDRAWSURFACE7 temp_surface;

	check_texture_subsize(canv, w, h, nw, nh);

	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
                   DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;

	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;


	if( ((OMediaDXRenderer*)target->get_renderer())->accelerated )
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;


	if (canv->get_filtering_min()>omtfc_Linear)
	{
		ddsd.ddsCaps.dwCaps |=  DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	}


	ddsd.ddpfPixelFormat = dxpixelformat;

	slave_key_long = omcskf_SubDivided;	
	
	subdiv_shift_w = getnearestpow_shift((nw>w)?nw>>1:nw);
	subdiv_shift_h = getnearestpow_shift((nh>h)?nh>>1:nh);

	subdiv_w = float(1<<subdiv_shift_w);
	subdiv_h = float(1<<subdiv_shift_h);

	subdiv_w_mask = (1<<subdiv_shift_w)-1;
	subdiv_h_mask = (1<<subdiv_shift_h)-1;

	n_textw = w>>subdiv_shift_w;	if (w&subdiv_w_mask) n_textw++;
	n_texth = h>>subdiv_shift_h;	if (h&subdiv_h_mask) n_texth++;

	n_texture = n_textw * n_texth;


	if (new_texture) texturegrid = new LPDIRECTDRAWSURFACE7[n_texture];


	LPDIRECTDRAWSURFACE7	*tptr = texturegrid;

	
	long			tw,th;	
	long			last_th,
					last_tw;

	if ((w&subdiv_w_mask)!=0)
	{
		last_tw = getnearestpow(w&subdiv_w_mask);
		if (last_tw!=(w&subdiv_w_mask)) last_tw<<=1L;
		last_u = float(w&subdiv_w_mask)/float(last_tw);
	}
	else last_u = 0.9999f;

	if ((h&subdiv_h_mask)!=0)
	{
		last_th = getnearestpow(h&subdiv_h_mask);
		if (last_th!=(h&subdiv_h_mask)) last_th<<=1L;
		last_v = float(h&subdiv_h_mask)/float(last_th);
	}
	else last_v = 0.9999f;

	canv->lock(omlf_Read);

	for(long ty = 0; ty<n_texth; ty++)
	{
		if ((ty==n_texth-1 && (h&subdiv_h_mask)!=0))
		{
			th = last_th;			
		}
		else 
		{
			th = subdiv_h_mask+1;
		}
	
		for(long tx = 0; tx<n_textw; tx++,tptr++)
		{
			if (tx==n_textw-1 && (w&subdiv_w_mask)!=0)
			{
				tw = last_tw;
			}
			else				
			{
				tw = (subdiv_w_mask+1);
			}

			ddsd.dwWidth         = tw;
			ddsd.dwHeight        = th;

			if (target->square_texture  && ddsd.dwWidth!=ddsd.dwHeight)
			{
				if (ddsd.dwWidth>ddsd.dwHeight) ddsd.dwHeight = ddsd.dwWidth;
				else ddsd.dwWidth = ddsd.dwHeight;
			}

			if (new_texture)
			{

				*tptr = NULL;

				hr = target->dx_draw->CreateSurface( &ddsd, tptr, NULL ); 

				if (FAILED(hr)) omd_OSEXCEPTION(hr);

			}


			if (canv->get_filtering_min()>omtfc_Linear)
			{
				OMediaMipmapGenerator	*mm_gen;
				LPDIRECTDRAWSURFACE7	mip_level, next_mip_level; 
				DDSCAPS2				ddsCaps; 

				ZeroMemory( &ddsCaps, sizeof(DDSCAPS2) );

				r.left = (tx<<subdiv_shift_w);
				r.top  = (ty<<subdiv_shift_h);
				r.right = r.left + tw;
				r.bottom = r.top + th;

				dr.set(0,0,ddsd.dwWidth,ddsd.dwHeight);

				mm_gen = new OMediaMipmapGenerator(canv,r,dr);
				mm_gen->flip_alpha();

				mip_level = *tptr; 
				mip_level->AddRef();

				ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP; 

				for(;;)
				{
					temp_surface = create_mipmaptext(target->dx_draw, mm_gen);

					omf_UploadTexture(temp_surface,mip_level);

					temp_surface->Release();
					mip_level->Release(); 

					if (!mm_gen->generate_next_mipmap()) break;

					hr = mip_level->GetAttachedSurface( &ddsCaps, &next_mip_level); 

					if (FAILED(hr)) omd_OSEXCEPTION(hr);

					mip_level = next_mip_level; 
				}

				delete mm_gen;
			}
			else
			{
				temp_canv.create(tw,th,NULL);

				r.left = (tx<<subdiv_shift_w);
				r.top  = (ty<<subdiv_shift_h);
				r.right = r.left + tw;
				r.bottom = r.top + th;

				temp_canv.draw(canv, &r, 0,0);

				temp_surface = create_wraptext(target->dx_draw, &temp_canv);


				omf_UploadTexture(temp_surface,(*tptr));

				temp_surface->Release();

			}
		}
	}

	canv->unlock();
}

void OMediaDXCanvas::check_texture_subsize(OMediaCanvas *master, long w, long h, long &nw, long &nh)
{
	long	max_size;

	if (master->get_2Dsubdivision_width()!=-1)  
	{
		max_size = master->get_2Dsubdivision_width();
		if (w>max_size) w = max_size;
	
		nw = getnearestpow(w);
		if (w!=nw) nw <<=1;
		
		if (nw>max_size) nw = max_size;
	}

	if (master->get_2Dsubdivision_height()!=-1)  
	{
		max_size = master->get_2Dsubdivision_height();
	
		if (h>max_size) h = max_size;
	
		nh = getnearestpow(h);
		
		if (h!=nh) nh <<=1;

		if (nh>max_size) nh = max_size;	
	}
}

//------------------------------------
// Find texture format


void OMediaDXFindTextureFormat::find(LPDIRECT3DDEVICE7 d3d_device)
{
	switch(omtpixelformat)
	{
		case ompixfc_Best: 
		omtpixelformat = ompixfc_ARGB8888;
		break;

		case ompixfc_ResBest: 
		if (screen_depth<=16) omtpixelformat = ompixfc_RGB565;
		else omtpixelformat = ompixfc_RGB888;
		break;

		case ompixfc_ResBestAlpha: 
		if (screen_depth<=16) omtpixelformat = ompixfc_ARGB4444;
		else omtpixelformat = ompixfc_ARGB8888;
		break;

		case ompixfc_ResBestAlpha1bit: 
		if (screen_depth<=16) omtpixelformat = ompixfc_ARGB1555;
		else omtpixelformat = ompixfc_ARGB8888;
		break;
	}

	dxpixelformat.dwSize = 0;

	param = omdxftpc_Exact;
	d3d_device->EnumTextureFormats(enum_textures, this); 
	if (dxpixelformat.dwSize) return;

	param = omdxftpc_Higher;
	d3d_device->EnumTextureFormats(enum_textures, this); 
	if (dxpixelformat.dwSize) return;

	param = omdxftpc_Lower;
	d3d_device->EnumTextureFormats(enum_textures, this); 
	if (dxpixelformat.dwSize) return;

	param = omdxftpc_Any;
	d3d_device->EnumTextureFormats(enum_textures, this); 
	if (dxpixelformat.dwSize) return;

	// Can't find any texture format?!

	omd_EXCEPTION(omcerr_NoTextureFormatAvailable);
}




HRESULT CALLBACK OMediaDXFindTextureFormat::enum_textures(LPDDPIXELFORMAT pf,  
														LPVOID          lpContext)

{
	OMediaDXFindTextureFormat	*ft = (OMediaDXFindTextureFormat*)lpContext;
	short nb_a,nb_r,nb_g,nb_b;

    if( pf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) ) return DDENUMRET_OK; 
	if (!(pf->dwFlags &DDPF_RGB)) return DDENUMRET_OK;

    if( pf->dwFourCC != 0 ) return DDENUMRET_OK;

	if( pf->dwRGBBitCount <16 )  return DDENUMRET_OK;

    if( pf->dwFlags&DDPF_ALPHAPIXELS )
	{
		// Alpha	

		nb_a = ft->get_bits(pf->dwRGBAlphaBitMask);
	}
	else nb_a = 0;

	nb_r = ft->get_bits(pf->dwRBitMask);
	nb_g = ft->get_bits(pf->dwGBitMask);
	nb_b = ft->get_bits(pf->dwBBitMask);

	if (ft->param==omdxftpc_Exact)
	{
		if ((ft->omtpixelformat&ompixfc_ARGB8888) &&
			(nb_a==8 && nb_r==8 && nb_g==8 && nb_b==8)) {	ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }

		if ((ft->omtpixelformat&ompixfc_ARGB4444) &&
			(nb_a==4 && nb_r==4 && nb_g==4 && nb_b==4)) {	ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }

		if ((ft->omtpixelformat&ompixfc_RGB888) &&
			(nb_a==0 && nb_r==8 && nb_g==8 && nb_b==8)) {	ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }


		if ((ft->omtpixelformat&ompixfc_ARGB1555) &&
			(nb_a==1 && nb_r==5 && nb_g==5 && nb_b==5)) {	ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }	


		if ((ft->omtpixelformat&ompixfc_RGB565) &&
			(nb_a==0 && nb_r==5 && nb_g==6 && nb_b==5)) {	ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }	


		if ((ft->omtpixelformat&ompixfc_RGB555) &&
			(nb_a==0 && nb_r==5 && nb_g==5 && nb_b==5)) {	ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }	

	}
	else if (ft->param==omdxftpc_Higher)
	{
		if ((ft->omtpixelformat&ompixfc_ARGB8888)) return DDENUMRET_CANCEL;
		if (ft->omtpixelformat&ompixfc_ARGB4444)
		{
			if (nb_a==8 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
			if (nb_a==0 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

		}

		if (ft->omtpixelformat&ompixfc_RGB888)
		{
			if (nb_a==8 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
		}

		if (ft->omtpixelformat&ompixfc_ARGB1555)
		{
			if (nb_a==8 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
			if (nb_a==4 && nb_r==4 && nb_g==4 && nb_b==4) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

			if (ft->dxpixelformat.dwSize==0)
			{
				if (nb_a==0 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }
			}
		}

		if (ft->omtpixelformat&ompixfc_RGB565)
		{

			if (nb_a==0 && nb_r==5 && nb_g==5 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
			if (nb_a==0 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

			if (ft->dxpixelformat.dwSize==0)
			{
				if (nb_a==8 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }
				if (nb_a==4 && nb_r==4 && nb_g==4 && nb_b==4) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }
			}
		}

		if (ft->omtpixelformat&ompixfc_RGB555)
		{
			if (nb_a==0 && nb_r==5 && nb_g==6 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
			if (nb_a==0 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

			if (ft->dxpixelformat.dwSize==0)
			{
				if (nb_a==8 && nb_r==8 && nb_g==8 && nb_b==8) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }
				if (nb_a==4 && nb_r==4 && nb_g==4 && nb_b==4) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }
			}
		}

	}
	else if (ft->param==omdxftpc_Lower)
	{
		if ((ft->omtpixelformat&ompixfc_ARGB8888)) 
		{

			if (nb_a==4 && nb_r==4 && nb_g==4 && nb_b==4) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
			if (nb_a==1 && nb_r==5 && nb_g==5 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

			if (ft->dxpixelformat.dwSize==0)
			{
				ft->dxpixelformat = *pf; return DDENUMRET_OK;
			}

			return DDENUMRET_CANCEL;
		}

		if (ft->omtpixelformat&ompixfc_ARGB4444)
		{

			if (nb_a==1 && nb_r==5 && nb_g==5 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
			if (nb_a==0 && nb_r==5 && nb_g==6 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

			if (ft->dxpixelformat.dwSize==0)
			{
				ft->dxpixelformat = *pf; return DDENUMRET_OK;
			}
		}

		if (ft->omtpixelformat&ompixfc_RGB888)
		{
			if (nb_a==0 && nb_r==5 && nb_g==6 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }

			if (nb_a==0 && nb_r==5 && nb_g==5 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

			if (ft->dxpixelformat.dwSize==0)
			{
				ft->dxpixelformat = *pf; return DDENUMRET_OK;
			}

		}

		if (ft->omtpixelformat&ompixfc_ARGB1555)
		{

			if (nb_a==0 && nb_r==5 && nb_g==6 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }

			if (nb_a==0 && nb_r==5 && nb_g==5 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_OK; }

			if (ft->dxpixelformat.dwSize==0)
			{
				ft->dxpixelformat = *pf; return DDENUMRET_OK;
			}
		}

		if (ft->omtpixelformat&ompixfc_RGB565)
		{
			if (nb_a==0 && nb_r==5 && nb_g==5 && nb_b==5) {ft->dxpixelformat = *pf; return DDENUMRET_CANCEL; }
		}

		if (ft->omtpixelformat&ompixfc_RGB555) return DDENUMRET_CANCEL;

	}
	else if (ft->param==omdxftpc_Any)
	{

		ft->dxpixelformat = *pf; return DDENUMRET_CANCEL;

	}

	return DDENUMRET_OK;

}


#endif

