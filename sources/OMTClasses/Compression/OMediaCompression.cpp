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
 
#include "OMediaCompression.h"

#include "zlib.h"

// This code use the ZLib by Jean-loup Gailly and Mark Adler

void OMediaCompression::compress(void *source, long source_len, void *dest, long &dest_len,  omt_CompressionLevel level)
{
    z_stream stream;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    stream.next_in = (unsigned char*)source;
    stream.avail_in = (unsigned long)source_len;
    stream.next_out = (unsigned char*)dest;
    stream.avail_out = dest_len;

    deflateInit(&stream, level);
    deflate(&stream, Z_FINISH);
    dest_len = stream.total_out;
    deflateEnd(&stream);
}

void OMediaCompression::uncompress (void *source, long source_len, void *dest, long &dest_len)
{

    z_stream stream;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    stream.next_in = (unsigned char*)source;
    stream.avail_in = (unsigned long)source_len;
    stream.next_out = (unsigned char*)dest;
    stream.avail_out = dest_len;

    inflateInit(&stream);
    inflate(&stream, Z_FINISH);
    dest_len = stream.total_out;
    inflateEnd(&stream);
}


//-----------------------------------------------------------------

unsigned long OMediaCompression::adler32(unsigned long adler, const unsigned char *buf, unsigned long len)
{
    return adler32(adler, buf, len);
}
