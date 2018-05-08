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
 
#include "OMediaCanvasFont.h"
#include "OMediaCanvas.h"
#include "OMediaMemTools.h"
#include "OMediaError.h"

OMediaCanvasFont::OMediaCanvasFont()
{
	font = NULL;
	fwidth = 0;
	fheight = 0;
	vertical_delta = 0;
	space_size = 0;
	proportional = false;
	char_space = 2;
	for(short i=0; i<omd_FONT_TRANSLATE_TABLE_SIZE; i++) trans_tab[i] = i;
	
	prop_tab = NULL;

	set_default_inverse_table();
}

OMediaCanvasFont::~OMediaCanvasFont()
{
	db_update();

	if (font) font->db_unlock();

	delete [] prop_tab;
	prop_tab = NULL;
}

void OMediaCanvasFont::get_font_info(long &w, long &h, long &vm)
{
	w = fwidth;
	h = fheight;
	vm = vertical_delta;
}

void OMediaCanvasFont::set_font_canvas(OMediaCanvas *f)
{
	if (font) font->db_unlock();
	font = f;
	if (font) font->db_lock();
}	

void OMediaCanvasFont::set_font_canvas(OMediaCanvas *f, long w, long h, long vd)
{
	if (font) font->db_unlock();
	font = f;
	if (font) font->db_lock();

	fwidth = space_size = w;
	fheight = h;
	vertical_delta = (vd==-1)?fheight:vd;
}
	
void OMediaCanvasFont::set_translation_tab(const unsigned short *tab, short size)
{
	if (size>omd_FONT_TRANSLATE_TABLE_SIZE) omd_EXCEPTION(omcerr_OutOfRange);
	
	OMediaMemTools::copy(tab,trans_tab,size<<1);
}


void OMediaCanvasFont::draw_string(string text, OMediaDrawInterface *output, 
									long x, long y,
									omt_BlendFunc blend_src,
									omt_BlendFunc blend_dest)
{
	OMediaRect	r;
	long		i,j,w,ox;

	if (!font) return;

	font->lock(omlf_Read);
	output->lock(omlf_Write);

	if (proportional && prop_tab)
	{
		for(i=0;i<(long)text.length();i++)
		{
			if ((text.c_str())[i]==' ')	// Space
			{
				x += space_size;
			}
			else
			{	
				j = trans_tab[(unsigned char)(text.c_str())[i]];
				ox = prop_tab[(j<<1)];
				w = prop_tab[(j<<1)+1];
		
				r.left = ox;
				r.top = j*vertical_delta;
				r.right = ox+w;
				r.bottom = r.top + fheight;
				output->draw(font,&r,x,y,blend_src,blend_dest);
				x += w + char_space;
			}
		}
	}
	else
	{
		for(i=0;i<(long)text.length();i++)
		{
			j = trans_tab[(unsigned char)(text.c_str())[i]];
		
			r.left = 0;
			r.top = j*vertical_delta;
			r.right = fwidth;
			r.bottom = r.top + fheight;
			output->draw(font,&r,x,y,blend_src,blend_dest);
			x += r.get_width();
		}
	}

	font->unlock();
	output->unlock();	
}


void OMediaCanvasFont::set_proportional_tab(short *tab, short size)
{
	delete [] prop_tab;
	prop_tab = NULL;
	
	prop_tab = new short[size*2];
	
	OMediaMemTools::copy(tab,prop_tab,(size*2*sizeof(short)));
}


