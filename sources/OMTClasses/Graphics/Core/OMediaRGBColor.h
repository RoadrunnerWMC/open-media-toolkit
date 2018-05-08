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
#ifndef OMEDIA_RGBColor_H
#define OMEDIA_RGBColor_H

#include "OMediaTypes.h"
#include "OMediaPixelFormat.h"
#include "OMediaEndianSupport.h"

class OMediaRGBColor;
class OMediaARGBColor;
class OMediaFRGBColor;
class OMediaFARGBColor;
class OMediaPackedARGBColor;

// Packed ARGB Color

class OMediaPackedARGBColor
{
	public:

	unsigned long argb;

	inline int operator==(const OMediaPackedARGBColor& x) const {return (argb == x.argb);}
	inline int operator!=(const OMediaPackedARGBColor& x) const {return (argb != x.argb);}


	inline void set(unsigned short r, unsigned short g, unsigned short b)
	{
		argb = ((r>>8L)<<16L)|((g>>8L)<<8L)|(b>>8L);
	}

	inline void set(unsigned short a, unsigned short r, unsigned short g, unsigned short b)
	{
		argb = ((a>>8L)<<24L)|((r>>8L)<<16L)|((g>>8L)<<8L)|(b>>8L);
	}

	inline void fset(float a, float r, float g, float b)	// 0 - 1.0
	{
		set( (unsigned short)(a*(float(0xFFFF))),
			 (unsigned short)(r*(float(0xFFFF))),
			 (unsigned short)(g*(float(0xFFFF))),
			 (unsigned short)(b*(float(0xFFFF))) );
	
	}

	inline void fset(float r, float g, float b)	// 0 - 1.0
	{
		set((unsigned short)(r*(float(0xFFFF))),
			 (unsigned short)(g*(float(0xFFFF))),
			 (unsigned short)(b*(float(0xFFFF))) );
	
	}


	inline void fget(float &a, float &r, float &g, float &b) const	// 0 - 1.0
	{
		const float d = 1.0f/float(0xFF);
	
		a = ((float)((argb>>24L)&0xFF))*d;
		r = ((float)((argb>>16L)&0xFF))*d;
		g = ((float)((argb>>8L)&0xFF))*d;
		b = ((float)((argb)&0xFF))*d;	
	}

	inline void fget(float &r, float &g, float &b) const	// 0 - 1.0
	{
		const float d = 1.0f/float(0xFF);
	
		r = ((float)((argb>>16L)&0xFF))*d;
		g = ((float)((argb>>8L)&0xFF))*d;
		b = ((float)((argb)&0xFF))*d;	
	}

	inline void get(unsigned short &a, unsigned short &r, unsigned short &g, unsigned short &b) const
	{
		a = (unsigned short)((argb>>24L)&0xFF);	a = a|(a<<8);		
		r = (unsigned short)((argb>>16L)&0xFF);	r = r|(r<<8);		
		g = (unsigned short)((argb>>8L)&0xFF);	g = g|(g<<8);		
		b = (unsigned short)((argb)&0xFF);		b = b|(b<<8);
	}

	inline void get(unsigned short &r, unsigned short &g, unsigned short &b) const
	{
		r = (unsigned short)((argb>>16L)&0xFF);	r = r|(r<<8);		
		g = (unsigned short)((argb>>8L)&0xFF);	g = g|(g<<8);		
		b = (unsigned short)((argb)&0xFF);		b = b|(b<<8);
	}

	inline void set(const OMediaARGBColor &argb);
	inline void get(OMediaARGBColor &argb) const;

	inline void set(const OMediaFARGBColor &argb);
	inline void get(OMediaFARGBColor &argb) const;

	inline void set(const OMediaRGBColor &rgb);
	inline void get(OMediaRGBColor &argb) const;

	inline void set(const OMediaFRGBColor &rgb);
	inline void get(OMediaFRGBColor &argb) const;

};

// Packed RGBA Color

class OMediaPackedRGBAColor
{
	public:

	omt_RGBAPixel rgba;

	inline int operator==(const OMediaPackedRGBAColor& x) const {return (rgba == x.rgba);}
	inline int operator!=(const OMediaPackedRGBAColor& x) const {return (rgba != x.rgba);}

	inline void set(unsigned short r, unsigned short g, unsigned short b)
	{
		rgba = ((r>>8L)<<24)|((g>>8L)<<16L)|((b>>8L)<<8L);
	}

	inline void set(unsigned short a, unsigned short r, unsigned short g, unsigned short b)
	{
		rgba = ((r>>8L)<<24L)|((g>>8L)<<16L)|((b>>8L)<<8L)|(a>>8L);
	}

