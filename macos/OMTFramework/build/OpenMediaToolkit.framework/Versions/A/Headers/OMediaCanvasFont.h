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
#ifndef OMEDIA_CanvasFont_H
#define OMEDIA_CanvasFont_H

#include "OMediaFont.h"
#include "OMediaDBObject.h"

class OMediaCanvas;

#ifndef omd_FONT_TRANSLATE_TABLE_SIZE
#define omd_FONT_TRANSLATE_TABLE_SIZE	256
#endif


// By default this object is initialized with a default translation table. Look at bottom of this header.

class OMediaCanvasFont : public OMediaDBObject,
						 public OMediaFont
{
	public:

	// * Constructor

	omtshared OMediaCanvasFont();
	omtshared virtual ~OMediaCanvasFont();

	// * The font
	
	omtshared virtual void set_font_canvas(OMediaCanvas *, long fwidth, 
														   long fheight,
														   long vertical_delta);	
	omtshared virtual void set_font_canvas(OMediaCanvas *);	
	
	// The font canvas is a one dimension table that contains one image per
	// character. The size of the table should be (fontwidth*nchar) * vertical_delta.
	// The table is vertical oriented to allow the best speed result when
	// the canvas is compressed. fwidth and fheight represent a size of one charactere
	// in pixel. The vertical_delta is the vertical distance in pixel from the top of 
	// a character to the top of the next charactere.
	
	inline OMediaCanvas *get_font_canvas(void) const {return font;}
	inline long get_font_width(void) const {return fwidth;}
	inline long get_font_height(void) const {return fheight;}
	inline long get_font_vertical_delta(void) const {return vertical_delta;}

	omtshared virtual void get_font_info(long &w, long &h, long &vertical_modulo);


	// * Translation table
	
	omtshared virtual void set_translation_tab(const unsigned short *tab, short size);
	inline const unsigned short *get_translation_tab(void) const {return trans_tab;}
	
	// The translation table is used to convert an ASCII code to a position in the font
	// array. For example if your canvas contains "A,B,...", the table
	// positions tab['A'], tab['B'] should contains 0,1,...
	// Table is copied. You can also initialize the translation table using an inverse
	// table.


	// * Proportional font
	
	inline void set_proportional(bool prop) {proportional = prop;}
	inline bool get_proportional(void) {return proportional;}

		// Please note that the proportional table format has changed since the V1.x
		// New format is short table[size][2]
		// where table[x][0] is an horizontal offset in the source image for the charactere
		// and table[x][1] is the size of the charactere
		// Table is copied
		
	omtshared virtual void set_proportional_tab(short *prop_tab, short size);
	inline const short *get_proportional_tab(void) const {return prop_tab;}
		

		// OMT V2.0 uses the alpha-channel to find the space between characters.
		// An alpha of 0 is considered as an empty space.
		
	omtshared virtual void create_proportional_tab(void);
	// You should set all the other parameters before calling this function
	
	inline void set_space_size(long ss) {space_size = ss;}
	inline long get_space_size(void) const {return space_size;}

	inline void set_char_space(long cs) {char_space = cs;}
	inline long get_char_space(void) const {return char_space;}

	
	// * Draw text
	
	omtshared virtual void draw_string(string text, OMediaDrawInterface *output, long x, long y,omt_BlendFunc blend_src =omblendfc_One,  omt_BlendFunc blend_dest =omblendfc_Zero);


	// * Calculate string length in pixels
	
	omtshared virtual long get_text_length(string str);


	// * Inverse table. You can initialize your translate table using an inverse table. The inverse table
	// is easier to define than a translate table, because it contains characters as they appear in the
	// canvas. Table must be terminated by zero.
	
	omtshared virtual void set_inverse_table(unsigned short *itable);

	omtshared virtual void set_default_inverse_table(void);

	// Default inverse table

	inline unsigned short *get_translate_invtable(void) {return translate_invtable;}


	// * Database/streamer support
	
	enum { db_type = 'Font' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;


	protected:

	static unsigned short translate_invtable[];	// default inverse table

	
	OMediaCanvas 	*font;
	long			fwidth,fheight;
	unsigned short	trans_tab[omd_FONT_TRANSLATE_TABLE_SIZE];
	bool			proportional;
	short			*prop_tab;
	long			space_size;			// Space character size in pixel
	long			char_space;			// Space between two characters
	long			vertical_delta;



};



/*

This is the default character table. Cut & paste into your drawing application to create a font
canvas.

A
B
C
D
E
F
G
H
I
J
K
L
M
N
O
P
Q
R
S
T
U
V
W
X
Y
Z
a
b
c
d
e
f
g
h
i
j
k
l
m
n
o
p
q
r
s
t
u
v
w
x
y
z
0
1
2
3
4
5
6
7
8
9
(
)
.
;
,
!
?
:
{
}
+
-
=
[
]
#
@
  
©
$
"
/
>
<
'
&
%
*
¡
ˆ

Ž
™

‰
”
ž
•
š
Š
Ÿ
Ø
‘
‹
›


_
~
¤
*/

#endif