void OMediaCanvasFont::create_proportional_tab(void)
{
	OMediaARGBColor	rgb;
	long			x,y,l,max_x,min_x,full_max_x;
	long			max_width;
	bool			one_found;

	full_max_x = 0;

	delete [] prop_tab;
	prop_tab = new short[omd_FONT_TRANSLATE_TABLE_SIZE*2];
		
	if (!font || !prop_tab) return;
	
	font->lock(omlf_Read);
	
	max_width = font->get_width();
	if (max_width>fwidth) max_width =fwidth;
	
	for(l=0;;l+=vertical_delta)
	{
		if (l+fheight>font->get_height() || (l/vertical_delta)>=omd_FONT_TRANSLATE_TABLE_SIZE) break;
		
		one_found = false;
		max_x = 0;
		min_x = max_width;
		for(y=0; y<fheight;y++)
		{
			for(x=max_x; x<max_width; x++)
			{
				font->read_pixel(rgb,x,y+l);
				if (rgb.alpha != 0)
		        {
					if (x>max_x) max_x = x;
				    one_found = true;
				}
			}

			for(x=0; x<max_width; x++)
			{
				font->read_pixel(rgb,x,y+l);
				if (rgb.alpha != 0) 
				{
					if (x<min_x) min_x = x;
					break;
			    }
			}
		}
		
		if (!one_found) 
		{
			max_x = max_width;
			min_x = 0;
		}
		else 
		{
			max_x++;
			if (full_max_x<max_x) full_max_x = max_x;
		}
		
		prop_tab[(l/vertical_delta)<<1] = min_x;
		prop_tab[((l/vertical_delta)<<1)+1] = max_x - min_x;
	}
	
	font->unlock();
	
	space_size = full_max_x;
}

long OMediaCanvasFont::get_text_length(string str)
{
	if (!proportional || !prop_tab)
	{
		return str.length() * fwidth;
	}
	else
	{
		long siz = 0,i,pos;
	
		for(i=0; i<(long)str.length();i++)
		{
			if ((str.c_str())[i]!=' ') 
			{
				pos = trans_tab[(unsigned char)(str.c_str())[i]];
				siz += prop_tab[(pos<<1L)+1]; 
				if (i!=(long)str.length()-1) siz += char_space;
			}
			else siz += space_size;
		}
	
		return siz;
	}
}

void OMediaCanvasFont::set_inverse_table(unsigned short *translate_invtable)
{
	// Fill the font translation table wih an empty character

	unsigned short emptychar = 0,p;
	short i;
	
	for(i=0; translate_invtable[i]; i++)
	{
		if (translate_invtable[i] == (unsigned short)'.') 
		{
			emptychar = i;
			break;
		}
	}

	for(i=0; i<omd_FONT_TRANSLATE_TABLE_SIZE; i++)
	{
		trans_tab[i] = emptychar;
	}

	for(i=0; translate_invtable[i]; i++)
	{	
		p = ((unsigned short)translate_invtable[i]);
		if (p>=omd_FONT_TRANSLATE_TABLE_SIZE) omd_EXCEPTION(omcerr_OutOfRange);	
		trans_tab[p] = i;
	}

}

void OMediaCanvasFont::set_default_inverse_table(void)
{
	short 	i;
	unsigned short	apos,bpos,p;

	set_inverse_table(translate_invtable);

	apos = bpos = 0;

	for(i=0; translate_invtable[i]; i++)
	{
		if (translate_invtable[i]==(unsigned char)'\'') apos = i;
		if (translate_invtable[i]==(unsigned char)'\"') bpos = i;
	}

	p = ((unsigned char)'Õ');
	trans_tab[p] = apos;

	p = ((unsigned char)'Ò');
	trans_tab[p] = bpos;

	p = ((unsigned char)'Ó');
	trans_tab[p] = bpos;
}