	inline void fset(float a, float r, float g, float b)	// 0 - 1.0
	{
		set( (unsigned short)(a*(float(0xFFFF))),
			 (unsigned short)(r*(float(0xFFFF))),
			 (unsigned short)(g*(float(0xFFFF))),
			 (unsigned short)(b*(float(0xFFFF))) );
	
	}

	inline void fset(float r, float g, float b)	// 0 - 1.0
	{
		set((unsigned short)(r*(float(0xFFFF))),
			 (unsigned short)(g*(float(0xFFFF))),
			 (unsigned short)(b*(float(0xFFFF))) );
	
	}


	inline void fget(float &a, float &r, float &g, float &b) const	// 0 - 1.0
	{
		const float d = 1.0f/float(0xFF);
	
		r = ((float)((rgba>>24L)&0xFF))*d;
		g = ((float)((rgba>>16L)&0xFF))*d;
		b = ((float)((rgba>>8L)&0xFF))*d;
		a = ((float)((rgba)&0xFF))*d;	
	}

	inline void fget(float &r, float &g, float &b) const	// 0 - 1.0
	{
		const float d = 1.0f/float(0xFF);
	
		r = ((float)((rgba>>24L)&0xFF))*d;
		g = ((float)((rgba>>16L)&0xFF))*d;
		b = ((float)((rgba>>8L)&0xFF))*d;
	}

	inline void get(unsigned short &a, unsigned short &r, unsigned short &g, unsigned short &b) const
	{
		r = (unsigned short)((rgba>>24L)&0xFF);	r = r|(r<<8);		
		g = (unsigned short)((rgba>>16L)&0xFF);	g = g|(g<<8);		
		b = (unsigned short)((rgba>>8L)&0xFF);	b = b|(b<<8);		
		a = (unsigned short)((rgba)&0xFF);		a = a|(a<<8);
	}

	inline void get(unsigned short &r, unsigned short &g, unsigned short &b) const
	{
		r = (unsigned short)((rgba>>24L)&0xFF);	r = r|(r<<8);		
		g = (unsigned short)((rgba>>16L)&0xFF);	g = g|(g<<8);		
		b = (unsigned short)((rgba>>8L)&0xFF);	b = b|(b<<8);		
	}

	inline void set(const OMediaARGBColor &argb);
	inline void get(OMediaARGBColor &argb) const;

	inline void set(const OMediaFARGBColor &argb);
	inline void get(OMediaFARGBColor &argb) const;

	inline void set(const OMediaRGBColor &rgb);
	inline void get(OMediaRGBColor &rgb) const;

	inline void set(const OMediaFRGBColor &rgb);
	inline void get(OMediaFRGBColor &rgb) const;

};


// RGB Color 3 x 16bits

class OMediaRGBColor
{
	public:
	
	inline OMediaRGBColor(void) {}
	inline OMediaRGBColor(unsigned short ared, unsigned short agreen, unsigned short ablue) 
						{red = ared; green = agreen; blue = ablue;}

	inline void set(unsigned short ared, unsigned short agreen, unsigned short ablue)
						{red = ared; green = agreen; blue = ablue;}


	
	omtshared unsigned long getrgb32bits() const;
	omtshared void setrgb32bits(unsigned long rgb);

	omtshared unsigned long getbgr32bits() const;
	omtshared void setbgr32bits(unsigned long rgb);

	omtshared unsigned short getrgb16bits() const;
	omtshared void setrgb16bits(unsigned short rgb);

	omtshared unsigned short getbgr16bits() const;
	omtshared void setbgr16bits(unsigned short rgb);


	omtshared unsigned char getred8bits(void) const;
	omtshared unsigned char getgreen8bits(void) const;
	omtshared unsigned char getblue8bits(void) const;
	
	omtshared void setred8bits(unsigned char r);
	omtshared void setgreen8bits(unsigned char g);
	omtshared void setblue8bits(unsigned char b);

	inline int operator==(const OMediaRGBColor& x) const {return (red == x.red && green == x.green && blue ==x.blue);}
	inline int operator!=(const OMediaRGBColor& x) const {return (red != x.red || green != x.green || blue !=x.blue);}

	void apply(const OMediaARGBColor &argb, bool precise =false);

	inline unsigned short max_gun(void) const
	{
		if (red > green)
		{
			if (red > blue ) return red;
			else return blue;
		}
		else
		{
			if (green > blue) return green;
			else return blue;
		}
	}

	inline unsigned short min_gun(void) const
	{
		if (red < green)
		{
			if (red < blue ) return red;
			else return blue;
		}
		else
		{
			if (green < blue) return green;
			else return blue;
		}
	}

	inline void fset(float r, float g, float b)	// 0 - 1.0
	{
		red = (unsigned short)(r*(float(0xFFFF)));
		green = (unsigned short)(g*(float(0xFFFF)));
		blue = (unsigned short)(b*(float(0xFFFF)));
	}

