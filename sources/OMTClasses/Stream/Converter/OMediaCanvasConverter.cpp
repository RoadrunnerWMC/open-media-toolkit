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


#include "OMediaCanvasConverter.h"
#include "OMediaError.h"
#include "OMediaEndianSupport.h"
#include "OMediaCompression.h"
#include "OMediaMemTools.h"

OMediaCanvasConverter::OMediaCanvasConverter(OMediaStreamOperators *stream, OMediaCanvas *canvas)
{
	this->stream = stream;
	this->canvas = canvas;
}

OMediaCanvasConverter::~OMediaCanvasConverter()
{
}
	
OMediaCanvasConverter *OMediaCanvasConverter::create_best_converter(OMediaStreamOperators *stream,
																	OMediaCanvas *canvas)
{
	char buffer[4];

	stream->read(buffer,4);
        
	if (buffer[0]=='G' &&
		buffer[1]=='I' &&
		buffer[2]=='F' &&
		buffer[3]=='8')
	{
		// GIF
	
		return new OMediaGIFConverter(stream,canvas);
	}
	else if (buffer[0]==char(0x89) &&
			 buffer[1]=='P' &&
			buffer[2]=='N' &&
			buffer[3]=='G')
	{
		// PNG	
		
		return new OMediaPNGConverter(stream,canvas);	
	}	
	else
	{
		// OMT V1.x
	
		if ((buffer[0]=='O' &&
			buffer[1]=='m' &&
			buffer[2]=='C' &&
			buffer[3]=='v') ||
			(buffer[0]=='O' &&
			 buffer[1]=='m' &&
			 buffer[2]=='M' &&
			 buffer[3]=='C'))
		{
			return new OMediaOMTConverter(stream,canvas);
		}
	}
	
	return NULL;
}


void OMediaCanvasConverter::convert(void) {}



//--------------------------------------------------------------------
// * GIF converter

static unsigned long omt_gif_mask[16] = {1,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,0,0};
static unsigned long omt_gif_powerof2[16] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,0,0};

OMediaGIFConverter::OMediaGIFConverter(OMediaStreamOperators *stream, OMediaCanvas *canvas):
					OMediaCanvasConverter(stream,canvas)
{
}

OMediaGIFConverter::~OMediaGIFConverter()
{
}

void OMediaGIFConverter::convert(void)
{
	bool	locked =false;

	canvas->purge();
	stream->set_reverse_bytes_order(!stream->get_reverse_bytes_order());
	
	try
	{
		bgcolor			= 0;
		has_transparent = false;

		read_global_header();
		read_image_header();

		width = image_rect.get_width();
		height = image_rect.get_height();
		
		canvas->create(width,height);

		if (has_transparent) cmap[transparent_index].alpha = 0;
		
		canvas->fill(cmap[bgcolor]);

		canvas->lock(omlf_Write);
		locked = true;
				
		pixel_ptr = pix_bits = (char*)canvas->get_pixels();
		pix_rowbytes = width<<2;

		line_count = 0;
		y_count = 0;
		interlace_yinc = 8;
		interlace_pass = 0;

		decompress();
		
		canvas->unlock();
	}
	
	catch(OMediaError err)
	{
		if (locked) canvas->unlock();
		stream->set_reverse_bytes_order(!stream->get_reverse_bytes_order());
		throw err;
	}	
}

void OMediaGIFConverter::read_global_header(void)
{
	char			buffer[2];		// 7a or 9a
	unsigned char	packed;
	short			i;

	stream->read(buffer,2);
	
	*stream>>width;
	*stream>>height;
	
	*stream>>packed;

	has_map = (packed & 0x80)!=0;

	cres	= (packed & 0x70) >> 4;
	pixbits =  packed & 0x07;

	*stream>>bgcolor;
	*stream>>ratio;	

  	ncolors=omt_gif_powerof2[(1+pixbits)];

  	if (has_map)
  	{
   		for(i=0;i<(short)ncolors; i++)
   		{
   			unsigned char r,g,b;
   			
   			*stream>>r;
   			*stream>>g;
   			*stream>>b;

			cmap[i].alpha	= 0xFFFF;
   			cmap[i].red 	= (r<<8)|r; 
   			cmap[i].green 	= (g<<8)|g;
   			cmap[i].blue 	= (b<<8)|b;
   			   			

		}		
	}
	
	scan_special_blocks();
}