unsigned short OMediaCanvasFont::translate_invtable[] = 
{
	(unsigned char)'A',(unsigned char)'B',(unsigned char)'C',(unsigned char)'D',(unsigned char)'E',
	(unsigned char)'F',(unsigned char)'G',(unsigned char)'H',(unsigned char)'I',(unsigned char)'J',
	(unsigned char)'K',(unsigned char)'L',(unsigned char)'M',(unsigned char)'N',(unsigned char)'O',
	(unsigned char)'P',(unsigned char)'Q',(unsigned char)'R',(unsigned char)'S',(unsigned char)'T',
	(unsigned char)'U',(unsigned char)'V',(unsigned char)'W',(unsigned char)'X',(unsigned char)'Y',
	(unsigned char)'Z',(unsigned char)'a',(unsigned char)'b',(unsigned char)'c',(unsigned char)'d',
	(unsigned char)'e',(unsigned char)'f',(unsigned char)'g',(unsigned char)'h',(unsigned char)'i',
	(unsigned char)'j',(unsigned char)'k',(unsigned char)'l',(unsigned char)'m',(unsigned char)'n',
	(unsigned char)'o',(unsigned char)'p',(unsigned char)'q',(unsigned char)'r',(unsigned char)'s',
	(unsigned char)'t',(unsigned char)'u',(unsigned char)'v',(unsigned char)'w',(unsigned char)'x',
	(unsigned char)'y',(unsigned char)'z',(unsigned char)'0',(unsigned char)'1',(unsigned char)'2',
	(unsigned char)'3',(unsigned char)'4',(unsigned char)'5',(unsigned char)'6',(unsigned char)'7',
	(unsigned char)'8',(unsigned char)'9',(unsigned char)'(',(unsigned char)')',(unsigned char)'.',
	(unsigned char)';',(unsigned char)',',(unsigned char)'!',(unsigned char)'?',(unsigned char)':',
	(unsigned char)'{',(unsigned char)'}',(unsigned char)'+',(unsigned char)'-',(unsigned char)'=',
	(unsigned char)'[',(unsigned char)']',(unsigned char)'#',(unsigned char)'@',(unsigned char)' ',
	(unsigned char)'©',(unsigned char)'$',(unsigned char)'"',(unsigned char)'/',(unsigned char)'>',
	(unsigned char)'<',(unsigned char)'\'',(unsigned char)'&',(unsigned char)'%',(unsigned char)'*',
	(unsigned char)'¡',(unsigned char)'ˆ',(unsigned char)'',(unsigned char)'Ž',(unsigned char)'™',
	(unsigned char)'',(unsigned char)'‰',(unsigned char)'”',(unsigned char)'ž',(unsigned char)'•',
	(unsigned char)'š',(unsigned char)'Š',(unsigned char)'Ÿ',(unsigned char)'Ø',(unsigned char)'‘',
	(unsigned char)'‹',(unsigned char)'›',(unsigned char)'',(unsigned char)'',(unsigned char)'_',
	(unsigned char)'~',(unsigned char)'¤',
	0
};


OMediaDBObject *OMediaCanvasFont::db_builder(void)
{
	return new OMediaCanvasFont();
}

void OMediaCanvasFont::read_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;
	long						tabsize,i;
	long						version = 0;

	OMediaDBObject::read_class(stream);

	stream>>version;
	stream>>slink;
	set_font_canvas((OMediaCanvas*)slink.get_object());

	stream>>fwidth;
	stream>>fheight;
	stream>>tabsize;
	for(i=0;i<tabsize && i<omd_FONT_TRANSLATE_TABLE_SIZE;i++) stream>>trans_tab[i];
	stream>>proportional;
	stream>>space_size;
	stream>>char_space;
	stream>>vertical_delta;
	
	if (proportional) create_proportional_tab();
}

void OMediaCanvasFont::write_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;
	long						version = 0;
	long						tabsize=omd_FONT_TRANSLATE_TABLE_SIZE,i;

	OMediaDBObject::read_class(stream);

	slink.set_object(font);
	
	stream<<version;
	stream<<slink;

	stream<<fwidth;
	stream<<fheight;
	stream<<tabsize;
	for(i=0;i<omd_FONT_TRANSLATE_TABLE_SIZE;i++) stream<<trans_tab[i];
	stream<<proportional;
	stream<<space_size;
	stream<<char_space;
	stream<<vertical_delta;
}

unsigned long OMediaCanvasFont::get_approximate_size(void)
{
	return sizeof(*this);
}

unsigned long OMediaCanvasFont::db_get_type(void) const
{
	return OMediaCanvasFont::db_type;
}
