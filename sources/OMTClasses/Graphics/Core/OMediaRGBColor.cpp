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
 
 
 #include "OMediaRGBColor.h"

// These methods should be improved by using shift/or operation.

unsigned long OMediaRGBColor::getrgb32bits(void) const
{
	unsigned char r,g,b;
	unsigned long rgb;
	
	r =  (unsigned char)(((unsigned long)red * 0xFFL)/0xFFFFL);
	g =  (unsigned char)(((unsigned long)green * 0xFFL)/0xFFFFL);
	b =  (unsigned char)(((unsigned long)blue * 0xFFL)/0xFFFFL);
	
	rgb = ((unsigned long)r<<16L)|((unsigned long)g<<8L)|(unsigned long)b;
	
	return rgb;
}

void OMediaRGBColor::setrgb32bits(unsigned long rgb)
{
	unsigned char r,g,b;
	
	r = (unsigned char) (((rgb)>>16L)&0xFFL);
	g = (unsigned char) (((rgb)>>8L)&0xFFL);
	b = (unsigned char) (((rgb))&0xFFL);

	red =  (unsigned short)(((unsigned long)r * 0xFFFFL)/0xFFL);
	green =  (unsigned short)(((unsigned long)g * 0xFFFFL)/0xFFL);
	blue =  (unsigned short)(((unsigned long)b * 0xFFFFL)/0xFFL);			
}

unsigned long OMediaRGBColor::getbgr32bits(void) const
{
	unsigned char r,g,b;
	unsigned long rgb;
	
	r =  (unsigned char)(((unsigned long)red * 0xFFL)/0xFFFFL);
	g =  (unsigned char)(((unsigned long)green * 0xFFL)/0xFFFFL);
	b =  (unsigned char)(((unsigned long)blue * 0xFFL)/0xFFFFL);
	
	rgb = ((unsigned long)b<<16L)|((unsigned long)g<<8L)|(unsigned long)r;
	
	return rgb;
}

void OMediaRGBColor::setbgr32bits(unsigned long rgb)
{
	unsigned char r,g,b;
	
	b = (unsigned char) (((rgb)>>16L)&0xFFL);
	g = (unsigned char) (((rgb)>>8L)&0xFFL);
	r = (unsigned char) (((rgb))&0xFFL);

	red =  (unsigned short)(((unsigned long)r * 0xFFFFL)/0xFFL);
	green =  (unsigned short)(((unsigned long)g * 0xFFFFL)/0xFFL);
	blue =  (unsigned short)(((unsigned long)b * 0xFFFFL)/0xFFL);			
}

unsigned char OMediaRGBColor::getred8bits(void) const
{
	return (unsigned char)(((unsigned long)red * 0xFFL)/0xFFFFL);
}
	
unsigned char OMediaRGBColor::getgreen8bits(void) const
{
	return (unsigned char)(((unsigned long)green * 0xFFL)/0xFFFFL);
}

unsigned char OMediaRGBColor::getblue8bits(void) const
{
	return (unsigned char)(((unsigned long)blue * 0xFFL)/0xFFFFL);
}

void OMediaRGBColor::setred8bits(unsigned char r)
{
	red =  (unsigned short)(((unsigned long)r * 0xFFFFL)/0xFFL);
}

void OMediaRGBColor::setgreen8bits(unsigned char g)
{
	green =  (unsigned short)(((unsigned long)g * 0xFFFFL)/0xFFL);
}

void OMediaRGBColor::setblue8bits(unsigned char b)
{
	blue =  (unsigned short)(((unsigned long)b * 0xFFFFL)/0xFFL);
}



unsigned short OMediaRGBColor::getrgb16bits(void) const
{
	unsigned char r,g,b;
	unsigned short rgb;
	
	r =  (unsigned char)(((unsigned long)red * 0x1FL)/0xFFFFL);
	g =  (unsigned char)(((unsigned long)green * 0x1FL)/0xFFFFL);
	b =  (unsigned char)(((unsigned long)blue * 0x1FL)/0xFFFFL);
	
	rgb = (unsigned short)(((unsigned long)r<<10L)|((unsigned long)g<<5L)|(unsigned long)b);
	
	return rgb;
}

