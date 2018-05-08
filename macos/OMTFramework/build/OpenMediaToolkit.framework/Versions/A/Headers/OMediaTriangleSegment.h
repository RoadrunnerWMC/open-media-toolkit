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
#ifndef OMEDIA_TriangleSegment_H
#define OMEDIA_TriangleSegment_H

#include "OMediaTypes.h"
#include "OMediaPipePoint.h"
#include "OMediaSegmentRasterizer.h"

#define omd_ZBufferPixScale 0x7FFFFFFFL

class OMediaTriangleSegment
{
	public:
	
	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		y1 = (short)p1->xyzw[1];
		y2 = (short)p2->xyzw[1];

		x	= p1->xyzw[0];
		if (y2!=y1) dx = (p2->xyzw[0]-p1->xyzw[0])/float(y2-y1);
	}
	
	inline void step(void)
	{
		x += dx;
	}
	
	short	y1,y2;
	float	x,dx;
};



class OMediaTriangleZSegment : public OMediaTriangleSegment
{
	public:
	
	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		OMediaTriangleSegment::set(p1,p2);
		
		z = long(p1->xyzw[2]*float(omd_ZBufferPixScale));	
		if (y2!=y1) dz = long((p2->xyzw[2]-p1->xyzw[2])*float(omd_ZBufferPixScale)) / long(y2-y1);
	}
	
	inline void step(void)
	{
		OMediaTriangleSegment::step();
		z += dz;
	}
	
	long z,dz;
};


class OMediaTriangleGouraudSegment : public OMediaTriangleSegment
{
	public:

	static inline void scale_rgb(const OMediaPipePoint	*p, 
								unsigned short &a, unsigned short &r, unsigned short &g, unsigned short &b)
	{
		a	= (unsigned short)((p->a * float(0xFF)));
		r	= (unsigned short)((p->dr * float(0xFF)) 	+ (p->sr * float(0xFF)));
		g	= (unsigned short)((p->dg * float(0xFF)) 	+ (p->sg * float(0xFF)));
		b	= (unsigned short)((p->db * float(0xFF)) 	+ (p->sb * float(0xFF)));
	
		if (a>0xFF) a = 0xFF;
		if (r>0xFF) r = 0xFF;
		if (g>0xFF) g = 0xFF;
		if (b>0xFF) b = 0xFF;		
	}	

	static inline void scale_rgb_spec(const OMediaPipePoint	*p, 
								unsigned short &a, unsigned short &r, unsigned short &g, unsigned short &b,
								unsigned short &sr, unsigned short &sg, unsigned short &sb)
	{
		a	= (unsigned short)((p->a * float(0xFF)));
		r	= (unsigned short)((p->dr * float(0xFF)));
		g	= (unsigned short)((p->dg * float(0xFF)));
		b	= (unsigned short)((p->db * float(0xFF)));
		sr	= (unsigned short)(p->sr * float(0xFF));
		sg	= (unsigned short)(p->sg * float(0xFF));
		sb	= (unsigned short)(p->sb * float(0xFF));
	
		if (a>0xFF) a = 0xFF;
		if (r>0xFF) r = 0xFF;
		if (g>0xFF) g = 0xFF;
		if (b>0xFF) b = 0xFF;		
		if (sr>0xFF) sr = 0xFF;
		if (sg>0xFF) sg = 0xFF;
		if (sb>0xFF) sb = 0xFF;		
	}	

	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		OMediaTriangleSegment::set(p1,p2);
		
		unsigned short	a1,r1,g1,b1,a2,r2,g2,b2;
		long	delta;

		scale_rgb(p1, a1, r1, g1, b1);	// Scale to depth
		scale_rgb(p2, a2, r2, g2, b2);
	
		a = omd_IntToPixInter(a1);
		r = omd_IntToPixInter(r1);
		g = omd_IntToPixInter(g1);
		b = omd_IntToPixInter(b1);
		
		delta = (y2 - y1);
		if (delta) 
		{
			da = omd_IntToPixInter(a2-a1) / delta;
			dr = omd_IntToPixInter(r2-r1) / delta;
			dg = omd_IntToPixInter(g2-g1) / delta;
			db = omd_IntToPixInter(b2-b1) / delta;
		}
	}
	
	inline void step(void)
	{
		OMediaTriangleSegment::step();

		a += da;		
		r += dr;
		g += dg;
		b += db;
	}
	
	omt_PixInterValue	a,r,g,b, da,dr,dg,db;
};


class OMediaTriangleZGouraudSegment : public OMediaTriangleGouraudSegment
{
	public:
	
	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		OMediaTriangleGouraudSegment::set(p1,p2);

		z = long(p1->xyzw[2]*float(omd_ZBufferPixScale));	
		if (y2!=y1) dz = long((p2->xyzw[2]-p1->xyzw[2])*float(omd_ZBufferPixScale)) / long(y2-y1);
	}
	
	inline void step(void)
	{
		OMediaTriangleGouraudSegment::step();
		z += dz;
	}
	
	long	z,dz;
};