void OMediaGIFConverter::scan_special_blocks(void)
{
	unsigned char	c;
	unsigned char 	packed,color_index;
	short			delaytime;

	for(;;)
	{
		*stream>>c;
		if (c==',') break;
		
		if (c==0x21)
		{
			*stream>>c;
		
			switch (c)
			{
				case 0xF9:				
				// Graphic control extension
			
				*stream>>c;
				*stream>>packed;
				*stream>>delaytime;
				*stream>>color_index;
				*stream>>c;
				
				if (packed&1)
				{
					has_transparent = true;
					transparent_index = color_index;
				}
				else has_transparent = false;
				
				break;
				
				case 0xFE:
				// Comments
				
				*stream>>c;
				
				do
				{
					stream->setposition(c,omcfr_Current);
					*stream>>c;
					
				}while(c);				
				break;
				
				default:
				*stream>>c;
				stream->setposition(c+1,omcfr_Current);		
				break;
			}
		}
	}
}

void OMediaGIFConverter::read_image_header(void)
{
	short i;
	unsigned short ncolors;
	unsigned char	c;
	bool			has_map;
	OMediaShortRect	r;
	
	
	*stream>>r.left;
	*stream>>r.top;
	*stream>>r.right;
	*stream>>r.bottom;

	r.right += r.left;
	r.bottom += r.top;
	
	image_rect = r;
	
	*stream>>c;
    
    interlaced      = (c & 0x40)!=0;
   	pixbits 		= c & 0x07;
	has_map 		= (c & 0x80)!=0;

	if (has_map)
	{
    	ncolors = omt_gif_powerof2[(1+pixbits)];

		for(i=0;i<ncolors;i++)
		{
			unsigned char	r,g,b;
		
  			*stream>>r;
   			*stream>>g;
   			*stream>>b;

			cmap[i].alpha	= 0xFFFF;
   			cmap[i].red 	= (r<<8)|r; 
   			cmap[i].green 	= (g<<8)|g;
   			cmap[i].blue 	= (b<<8)|b;
   		}
  	}
}

void OMediaGIFConverter::clear_table(void)
{
	long i;
  	for(i=0;i<omd_MAXVAL;i++) table[i].valid=0;
  	init_table();
}

void OMediaGIFConverter::init_table()       
{
	long maxi,i;

	maxi=omt_gif_powerof2[root_code_size];
  	for(i=0; i<maxi; i++)
  	{
  		table[i].data=i;   
  		table[i].first=i;
  		table[i].valid=1;  
  		table[i].last = -1;
  	}
  	
  	CLEAR=maxi; 
  	EOI=maxi+1; 
  	nextab=maxi+2;
  	INCSIZE = (2*maxi)-1;
  	code_size=root_code_size+1;
}


void OMediaGIFConverter::decompress(void)
{
	unsigned long 		code,old;
	unsigned char			c;
	
	bits=0;
	num_bits=0;
	gif_block_size=0;
	
	*stream>>c;
	root_code_size = c&0xff;
	
	clear_table();

  	code=get_code();

	if (code==CLEAR) 
	{
		clear_table();	
   		code=get_code();
	}
	
	process_data(code);
	
	old=code;
  	code=get_code();
  	
  	do
  	{
   		if (table[code].valid==1)
   		{
     		process_data(code);
     		get_next_entry();
			add_to_table(old,code,nextab);
     		old=code;
	   	}
	   	else
   		{
    		add_to_table(old,old,code); 
     		process_data(code);
		    old=code;
  	 	}
   
   		code=get_code();
   
   		if (code==CLEAR)
   		{ 
	   		clear_table();
	    	code=get_code();
	    	process_data(code);
	    	old=code;
	    	code=get_code();
   		}
  	}
  	while(code!=EOI);
}

