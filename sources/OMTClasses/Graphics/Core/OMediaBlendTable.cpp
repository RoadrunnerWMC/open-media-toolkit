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
 
 #include "OMediaBlendTable.h"
 

#define omd_BLEND_PIX2FLOAT(p,i) (((float)((unsigned char *)(p))[(i)]) * (1.0f/255.0f))
#define omd_BLEND_FLOAT2PIX(v) (unsigned char)((v)*255.0f)

#define omd_BLEND_R	0
#define omd_BLEND_G	1
#define omd_BLEND_B	2
#define omd_BLEND_A	3

void OMediaBlendTable::omf_bfunc_Zero(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	rgba_res[omd_BLEND_R]=rgba_res[omd_BLEND_G]=rgba_res[omd_BLEND_B]=rgba_res[omd_BLEND_A]=0;
}

void OMediaBlendTable::omf_bfunc_One(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	rgba_res[omd_BLEND_R] = ((unsigned char *)rgba)[omd_BLEND_R];
	rgba_res[omd_BLEND_G] = ((unsigned char *)rgba)[omd_BLEND_G];
	rgba_res[omd_BLEND_B] = ((unsigned char *)rgba)[omd_BLEND_B];
	rgba_res[omd_BLEND_A] = ((unsigned char *)rgba)[omd_BLEND_A];
}

void OMediaBlendTable::omf_bfunc_Dst_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*d);
}

void OMediaBlendTable::omf_bfunc_Src_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*d);
}

void OMediaBlendTable::omf_bfunc_Inv_Dst_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));
}

void OMediaBlendTable::omf_bfunc_Inv_Src_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));
}

void OMediaBlendTable::omf_bfunc_Src_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(a));
}

void OMediaBlendTable::omf_bfunc_Inv_Src_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));
}

void OMediaBlendTable::omf_bfunc_Dst_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(a));
}

void OMediaBlendTable::omf_bfunc_Inv_Dst_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));
}

void OMediaBlendTable::omf_bfunc_Src_Alpha_Saturate(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float as,ad,d,s;

	as = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	ad = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	d = min(as, 1.0f - ad);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*d);

	rgba_res[omd_BLEND_A] = 0xFF;
}


omt_BlendFunctionPtr OMediaBlendTable::find_blend_func(omt_BlendFunc f)
{
	static omt_BlendFunctionPtr	func_table[] =
	{
		omf_bfunc_Zero,
		omf_bfunc_One,
		omf_bfunc_Dst_Color,
		omf_bfunc_Src_Color,
		omf_bfunc_Inv_Dst_Color,
		omf_bfunc_Inv_Src_Color,
		omf_bfunc_Src_Alpha,
		omf_bfunc_Inv_Src_Alpha,
		omf_bfunc_Dst_Alpha,
		omf_bfunc_Inv_Dst_Alpha,
		omf_bfunc_Src_Alpha_Saturate	
	};

	return func_table[long(f)];
}

//---------------------------------------------------------------------------------------

#undef omd_BLEND_R
#undef omd_BLEND_G
#undef omd_BLEND_B
#undef omd_BLEND_A

#define omd_BLEND_A	0
#define omd_BLEND_R	1
#define omd_BLEND_G	2
#define omd_BLEND_B	3

void OMediaBlendTable::omf_bfunc_argb_Zero(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	rgba_res[omd_BLEND_R]=rgba_res[omd_BLEND_G]=rgba_res[omd_BLEND_B]=rgba_res[omd_BLEND_A]=0;
}

void OMediaBlendTable::omf_bfunc_argb_One(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	rgba_res[omd_BLEND_R] = ((unsigned char *)rgba)[omd_BLEND_R];
	rgba_res[omd_BLEND_G] = ((unsigned char *)rgba)[omd_BLEND_G];
	rgba_res[omd_BLEND_B] = ((unsigned char *)rgba)[omd_BLEND_B];
	rgba_res[omd_BLEND_A] = ((unsigned char *)rgba)[omd_BLEND_A];
}

void OMediaBlendTable::omf_bfunc_argb_Dst_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*d);
}

void OMediaBlendTable::omf_bfunc_argb_Src_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*d);
}

void OMediaBlendTable::omf_bfunc_argb_Inv_Dst_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));
}

void OMediaBlendTable::omf_bfunc_argb_Inv_Src_Color(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,d;
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);	d = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-d));
}

void OMediaBlendTable::omf_bfunc_argb_Src_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(a));
}

void OMediaBlendTable::omf_bfunc_argb_Inv_Src_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));
}

void OMediaBlendTable::omf_bfunc_argb_Dst_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(a));
}

void OMediaBlendTable::omf_bfunc_argb_Inv_Dst_Alpha(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float s,a;
	
	a = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	
	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_A);
	rgba_res[omd_BLEND_A] = omd_BLEND_FLOAT2PIX(s*(1.0f-a));
}

void OMediaBlendTable::omf_bfunc_argb_Src_Alpha_Saturate(omt_RGBAPixel *rgba, omt_RGBAPixel *src_rgba, omt_RGBAPixel *dest_rgba, short *rgba_res)
{
	float as,ad,d,s;

	as = omd_BLEND_PIX2FLOAT(src_rgba,omd_BLEND_A);
	ad = omd_BLEND_PIX2FLOAT(dest_rgba,omd_BLEND_A);
	d = min(as, 1.0f - ad);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_R);
	rgba_res[omd_BLEND_R] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_G);
	rgba_res[omd_BLEND_G] = omd_BLEND_FLOAT2PIX(s*d);

	s = omd_BLEND_PIX2FLOAT(rgba,omd_BLEND_B);
	rgba_res[omd_BLEND_B] = omd_BLEND_FLOAT2PIX(s*d);

	rgba_res[omd_BLEND_A] = 0xFF;
}


omt_BlendFunctionPtr OMediaBlendTable::find_blend_func_argb(omt_BlendFunc f)
{
	static omt_BlendFunctionPtr	func_table[] =
	{
		omf_bfunc_argb_Zero,
		omf_bfunc_argb_One,
		omf_bfunc_argb_Dst_Color,
		omf_bfunc_argb_Src_Color,
		omf_bfunc_argb_Inv_Dst_Color,
		omf_bfunc_argb_Inv_Src_Color,
		omf_bfunc_argb_Src_Alpha,
		omf_bfunc_argb_Inv_Src_Alpha,
		omf_bfunc_argb_Dst_Alpha,
		omf_bfunc_argb_Inv_Dst_Alpha,
		omf_bfunc_argb_Src_Alpha_Saturate	
	};

	return func_table[long(f)];
}


//---------------------------------------------------------------------------------------