#define omd_InvWBits 30L
#define omd_UVBits 	 23L

class OMediaTriangleTextureSegment : public OMediaTriangleSegment
{
	public:
	
	#define omd_FloatToFixedInvW(x) (long((x)*(float(1<<omd_InvWBits))))
	#define omd_FloatToFixedUV(x) 	(long((x)*(float(1<<omd_UVBits))))
	
	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		OMediaTriangleSegment::set(p1,p2);

		long	delta;
		float	u1,v1,u2,v2;
		
		u1 = p1->u;
		v1 = p1->v;
		u2 = p2->u;
		v2 = p2->v;

		inv_w = omd_FloatToFixedInvW(p1->inv_w);
		u = omd_FloatToFixedUV(u1);				// u&v should be in inverse form
		v = omd_FloatToFixedUV(v1);
		
		delta = y2 - y1;
		if (delta) 
		{
			du 		= ((omd_FloatToFixedUV(u2-u1)) 	/ delta);
			dv 		= ((omd_FloatToFixedUV(v2-v1)) 	/ delta);
			dinv_w	= ((omd_FloatToFixedInvW(p2->inv_w-p1->inv_w)) 				/ delta);
		}
	}
	
	inline void step(void)
	{
		OMediaTriangleSegment::step();
		
		u 		+= du;
		v 		+= dv;
		inv_w	+= dinv_w;
	}
	
	long	du,dv,dinv_w,u,v,inv_w;
};

class OMediaTriangleZTextureSegment : public OMediaTriangleTextureSegment
{
	public:
	
	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		OMediaTriangleTextureSegment::set(p1,p2);

		z = long(p1->xyzw[2]*float(omd_ZBufferPixScale));	
		if (y2!=y1) dz = long((p2->xyzw[2]-p1->xyzw[2])*float(omd_ZBufferPixScale)) / long(y2-y1);
	}
	
	inline void step(void)
	{
		OMediaTriangleTextureSegment::step();
		z += dz;
	}
	
	long	z,dz;
};

class OMediaTriangleGouraudTextureSegment : public OMediaTriangleTextureSegment
{
	public:
	
	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		OMediaTriangleTextureSegment::set(p1,p2);

		unsigned short	a1,r1,g1,b1,a2,r2,g2,b2;
		unsigned short	sr1,sg1,sb1,sr2,sg2,sb2;
		long	delta;

		OMediaTriangleGouraudSegment::scale_rgb_spec(p1, a1,r1, g1, b1, sr1, sg1, sb1);	// Scale to depth
		OMediaTriangleGouraudSegment::scale_rgb_spec(p2, a2,r2, g2, b2, sr2, sg2, sb2);
		
		a_light = omd_IntToPixInter(a1);
		r_light = omd_IntToPixInter(r1);
		g_light = omd_IntToPixInter(g1);
		b_light = omd_IntToPixInter(b1);
		r_hilspec = omd_IntToPixInter(sr1);
		g_hilspec = omd_IntToPixInter(sg1);
		b_hilspec = omd_IntToPixInter(sb1);
		
		delta = (y2 - y1);
		if (delta) 
		{
			ia = omd_IntToPixInter(a2-a1) / delta;
			ir = omd_IntToPixInter(r2-r1) / delta;
			ig = omd_IntToPixInter(g2-g1) / delta;
			ib = omd_IntToPixInter(b2-b1) / delta;
			isr = omd_IntToPixInter(sr2-sr1) / delta;
			isg = omd_IntToPixInter(sg2-sg1) / delta;
			isb = omd_IntToPixInter(sb2-sb1) / delta;
		}
	}
	
	inline void step(void)
	{
		OMediaTriangleTextureSegment::step();
		a_light += ia;
		r_light += ir;
		g_light += ig;
		b_light += ib;
		r_hilspec += isr;
		g_hilspec += isg;
		b_hilspec += isb;
	}

	omt_PixInterValue	a_light,r_light,g_light,b_light,r_hilspec,g_hilspec,b_hilspec;
	omt_PixInterValue	ia,ir,ig,ib,isr,isg,isb;

};

class OMediaTriangleZGouraudTextureSegment : public OMediaTriangleGouraudTextureSegment
{
	public:
	
	inline void set(const OMediaPipePoint *p1, const OMediaPipePoint *p2)
	{
		OMediaTriangleGouraudTextureSegment::set(p1,p2);

		z = long(p1->xyzw[2]*float(omd_ZBufferPixScale));	
		if (y2!=y1) dz = long((p2->xyzw[2]-p1->xyzw[2])*float(omd_ZBufferPixScale)) / long(y2-y1);
	}
	
	inline void step(void)
	{
		OMediaTriangleGouraudTextureSegment::step();
		z += dz;
	}
	
	long	z,dz;
};



#endif