unsigned long OMediaGIFConverter::get_code(void)
{
	unsigned long code;
  	unsigned char c;
 
	while((long)num_bits < (long)code_size)
  	{
   		if (gif_block_size==0) 
   		{
    		*stream>>c;
   			gif_block_size=(unsigned long)(c);
   		}   

		*stream>>c;
		gif_block_size--;
		
	    bits |= ( ((unsigned long)(c) & 0xff) << num_bits );
	    num_bits+=8;
    }
   
	code = bits & omt_gif_mask[code_size];
  	bits >>= code_size;
  	num_bits -= code_size; 

  	if (code>omd_MAXVAL)
  	{ 
   		code=EOI;
  	}

  	if (code==INCSIZE)
  	{
   		if (code_size<12)
   		{
    		code_size++; INCSIZE=(INCSIZE*2)+1;
   		}
  	}

  	return(code);
}

void OMediaGIFConverter::process_data(long index)
{
	long i,j;
	i=0;
  	do
  	{
   		gif_buff[i]=table[index].data; 
   		i++;
   		index=table[index].last;
   		if (i>omd_MAXVAL) omd_EXCEPTION(omcerr_BadFormat);
	} 
	while(index>=0);

	i--;
	
	
  	for(j=i;j>=0;j--)
  	{
  		OMediaARGBColor	*argb = &cmap[gif_buff[j]];
  		
  		pixel_ptr[omd_CGUN_R] = argb->red>>8;
  		pixel_ptr[omd_CGUN_G] = argb->green>>8;
  		pixel_ptr[omd_CGUN_B] = argb->blue>>8;
  		pixel_ptr[omd_CGUN_A] = argb->alpha>>8;
  		pixel_ptr+=4;
  		
		line_count++;
		
		if (line_count==width)
		{
			if (interlaced)
			{
				line_count=0;				
				y_count += interlace_yinc;
				
				while (y_count>=height)
				{
					interlace_pass++;
				
					switch(interlace_pass)
					{
						case 1:
						interlace_yinc = 8;
						y_count = 4;
						break;
						
						case 2:
						interlace_yinc = 4;
						y_count = 2;
						break;
						
						case 3:
						interlace_yinc = 2;
						y_count = 1;
						break;
						
						default:
						y_count = 0;
						break;		
					}
					
					if (interlace_pass>3) break;
				}
				
				pixel_ptr = pix_bits + (y_count*pix_rowbytes);
			}
			else
			{	
				line_count=0;
			}
		}	
  	}
}

void OMediaGIFConverter::get_next_entry(void)
{ 
	while(  (table[nextab].valid==1) &&(nextab<omd_MAXVAL) ) nextab++;

	if (nextab>=omd_MAXVAL)   omd_EXCEPTION(omcerr_BadFormat);
	
	if (nextab==INCSIZE)
  	{
    	code_size++; INCSIZE=(INCSIZE*2)+1;
    	if (code_size>=12) code_size=12;
	}
}

void OMediaGIFConverter::add_to_table(unsigned long body, unsigned long next, unsigned long index)
{
	if (index>omd_MAXVAL) omd_EXCEPTION(omcerr_BadFormat);

	table[index].valid=1;
	table[index].data=table[next].first;
	table[index].first=table[body].first;
	table[index].last=body;
}


//--------------------------------------------------------------------
// * OMT V1.x format standard reader

const unsigned long omc_GfxWorldType = 'OmGW';

OMediaOMTConverter::OMediaOMTConverter(OMediaStreamOperators *stream, OMediaCanvas *canvas):
					OMediaCanvasConverter(stream,canvas)
{
}

OMediaOMTConverter::~OMediaOMTConverter()
{
}