	inline void fget(float &r, float &g, float &b) const	// 0 - 1.0
	{
		const float d = 1.0f/float(0xFFFF);
	
		r = ((float)red)*d;
		g = ((float)green)*d;
		b = ((float)blue)*d;	
	}


	unsigned short red,green,blue;
};

// ARGB Color 4 x 16bits

class OMediaARGBColor : public OMediaRGBColor
{
	public:
	
	inline OMediaARGBColor(void) {}
	inline OMediaARGBColor(unsigned short ared, unsigned short agreen, unsigned short ablue):
		   OMediaRGBColor(ared,agreen,ablue) {}

	inline OMediaARGBColor(unsigned short aalpha, unsigned short ared, unsigned short agreen, unsigned short ablue):
		   OMediaRGBColor(ared,agreen,ablue) {alpha = aalpha;}

   	inline void set(unsigned short ared, unsigned short agreen, unsigned short ablue)
					{red = ared; green = agreen; blue = ablue;}

   	inline void set(unsigned short aalpha, unsigned short ared, unsigned short agreen, unsigned short ablue)
					{red = ared; green = agreen; blue = ablue; alpha = aalpha;}


	void apply(const OMediaARGBColor &argb, bool precise =false);
	void scaled_add(const OMediaARGBColor &argb, bool precise =false);

	inline int operator==(const OMediaARGBColor& x) const {return (alpha==x.alpha && red == x.red && green == x.green && blue ==x.blue);}
	inline int operator!=(const OMediaARGBColor& x) const {return (alpha!=x.alpha || red != x.red || green != x.green || blue !=x.blue);}


	inline void fset(float a, float r, float g, float b)	// 0 - 1.0
	{
		alpha = (unsigned short)(a*(float(0xFFFF)));
		red = (unsigned short)(r*(float(0xFFFF)));
		green = (unsigned short)(g*(float(0xFFFF)));
		blue = (unsigned short)(b*(float(0xFFFF)));
	}

	inline void fget(float &a, float &r, float &g, float &b) const	// 0 - 1.0
	{
		const float d = 1.0f/float(0xFFFF);
	
		a = ((float)alpha)*d;
		r = ((float)red)*d;
		g = ((float)green)*d;
		b = ((float)blue)*d;	
	}

	inline omt_RGBAPixel get_rgba(void) const
	{
		OMediaPackedRGBAColor	packed;
		packed.set(*this);
		return (packed.rgba);
	}

	inline void set_rgba(omt_RGBAPixel rgba)
	{
		OMediaPackedRGBAColor	packed;
		packed.rgba =  (rgba);
		packed.get(*this);
	}

	unsigned short alpha;
};



// RGB Color 3 x float (0.0f - 1.0f)

class OMediaFRGBColor
{
	public:
	
	inline OMediaFRGBColor(void) {}
	inline OMediaFRGBColor(float ared, float agreen, float ablue) 
						{red = ared; green = agreen; blue = ablue;}

	inline void set(float ared, float agreen, float ablue)
						{red = ared; green = agreen; blue = ablue;}

	inline int operator==(const OMediaRGBColor& x) const {return (red == x.red && green == x.green && blue ==x.blue);}
	inline int operator!=(const OMediaRGBColor& x) const {return (red != x.red || green != x.green || blue !=x.blue);}

	inline float max_gun(void) const
	{
		if (red > green)
		{
			if (red > blue ) return red;
			else return blue;
		}
		else
		{
			if (green > blue) return green;
			else return blue;
		}
	}

	inline float min_gun(void) const
	{
		if (red < green)
		{
			if (red < blue ) return red;
			else return blue;
		}
		else
		{
			if (green < blue) return green;
			else return blue;
		}
	}

	inline omt_RGBAPixel get_rgba(void) const
	{
		OMediaPackedRGBAColor	packed;
		packed.set(*this);
		return (packed.rgba);
	}

	inline void set_rgba(omt_RGBAPixel rgba)
	{
		OMediaPackedRGBAColor	packed;
		packed.rgba = (rgba);
		packed.get(*this);
	}

	inline unsigned long get_argb(void) const
	{
		OMediaPackedARGBColor	packed;
		packed.set(*this);
		return (packed.argb);
	}

	inline void set_argb(unsigned long  argb)
	{
		OMediaPackedARGBColor	packed;
		packed.argb = (argb);
		packed.get(*this);
	}



	float red,green,blue;
};

// ARGB Color 4 x float (0.0f - 1.0f)

class OMediaFARGBColor : public OMediaFRGBColor
{
	public:
	
	inline OMediaFARGBColor(void) {}
	inline OMediaFARGBColor(float ared, float agreen, float ablue):
		   OMediaFRGBColor(ared,agreen,ablue) {}

	inline OMediaFARGBColor(float aalpha, float ared, float agreen, float ablue):
		   OMediaFRGBColor(ared,agreen,ablue) {alpha = aalpha;}