void OMediaRGBColor::setrgb16bits(unsigned short rgb)
{
	unsigned char r,g,b;
	
	r = (unsigned char) (((rgb)>>10L)&0x1FL);
	g = (unsigned char) (((rgb)>>5L)&0x1FL);
	b = (unsigned char) (((rgb))&0x1FL);

	red =  (unsigned short)(((unsigned long)r * 0xFFFFL)/0x1FL);
	green =  (unsigned short)(((unsigned long)g * 0xFFFFL)/0x1FL);
	blue =  (unsigned short)(((unsigned long)b * 0xFFFFL)/0x1FL);			
}

unsigned short OMediaRGBColor::getbgr16bits(void) const
{
	unsigned char r,g,b;
	unsigned long rgb;
	
	r =  (unsigned char)(((unsigned long)red * 0x1FL)/0xFFFFL);
	g =  (unsigned char)(((unsigned long)green * 0x1FL)/0xFFFFL);
	b =  (unsigned char)(((unsigned long)blue * 0x1FL)/0xFFFFL);
	
	rgb = ((unsigned long)b<<10L)|((unsigned long)g<<5L)|(unsigned long)r;
	
	return (unsigned short)rgb;
}

void OMediaRGBColor::setbgr16bits(unsigned short rgb)
{
	unsigned char r,g,b;
	
	b = (unsigned char) (((rgb)>>10L)&0x1FL);
	g = (unsigned char) (((rgb)>>5L)&0x1FL);
	r = (unsigned char) (((rgb))&0x1FL);

	red =  (unsigned short)(((unsigned long)r * 0xFFFFL)/0x1FL);
	green =  (unsigned short)(((unsigned long)g * 0xFFFFL)/0x1FL);
	blue =  (unsigned short)(((unsigned long)b * 0xFFFFL)/0x1FL);			
}

void OMediaRGBColor::apply(const OMediaARGBColor &argb, bool precise)
{		
	unsigned long gradient = argb.alpha;

	if (!gradient) return;

	if (!precise)
	{
		red = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)red + gradient*(unsigned long)argb.red)>>16L);
		green = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)green + gradient*(unsigned long)argb.green)>>16L);
		blue = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)blue + gradient*(unsigned long)argb.blue)>>16L);
	}
	else
	{
		red = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)red + gradient*(unsigned long)argb.red)/0xFFFFL);
		green = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)green + gradient*(unsigned long)argb.green)/0xFFFFL);
		blue = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)blue + gradient*(unsigned long)argb.blue)/0xFFFFL);
	}
}

void OMediaARGBColor::apply(const OMediaARGBColor &argb, bool precise)
{	
	if (argb.alpha>alpha) alpha = argb.alpha;
		
	unsigned long gradient = argb.alpha;

	if (!precise)
	{
		red = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)red + gradient*(unsigned long)argb.red)>>16L);
		green = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)green + gradient*(unsigned long)argb.green)>>16L);
		blue = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)blue + gradient*(unsigned long)argb.blue)>>16L);
	}
	else
	{
		red = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)red + gradient*(unsigned long)argb.red)/0xFFFFL);
		green = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)green + gradient*(unsigned long)argb.green)/0xFFFFL);
		blue = (unsigned short)(((0xFFFFL-gradient)*(unsigned long)blue + gradient*(unsigned long)argb.blue)/0xFFFFL);
	}
}

void OMediaARGBColor::scaled_add(const OMediaARGBColor &argb, bool precise)
{
	unsigned long a,r,g,b,a2,r2,g2,b2;

	if (argb.alpha>alpha) 
	{
		a=argb.alpha; 
		r=argb.red;
		g=argb.green;
		b=argb.blue;
		a2 = alpha;
		r2 = red;
		g2 = green;
		b2 = blue;
	}
	else
	{
		a2=argb.alpha; 
		r2=argb.red;
		g2=argb.green;
		b2=argb.blue;
		a = alpha;
		r = red;
		g = green;
		b = blue;
	}

	if (a>=a2)
	{
		unsigned long scale = a2;

		if (!precise)
		{
			r2 = (scale*r2)>>16L;
			g2 = (scale*g2)>>16L;
			b2 = (scale*b2)>>16L;
		}
		else
		{
			r2 = (scale*r2)/0xFFFF;
			g2 = (scale*g2)/0xFFFF;
			b2 = (scale*b2)/0xFFFF;
		}
	}

	r += r2;
	g += g2;
	b += b2;
	if (r>0xFFFF) r = 0xFFFF;
	if (g>0xFFFF) g = 0xFFFF;
	if (b>0xFFFF) b = 0xFFFF;
	
	alpha = (unsigned short)a;
	red = (unsigned short)r;
	green = (unsigned short)g;
	blue = (unsigned short)b;
} 