void OMediaOMTConverter::read_palette(void)
{
	const unsigned long omc_OldPalType = 'OmPa';
	const unsigned long omc_PalType = 'OPa2';
	long l;

	unsigned long hdr, size;
	bool			old_format;
	string			desc;
		
	*stream>>hdr>>desc>>size;

	switch(hdr)
	{
		case omc_OldPalType:
		old_format = true;
		break;
		
		case omc_PalType:
		old_format = false;
		break;
		
		default:
		omd_EXCEPTION(omcerr_BadFormat);
	}

	stream->read(palette,size*2*3);
	
	unsigned short *p = palette[0];
	for(l=0; l<(long)size; l++)
	{
		*p = omd_IfLittleEndianReverseShort(*p);		p++;
		*p = omd_IfLittleEndianReverseShort(*p);		p++;
		*p = omd_IfLittleEndianReverseShort(*p);		p++;
	}
	
	if (!old_format)
	{
		char *dum = new char[16*16*16];
		stream->read(dum,16*16*16);
		delete [] dum;
	}

	// Prepare RGBA palette

	for(l=0; l<(long)size; l++)
	{
		rgba_palette[l][0] = palette[l][0];
		rgba_palette[l][1] = palette[l][1];
		rgba_palette[l][2] = palette[l][2];
		rgba_palette[l][3] = 0xFF;
	}
	
	if (size<=2)
	{
		rgba_palette[0][0] = 0xFF;
		rgba_palette[0][1] = 0xFF;
		rgba_palette[0][2] = 0xFF;
		rgba_palette[0][3] = 0xFF;
	
		rgba_palette[1][0] = 0;
		rgba_palette[1][1] = 0;
		rgba_palette[1][2] = 0;
		rgba_palette[1][3] = 0xFF;	
	}
}

void OMediaOMTConverter::convert(void)
{
	unsigned long attr;

	*stream>>attr;

	char *bits,*dest, *buffer;
	short width, height, depth, nbytes,nbyter, modulo;
	char compression = 0, version = 0;
	long nloop;
	unsigned long hdrtype,bufsize;
	bool dotransparent,dopalette;
	OMediaRGBColor transp_rgb;
	unsigned long	transp_packed;
	omt_RLEPackMode	pmode;

	// Read type
	*stream>>hdrtype;
	if (hdrtype != omc_GfxWorldType)  omd_EXCEPTION(omcerr_BadFormat);


	// Read info
	*stream>>width;
	*stream>>height;
	*stream>>depth;	
	*stream>>compression;
	*stream>>version;
	*stream>>nbytes;
	

	switch(compression)
	{
		case 0:
		pmode = omrlec_8Bits;
		break;

		case 1:
		pmode = omrlec_16Bits;
		break;

		case 2:
		pmode = omrlec_32Bits;
		break;
		
		default:
		omd_EXCEPTION(omcerr_BadFormat); // Should be RLE packed
	}

	// Transparent color
	*stream>>dotransparent;

	if (version==0)
	{
		*stream>>transp_rgb.red;
		*stream>>transp_rgb.green;
		*stream>>transp_rgb.blue;

	}
	else if (version>0 && dotransparent) // Version 1 increases transparent color precision
	{									 // by saving packed color guns instead of 48bits rgb.
		*stream>>transp_packed;
	}

	*stream>>dopalette;
	if (dopalette) read_palette();

	if (dotransparent)
	{
		if (version==0) 
		{
			switch(depth)
			{
				case 1:
				case 4:
				case 8:
				{
					for(short i=0;i<256;i++)
					{
						if (palette[i][0]==transp_rgb.red &&
							palette[i][1]==transp_rgb.green &&
							palette[i][2]==transp_rgb.blue)
						{
							transp_packed = i;
							break;
						}					
					}				
				}
				break;
			
				case 16:
				transp_packed = transp_rgb.getrgb16bits();
				break;
				
				case 32:
				transp_packed = transp_rgb.getrgb32bits();
				break;
				
				default:
				transp_packed = 0xFFFFFFFF;
				break;
			}
		}
	}
	else transp_packed = 0xFFFFFFFF;
	

	// Read data
	*stream>>bufsize;

	buffer = new char[bufsize];
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);

	stream->read(buffer,bufsize);

	// Start unpack

	char	*pix_buffer;
	
	bits = pix_buffer = new char[nbytes * height];

	modulo = 0;

	// Unpack data
	dest = buffer;

	nloop = height;
	while(nloop--)
	{
		nbyter = *((short*)dest);
		nbyter = omd_IfLittleEndianReverseShort(nbyter);

		dest += 2;
		if (!rle_unpack(&dest, &bits, nbyter, nbytes,pmode)) omd_EXCEPTION(omcerr_BadFormat);

		#ifdef omd_LITTLE_ENDIAN
		bool	little_endian = true;
		#else
		bool	little_endian = false;
		#endif

		if (little_endian && depth==32)
		{
			unsigned long *lrgb_p,lrgb;
			short lrgb_n;
			
			lrgb_n = nbytes>>2;
			lrgb_p = (unsigned long *) (((char*)bits)-nbytes);
		
			while(lrgb_n--)
			{
				lrgb = *lrgb_p;
				*lrgb_p = omd_ReverseLong(lrgb);
				lrgb_p++;
			}			
		}
		else if (depth==16 && little_endian)
		{
			unsigned short *wrgb_p,wrgb;
			short wrgb_n;
			
			wrgb_n = nbytes>>1;
			wrgb_p = (unsigned short *) (((char*)bits)-nbytes);
		
			while(wrgb_n--)
			{
				wrgb = *wrgb_p;

				if (little_endian) wrgb = omd_ReverseShort(wrgb);

				*wrgb_p = wrgb;
				wrgb_p++;
			}			
		}

		
		bits += modulo;
	}

	delete [] buffer;

	create_canvas(width,height,depth,nbytes,pix_buffer,transp_packed);
	if (!dotransparent) canvas->fill_alpha(0xFF);
	delete [] pix_buffer;
}