	inline int operator==(const OMediaFARGBColor& x) const {return (alpha==x.alpha && red == x.red && green == x.green && blue ==x.blue);}
	inline int operator!=(const OMediaFARGBColor& x) const {return (alpha!=x.alpha || red != x.red || green != x.green || blue !=x.blue);}


   	inline void set(float ared, float agreen, float ablue)
					{red = ared; green = agreen; blue = ablue;}

   	inline void set(float aalpha, float ared, float agreen, float ablue)
					{red = ared; green = agreen; blue = ablue; alpha = aalpha;}

	inline void set(const OMediaRGBColor &rgb)
	{
		const float d = 1.0f/float(0xFFFF);

		red = ((float)rgb.red)*d;
		green = ((float)rgb.green)*d;
		blue = ((float)rgb.blue)*d;	
	}

	inline void get(OMediaRGBColor &rgb) const
	{
		rgb.red = (unsigned short)(red*(float(0xFFFF)));
		rgb.green = (unsigned short)(green*(float(0xFFFF)));
		rgb.blue = (unsigned short)(blue*(float(0xFFFF)));
	}

	inline void set(const OMediaARGBColor &rgb)
	{
		const float d = 1.0f/float(0xFFFF);

		alpha = ((float)rgb.alpha)*d;
		red = ((float)rgb.red)*d;
		green = ((float)rgb.green)*d;
		blue = ((float)rgb.blue)*d;	
	}

	inline void get(OMediaARGBColor &rgb) const
	{
		rgb.alpha = (unsigned short)(alpha*(float(0xFFFF)));
		rgb.red = (unsigned short)(red*(float(0xFFFF)));
		rgb.green = (unsigned short)(green*(float(0xFFFF)));
		rgb.blue = (unsigned short)(blue*(float(0xFFFF)));
	}

	inline omt_RGBAPixel get_rgba(void) const
	{
		OMediaPackedRGBAColor	packed;
		packed.set(*this);
		return (packed.rgba);
	}

	inline void set_rgba(omt_RGBAPixel rgba)
	{
		OMediaPackedRGBAColor	packed;
		packed.rgba = (rgba);
		packed.get(*this);
	}

	inline unsigned long get_argb(void) const
	{
		OMediaPackedARGBColor	packed;
		packed.set(*this);
		return (packed.argb);
	}

	inline void set_argb(unsigned long  argb)
	{
		OMediaPackedARGBColor	packed;
		packed.argb = (argb);
		packed.get(*this);
	}

	float alpha;
};


//-------------

void OMediaPackedARGBColor::set(const OMediaFARGBColor &c)
{
	fset(c.alpha,c.red,c.green,c.blue);
}


void OMediaPackedARGBColor::get(OMediaFARGBColor &c) const
{
	fget(c.alpha,c.red,c.green,c.blue);
}

void OMediaPackedRGBAColor::set(const OMediaFARGBColor &c)
{
	fset(c.alpha,c.red,c.green,c.blue);
}


void OMediaPackedRGBAColor::get(OMediaFARGBColor &c) const
{
	fget(c.alpha,c.red,c.green,c.blue);
}


void OMediaPackedARGBColor::set(const OMediaARGBColor &c)
{
	set(c.alpha,c.red,c.green,c.blue);
}


void OMediaPackedARGBColor::get(OMediaARGBColor &c) const
{
	get(c.alpha,c.red,c.green,c.blue);
}

void OMediaPackedRGBAColor::set(const OMediaARGBColor &c)
{
	set(c.alpha,c.red,c.green,c.blue);
}


void OMediaPackedRGBAColor::get(OMediaARGBColor &c) const
{
	get(c.alpha,c.red,c.green,c.blue);
}

// Without alpha

void OMediaPackedARGBColor::set(const OMediaFRGBColor &c)
{
	fset(c.red,c.green,c.blue);
}

void OMediaPackedARGBColor::get(OMediaFRGBColor &c) const
{
	fget(c.red,c.green,c.blue);
}

void OMediaPackedRGBAColor::set(const OMediaFRGBColor &c)
{
	fset(c.red,c.green,c.blue);
}

void OMediaPackedRGBAColor::get(OMediaFRGBColor &c) const
{	
	fget(c.red,c.green,c.blue);
}

void OMediaPackedARGBColor::set(const OMediaRGBColor &c)
{
	set(c.red,c.green,c.blue);
}


void OMediaPackedARGBColor::get(OMediaRGBColor &c) const
{
	get(c.red,c.green,c.blue);
}

void OMediaPackedRGBAColor::set(const OMediaRGBColor &c)
{
	set(c.red,c.green,c.blue);
}


void OMediaPackedRGBAColor::get(OMediaRGBColor &c) const
{
	get(c.red,c.green,c.blue);
}




#endif

