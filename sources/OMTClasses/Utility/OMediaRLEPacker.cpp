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
 

#include "OMediaRLEPacker.h"
#include "OMediaEndianSupport.h"

#define omc_DUMP	0
#define omc_RUN		1

#define MinRun 6	
#define MaxRun 128
#define MaxDat 128 



template <class T>
class OMediaRLEPackTemplate
{
	public:

	T buf[256];
   	T *source, *dest;
   	long putSize;

	inline void PutUnit(T c, T *&dest)
	{
		#ifdef omd_BIG_ENDIAN

		switch(sizeof(T))
		{
			case 2:
			c = omd_ReverseShort(c);
			break;
			
			case 4:
			c = omd_ReverseLong(c);
			break;
		}
		
		#endif
	
		*dest++ = c;
		++putSize;
	}

	inline T GetUnit(void)
	{
		return UGetUnit();
	}

	inline T UGetUnit(void)
	{
		T c = (*source++);
	
		#ifdef omd_BIG_ENDIAN

		switch(sizeof(T))
		{
			case 2:
			c = omd_ReverseShort(c);
			break;
			
			case 4:
			c = omd_ReverseLong(c);
			break;
		}
		
		#endif
	
		return c;
	}

	inline void UPutUnit(T c)
	{
		#ifdef omd_BIG_ENDIAN

		switch(sizeof(T))
		{
			case 2:
			c = omd_ReverseShort(c);
			break;
			
			case 4:
			c = omd_ReverseLong(c);
			break;
		}
		
		#endif
	
		*dest++ = c;
	}


	inline T *PutDump(T *dest, short nn)
	{
		int i;
		PutUnit(nn-1,dest);
		for(i = 0;  i < nn;  i++)   PutUnit(buf[i],dest);
		return(dest);
	}

	inline T *PutRun(T *dest, int nn, int cc)
	{
		PutUnit(-(nn-1),dest);
		PutUnit(cc,dest);
		return(dest);
	}

	inline void OutDump(int nn)   
	{
		dest = PutDump(dest, nn);
	}
	
	inline void OutRun(int nn, int cc) 
	{
		dest = PutRun(dest, nn, cc);
	}

	long rle_pack(T **pSource, T **pDest, short rowSize)
	{
    	T c,lastc = '\0';
    	bool mode = omc_DUMP;
    	short nbuf = 0;	
    	short rstart = 0;

   	 	source = *pSource;
    	dest = *pDest;
    	putSize = 0;
    	buf[0] = lastc = c = GetUnit(); 
    	nbuf = 1;   rowSize--;


    	for (;  rowSize;  --rowSize) 
    	{
			buf[nbuf++] = c = GetUnit();
			switch (mode) 
			{
				case omc_DUMP: 
				if (nbuf>MaxDat) 
				{
					OutDump(nbuf-1);  
					buf[0] = c; 
					nbuf = 1;   rstart = 0; 
					break;
				}

				if (c == lastc) 
				{
				    if (nbuf-rstart >= MinRun) 
			    	{
						if (rstart > 0) OutDump(rstart);
						mode = omc_RUN;
					}
				    else if (rstart == 0) mode = omc_RUN;
				}
				else  rstart = nbuf-1;
				break;

				case omc_RUN: 
		
				if ( (c != lastc) || ( nbuf-rstart > MaxRun)) 
				{
		   			OutRun(nbuf-1-rstart,lastc);
		    		buf[0] = c;
	    			nbuf = 1; rstart = 0;
	    			mode = omc_DUMP;
	    		}
				break;
			}

			lastc = c;
		}

    	switch (mode) 
    	{
			case omc_DUMP: OutDump(nbuf); break;
			case omc_RUN: OutRun(nbuf-rstart,lastc); break;
		}
	
	    *pSource = source;
    	*pDest = dest;
    
    	return(putSize);
	}