void OMediaOMTConverter::create_canvas(	short width, short height, 
										short depth, long nbytes, 
										char 			*pix_buffer,
										unsigned long	packed_transp)
{
	canvas->create(width,height);
	canvas->lock(omlf_Write);
	unsigned char *dest_pix = (unsigned char *) canvas->get_pixels();

	short x,y,lines;
	long modulo;

	if (depth<8)
	{
		unsigned long	p=0;
	
		for(y = 0; y<height; y++)
		for(x = 0; x<width; x++)
		{
			switch(depth)
			{
				case 1:
				p = *(((unsigned char*)pix_buffer) + (y*nbytes) + (x>>3));
				p = (p>>(7-(x&7)))&1;		
				break;
		
				case 4:
				p = *(((unsigned char*)pix_buffer) + (y*nbytes) + (x>>1));
				p = ((x&1)?(p>>4):p)&15;		
				break;
			}
			
			dest_pix[omd_CGUN_R] = rgba_palette[p][0];
			dest_pix[omd_CGUN_G] = rgba_palette[p][1];
			dest_pix[omd_CGUN_B] = rgba_palette[p][2];
			dest_pix[omd_CGUN_A] = p==packed_transp?0:0xFF;				
			dest_pix += 4;
		}
	
	}
	else if (depth==8)
	{
		char	*p = pix_buffer;

		modulo = nbytes-width;
		lines = height;
		
		while(lines--)
		{
			for(x=0;x<width;x++,p++)
			{
				dest_pix[omd_CGUN_R] = rgba_palette[*p][0];
				dest_pix[omd_CGUN_G] = rgba_palette[*p][1];
				dest_pix[omd_CGUN_B] = rgba_palette[*p][2];
				dest_pix[omd_CGUN_A] = (unsigned long)(*p)==packed_transp?0:0xFF;				
				dest_pix += 4;			
			}
			
			p += modulo;
		}		
	}
	else if (depth==16)
	{
		unsigned short	*p = (unsigned short*) pix_buffer;	

		modulo = nbytes-(width<<1);
		lines = height;
		
		while(lines--)
		{
			for(x=0;x<width;x++,p++)
			{
				dest_pix[omd_CGUN_R] = (((*p)>>10)&0x1F)<<3;
				dest_pix[omd_CGUN_G] = (((*p)>>5)&0x1F)<<3;
				dest_pix[omd_CGUN_B] = (((*p))&0x1F)<<3;
				dest_pix[omd_CGUN_A] = (*p)==packed_transp?0:0xFF;				
				dest_pix += 4;
			}
			
			p = (unsigned short *) (((unsigned char*)p)+modulo);
		}	
	}
	else if (depth==32)
	{
		unsigned long	*p = (unsigned long*) pix_buffer;	

		modulo = nbytes-(width<<2);
		lines = height;
		
		while(lines--)
		{
			for(x=0;x<width;x++,p++)
			{
				dest_pix[omd_CGUN_R] = ((*p)>>16L)&0xFFL;
				dest_pix[omd_CGUN_G] = ((*p)>>8L)&0xFFL;
				dest_pix[omd_CGUN_B] = (*p)&0xFFL;
				dest_pix[omd_CGUN_A] = (*p)==packed_transp?0:0xFF;				
				
				dest_pix += 4;
			}
			
			p = (unsigned long *) (((unsigned char*)p)+modulo);
		}	
	}

	canvas->unlock();
}


