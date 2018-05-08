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
 
#pragma once
#ifndef OMEDIA_CanvasConverter_H
#define OMEDIA_CanvasConverter_H

#include "OMediaStreamOperators.h"
#include "OMediaCanvas.h"
#include "OMediaRGBColor.h"
#include "OMediaRLEPacker.h"

// * Abstract converter

class OMediaCanvasConverter
{
	public:

	omtshared OMediaCanvasConverter(OMediaStreamOperators *stream, OMediaCanvas *shape);
	omtshared virtual ~OMediaCanvasConverter();	

	static OMediaCanvasConverter *create_best_converter(OMediaStreamOperators *stream, OMediaCanvas *canvas);

	omtshared virtual void convert(void);

	protected:
	
	OMediaStreamOperators *stream;
	OMediaCanvas 		  *canvas;
};


// * GIF converter

class OMediaGIFTableEntry
{
	public:

	unsigned char valid,data,first,res;
	long		  last;

};

class OMediaGIFConverter : public OMediaCanvasConverter
{
	public:

	omtshared OMediaGIFConverter(OMediaStreamOperators *stream, OMediaCanvas *canvas);
	omtshared virtual ~OMediaGIFConverter();

	omtshared virtual void convert(void);
	
	void read_global_header(void);
	void read_image_header(void);

	void clear_table(void);
	void get_next_entry(void);
	void add_to_table(unsigned long body, unsigned long next, unsigned long index);
	void process_data(long index);
	unsigned long get_code(void);
	void decompress(void);
	void init_table(void);
	void scan_special_blocks(void);
	
	// Header
	
	short			width,height;
	bool			has_map;
	unsigned char	cres,pixbits,ratio;
	
	unsigned char	bgcolor;
	unsigned long	ncolors;

	OMediaARGBColor	cmap[256];

	#define omd_MAXVAL  4100 
	#define omd_MAXVALP 4200


	OMediaShortRect		image_rect;
	OMediaGIFTableEntry	table[omd_MAXVALP];
 	unsigned char		gif_buff[omd_MAXVALP];

	bool			interlaced;
	short			y_count;
	short			interlace_yinc,interlace_pass;
	
	bool			has_transparent;
	unsigned short	transparent_index;

 	long			num_bits,bits;
	unsigned long	gif_block_size;
	unsigned long	root_code_size,code_size,CLEAR,EOI,INCSIZE;
 	unsigned long	nextab;

	char			*pixel_ptr,*pix_bits;
	long			pix_rowbytes;
	long			line_count;
};

// * Standard OMT V1.x reader

class OMediaOMTConverter : public OMediaCanvasConverter,
						   public OMediaRLEPacker
{
	public:

	omtshared OMediaOMTConverter(OMediaStreamOperators *stream, OMediaCanvas *canvas);
	omtshared virtual ~OMediaOMTConverter();

	unsigned short	palette[256][3];
	unsigned char	rgba_palette[256][4];
	
	
	omtshared virtual void read_palette(void);

	omtshared virtual void convert(void);

	omtshared virtual void create_canvas(	short width, 	short height,
											short depth, 	long nbytes,
											char 			*pix_buffer,
											unsigned long	packed_transp);
};


// * PNG converter

class OMediaPNGConverter : public OMediaCanvasConverter
{
	public:

	omtshared OMediaPNGConverter(OMediaStreamOperators *stream, OMediaCanvas *canvas);
	omtshared virtual ~OMediaPNGConverter();

	unsigned char	rgba_palette[256][4];

	unsigned char	depth,color_type,compression,filter,interlace;
	long			width,height;

	unsigned char	*image_data;

	void read_header(long chunk_size);
	void read_palette(long chunk_size);
	void read_data(long chunk_size);	
	
	void create_canvas(void);
	
	omtshared virtual void convert(void);


	void filter_row(int filter, unsigned long rowbytes, unsigned long pixel_depth, 
									unsigned char *row,
									unsigned char *prev_row);

};




#endif