	bool rle_unpack(T **pSource, T **pDest, short srcBytes0, short dstBytes0)
	{
    	source = *pSource;
    	dest   = *pDest;
    	short n;
    	T c;
   	 	short srcBytes = srcBytes0, dstBytes = dstBytes0;
    	bool error = true;
    	short minus128 = -128; 


    	while( dstBytes > 0 )  
    	{
			if ( (srcBytes -= 1) < 0 )  goto ErrorExit;
    	
    		n = (short)UGetUnit();

    		if (n >= 0) 
    		{
		    	n += 1;
		    	if ( (srcBytes -= n) < 0 )  goto ErrorExit;
		    	if ( (dstBytes -= n) < 0 )  goto ErrorExit;
		    	do {  UPutUnit(UGetUnit());  } while (--n > 0);
	    	}

	    	else if (n != minus128) 
    		{
			    n = -n + 1;
		    	if ( (srcBytes -= 1) < 0 )  goto ErrorExit;
		    	if ( (dstBytes -= n) < 0 )  goto ErrorExit;
		    	c = UGetUnit();
		    	do {  UPutUnit(c);  } while (--n > 0);
	    	}
		}
	
    	error = false;

  	ErrorExit:
    	*pSource = source;  *pDest = dest;
    	return(!error);
	}
};

//.................................................................

	
long OMediaRLEPacker::rle_max_pack_size(short s, omt_RLEPackMode pmode)
{
	switch(pmode)
	{
		case omrlec_8Bits:
		return s + ((s+127) >>7);
		break;

		case omrlec_16Bits:
		return s + (((s+127) >>7)<<1);
		break;

		case omrlec_32Bits:
		return s + (((s+127) >>7)<<2);
		break;	
	}
	
	
	return 0;
}	

long OMediaRLEPacker::rle_pack(char **pSource, char **pDest, short source_len, omt_RLEPackMode pmode)
{
	long	res;

	switch(pmode)
	{
		case omrlec_8Bits:
		{
			OMediaRLEPackTemplate<char> packer;
			res = packer.rle_pack(pSource,pDest,source_len);
		}
		break;
	
		case omrlec_16Bits:
		{
			OMediaRLEPackTemplate<short> packer;
			short		*source,*dest;
	
			source = (short*) *pSource;
			dest = (short*) *pDest;
						
			res = packer.rle_pack(&source,&dest,source_len>>1L);

			res<<=1L;
			*pSource = (char*)source;
			*pDest = (char*)dest;
		}
		break;

		case omrlec_32Bits:
		{
			OMediaRLEPackTemplate<long> packer;
			long		*source,*dest;

			source = (long*) *pSource;
			dest = (long*) *pDest;	
			res = packer.rle_pack(&source,&dest,source_len>>2L);
						
			res<<=2L;
			*pSource = (char*)source;
			*pDest = (char*)dest;
		}
		break;	
		
		default:
		res = 0;
	}
	
	return res;
}

bool OMediaRLEPacker::rle_unpack(char **pSource, char **pDest, short source_len, short dest_len, omt_RLEPackMode pmode)
{
	bool	res;

	switch(pmode)
	{
		case omrlec_8Bits:
		{
			OMediaRLEPackTemplate<char> packer;
			res = packer.rle_unpack(pSource,pDest,source_len,dest_len);
		}
		break;
	
		case omrlec_16Bits:
		{
			OMediaRLEPackTemplate<short> packer;
			short		*source,*dest;
	
			source = (short*) *pSource;
			dest = (short*) *pDest;
			res = packer.rle_unpack(&source,&dest,source_len>>1L,dest_len>>1L);
			*pSource = (char*)source;
			*pDest = (char*)dest;
		}
		break;

		case omrlec_32Bits:
		{
			OMediaRLEPackTemplate<long> packer;
			long		*source,*dest;
	
			source = (long*) *pSource;
			dest = (long*) *pDest;
			res = packer.rle_unpack(&source,&dest,source_len>>2L,dest_len>>2L);
			*pSource = (char*)source;
			*pDest = (char*)dest;
		}
		break;	
		
		default:
		res = false;
	}
	
	return res;

}