//--------------------------------------------------------------------
// * PNG Converter

OMediaPNGConverter::OMediaPNGConverter(OMediaStreamOperators *stream, OMediaCanvas *canvas):
					OMediaCanvasConverter(stream,canvas)
{
	image_data = NULL;

	rgba_palette[0][omd_CGUN_R] = 0xFF;
	rgba_palette[0][omd_CGUN_G] = 0xFF;
	rgba_palette[0][omd_CGUN_B] = 0xFF;
	rgba_palette[0][omd_CGUN_A] = 0xFF;

	rgba_palette[1][omd_CGUN_R] = 0;
	rgba_palette[1][omd_CGUN_G] = 0;
	rgba_palette[1][omd_CGUN_B] = 0;
	rgba_palette[1][omd_CGUN_A] = 0;
}

OMediaPNGConverter::~OMediaPNGConverter()
{
	delete [] image_data;
}

void OMediaPNGConverter::convert(void)
{
	unsigned char	sign_part2[4];
	unsigned long	chunk_size,chunk_type,crc;
	bool			eofile = false;

	canvas->purge();

	stream->read(sign_part2,4);
	if (!(	sign_part2[0] == 0x0D &&
			sign_part2[1] == 0x0A &&
			sign_part2[2] == 0x1A &&
			sign_part2[3] == 0x0A)) omd_EXCEPTION(omcerr_BadFormat);

	do
	{
		// Read chunk

		*stream>>chunk_size;
		*stream>>chunk_type;
		switch(chunk_type)
		{
			case 'IHDR':
			read_header(chunk_size);
			break;	

			case 'PLTE':
			read_palette(chunk_size);
			break;

			case 'IDAT':
			read_data(chunk_size);
			break;			
			
			case 'IEND':
			eofile = true;
			break;
		
			default:
			stream->skip(chunk_size);
			break;
		}
		
		*stream>>crc;
	}
	while(!eofile);

	create_canvas();
}

void OMediaPNGConverter::read_header(long chunk_size)
{
	*stream>>width;
	*stream>>height;
	*stream>>depth;
	*stream>>color_type;
	*stream>>compression;
	*stream>>filter;
	*stream>>interlace;

	if (depth<8) color_type = 3;

	if (chunk_size>13) stream->skip(chunk_size-13);
}

void OMediaPNGConverter::read_palette(long chunk_size)
{
	long	nentries = chunk_size/3;
	
	if (nentries>256) nentries=256;
	for(short i=0;i<nentries;i++)
	{
		*stream>>rgba_palette[i][0];
		*stream>>rgba_palette[i][1];
		*stream>>rgba_palette[i][2];
		rgba_palette[i][3] = 0xFF;			
	}
	
	if (chunk_size>256*3) stream->skip(chunk_size-(256*3));	
}

void OMediaPNGConverter::read_data(long chunk_size)
{
	long			pixsize = 0;
	unsigned char	*comp_image_data;

	comp_image_data = new unsigned char[chunk_size];
	stream->read(comp_image_data,chunk_size);

	switch(color_type)
	{
		case 0:
		if (depth<=8) pixsize = 1;
		else pixsize = 2;
		break;

		case 2:
		if (depth<=8) pixsize = 3;
		else pixsize = 6;
		break;	

		case 3:
		pixsize = 1;
		break;	

		case 4:
		if (depth<=8) pixsize = 2;
		else pixsize = 4;
		break;	

		case 6:
		if (depth<=8) pixsize = 4;
		else pixsize = 8;
		break;

		default:
		delete [] comp_image_data;
		return;
	}

	long	decomp_size = (width+1)*pixsize*height;

	image_data = new unsigned char[(width+1)*pixsize*height];

	OMediaCompression::uncompress(comp_image_data, chunk_size, image_data, decomp_size);
	
	delete [] comp_image_data;
}

void OMediaPNGConverter::create_canvas(void)
{
	unsigned char	*pix;
	unsigned char	*image_data = OMediaPNGConverter::image_data;
	unsigned char	filter;

	if (!image_data) return;

	canvas->create(width,height);
	canvas->lock(omlf_Write);
		
	pix = (unsigned char*)canvas->get_pixels();
	
	if (color_type==3)	// indexed
	{
		unsigned long	p=0,nbytes=0;
		
		switch(depth)
		{
			case 1:
			nbytes = width>>3; if (width&7) nbytes++;
			break;
			
			case 2:
			nbytes = width>>2; if (width&3) nbytes++;
			break;

			case 4:
			nbytes = width>>1; if (width&1) nbytes++;
			break;
			
			case 8:
			nbytes = width;
			break;
		}
		
		nbytes+=1;
		filter = *(image_data++);
	
		for(long y=0; y<height;y++)
		for(long x=0; x<width;x++,pix+=4)
		{		
			switch(depth)
			{
				case 1:
				p = *(((unsigned char*)image_data) + (y*nbytes) + (x>>3));
				p = (p>>(7-(x&7)))&1;
				break;

				case 2:
				p = *(((unsigned char*)image_data) + (y*nbytes) + (x>>2));
				switch(x&3)
				{
					case 0: p>>=6;
					case 1: p>>=4;
					case 2: p>>=2;
				}
				p&=3;
				break;
		
				case 4:
				p = *(((unsigned char*)image_data) + (y*nbytes) + (x>>1));
				p = ((x&1)?(p>>4):p)&15;
				break;
				
				case 8:
				p = *(((unsigned char*)image_data) + (y*nbytes) + x);
				break;
			}
	
			pix[omd_CGUN_R] = rgba_palette[p][0];
			pix[omd_CGUN_G] = rgba_palette[p][1];
			pix[omd_CGUN_B] = rgba_palette[p][2];
			pix[omd_CGUN_A] = 0xFF;
		}		
	}
	else // True color
	{
		unsigned char *src = image_data;
		unsigned char r,g,b,a;
		unsigned long rowbytes = 0;
		unsigned long pixdepth;
		
		switch(color_type)
		{
			case 0:
			pixdepth = ((depth==8)?1:2);
			break;
			
			case 2:
			pixdepth = ((depth==8)?1:2) * 3;
			break;
			
			case 4:
			pixdepth = ((depth==8)?1:2) * 2;
			break;
		
			case 6:
			pixdepth = ((depth==8)?1:2) * 4;
			break;
		}
		
		rowbytes = pixdepth * width;
		
		unsigned char *prevRow = new unsigned char[rowbytes];
		
		OMediaMemTools::zero(prevRow,rowbytes);
		
		for(long y=0; y<height;y++)
		{
			filter = *(src++);
			
			filter_row(filter, rowbytes, pixdepth,src, prevRow);		

			OMediaMemTools::copy(src,prevRow,rowbytes);
		
			for(long x=0; x<width;x++,pix+=4)
			{		
				switch(color_type)
				{
					case 0:
					r = g = b = *src;
					a = 0xFF;				
					src += (depth==8)?1:2;
					break;
					
					case 2:
					r = *src;	src += (depth==8)?1:2;
					g = *src;	src += (depth==8)?1:2;
					b = *src;	src += (depth==8)?1:2;
					a = 0xFF;
					break;
					
					case 4:
					r = g = b = *src;	src += (depth==8)?1:2;
					a = *src;			src += (depth==8)?1:2;				
					break;
				
					case 6:
					r = *src;	src += (depth==8)?1:2;
					g = *src;	src += (depth==8)?1:2;
					b = *src;	src += (depth==8)?1:2;
					a = *src;	src += (depth==8)?1:2;
					break;
				}
	
				pix[omd_CGUN_R] = r;
				pix[omd_CGUN_G] = g;
				pix[omd_CGUN_B] = b;
				pix[omd_CGUN_A] = a;
			}
		}
		
		delete [] prevRow;
		
	}	

	canvas->unlock();
}

void OMediaPNGConverter::filter_row(int filter, unsigned long rowbytes, unsigned long bpp, 
									unsigned char *row,
									unsigned char *prev_row)
{
	const int PNG_FILTER_VALUE_NONE = 0;
	const int PNG_FILTER_VALUE_SUB = 1;
	const int PNG_FILTER_VALUE_UP = 2;
	const int PNG_FILTER_VALUE_AVG = 3;
	const int PNG_FILTER_VALUE_PAETH = 4;

   switch (filter)
   {
      case PNG_FILTER_VALUE_NONE:
        break;
         
      case PNG_FILTER_VALUE_SUB:
      {
         unsigned long i;
         unsigned long istop = rowbytes;
         unsigned char *rp = row + bpp;
         unsigned char *lp = row;

         for (i = bpp; i < istop; i++)
         {
            *rp = (unsigned char)(((int)(*rp) + (int)(*lp++)) & 0xff);
            rp++;
         }
         break;
      }
      case PNG_FILTER_VALUE_UP:
      {
         unsigned long i;
         unsigned long istop = rowbytes;
         unsigned char * rp = row;
         unsigned char * pp = prev_row;

         for (i = 0; i < istop; i++)
         {
            *rp = (unsigned char)(((int)(*rp) + (int)(*pp++)) & 0xff);
            rp++;
         }
         break;
      }
      
      case PNG_FILTER_VALUE_AVG:
      {
         unsigned long  i;
         unsigned char *  rp = row;
         unsigned char *  pp = prev_row;
         unsigned char *  lp = row;
         unsigned long  istop = rowbytes - bpp;

         for (i = 0; i < bpp; i++)
         {
            *rp = (unsigned char)(((int)(*rp) +
               ((int)(*pp++) / 2 )) & 0xff);
            rp++;
         }

         for (i = 0; i < istop; i++)
         {
            *rp = (unsigned char)(((int)(*rp) +
               (int)(*pp++ + *lp++) / 2 ) & 0xff);
            rp++;
         }
         break;
      }
      
      case PNG_FILTER_VALUE_PAETH:
      {
         unsigned long   i;
         unsigned char * rp = row;
         unsigned char * pp = prev_row;
         unsigned char * lp = row;
         unsigned char * cp = prev_row;
         unsigned long istop=rowbytes - bpp;

         for (i = 0; i < bpp; i++)
         {
            *rp = (unsigned char)(((int)(*rp) + (int)(*pp++)) & 0xff);
            rp++;
         }

         for (i = 0; i < istop; i++)   /* use leftover rp,pp */
         {
            int a, b, c, pa, pb, pc, p;

            a = *lp++;
            b = *pp++;
            c = *cp++;

            p = b - c;
            pc = a - c;

            pa = p < 0 ? -p : p;
            pb = pc < 0 ? -pc : pc;
            pc = (p + pc) < 0 ? -(p + pc) : p + pc;

            p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;

            *rp = (unsigned char)(((int)(*rp) + p) & 0xff);
            rp++;
         }
         break;
      }
      
      default:
      *row=0;
      break;
   }
}

