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


#include "OMedia3DShapeConverter.h"
#include "OMedia3DShape.h"
#include "OMediaError.h"
#include "OMediaDataBase.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DMorphAnim.h"
#include "OMedia3DMorphAnimFrame.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"

#include <vector>
#include <algorithm>

omt_3DMaterialConvertMode OMedia3DShapeConverter::mat_conv_mode 
									= omc3dmconv_CreateNewMaterialIfNotFound;

omt_3DObjectConvertMode OMedia3DShapeConverter::obj_conv_mode = omc3doconv_MergeObjects;

omt_3DConvertFlags OMedia3DShapeConverter::conv_flags = 0;

omt_3DAnimConvertMode OMedia3DShapeConverter::anim_conv_mode = omc3danconv_Ignore;

omt_TextureConvertMode OMedia3DShapeConverter::texture_conv_mode = omctxtconv_Ignore;
omt_WorldConvertMode   OMedia3DShapeConverter::world_conv_mode = omcworldconv_None;


bool OMedia3DShapeConverter::converter_used;


string OMedia3DShapeConverter::new_material_prefix;
string OMedia3DShapeConverter::new_texture_prefix;



vector<omt_FSChunkID>				OMedia3DShapeConverter::related_worldelements;
vector<omt_FSChunkID>				OMedia3DShapeConverter::related_materials;
vector<OMedia3DShapeConvMorphDef>	OMedia3DShapeConverter::related_morphdefs;
vector<OMedia3DShapeConvTFileName>	OMedia3DShapeConverter::related_texturefile;
vector<omt_FSChunkID>				OMedia3DShapeConverter::related_textures;
vector<omt_FSChunkID>				OMedia3DShapeConverter::created_shapes;


OMedia3DShapeConverter::OMedia3DShapeConverter(OMediaStreamOperators *stream, OMedia3DShape *shape)
{
	this->stream = stream;
	this->shape = shape;
	anim_def = NULL;
	morph_frame = NULL; 

	related_worldelements.erase(related_worldelements.begin(),related_worldelements.end());
	related_materials.erase(related_materials.begin(),related_materials.end());
	related_morphdefs.erase(related_morphdefs.begin(),related_morphdefs.end());
	related_texturefile.erase(related_texturefile.begin(),related_texturefile.end());
	related_textures.erase(related_textures.begin(),related_textures.end());
	created_shapes.erase(created_shapes.begin(),created_shapes.end());
}


OMedia3DShapeConverter::~OMedia3DShapeConverter()
{
	if (anim_def) anim_def->db_unlock();

	if (conv_flags&(om3dcf_MergeIdenticalMaterials|om3dcf_OneMaterialPerTexture))
		merge_identical_materials(shape,conv_flags);

	import_textures();
	generate_world();
}
	
OMedia3DShapeConverter *OMedia3DShapeConverter::create_best_converter(	OMediaStreamOperators *stream,
																		OMedia3DShape *shape)
{
	#define ase_3dsmax "*3DSMAX_ASCIIEXPORT"
	#define xsi_softimage "xsi"

	short ase_3dsmax_len = strlen(ase_3dsmax);
	short xsi_softimage_len = strlen(xsi_softimage);
	char buffer[64];

	if ((long)stream->getsize()>=ase_3dsmax_len)
	{
		stream->read(buffer,ase_3dsmax_len);
		stream->setposition(-ase_3dsmax_len, omcfr_Current);
		
		buffer[ase_3dsmax_len] = 0;
		if (strcmp(buffer,ase_3dsmax)==0)
		{
			return new OMediaASEConverter(stream,shape);
		}		
	}

	if ((long)stream->getsize()>=xsi_softimage_len)
	{
		stream->read(buffer,xsi_softimage_len);
		stream->setposition(-xsi_softimage_len, omcfr_Current);
		
		buffer[xsi_softimage_len] = 0;
		if (strcmp(buffer,xsi_softimage)==0)
		{
			return new OMediaXSIConverter(stream,shape);
		}		
	}

	return new OMediaDXFConverter(stream,shape);
}


void OMedia3DShapeConverter::convert(void) {}

void OMedia3DShapeConverter::finalize_shape(void) 
{
	if (conv_flags&(om3dcf_MergeIdenticalMaterials|om3dcf_OneMaterialPerTexture))
		merge_identical_materials(shape,conv_flags);

	created_shapes.push_back(shape->get_chunk_ID());
}

bool OMedia3DShapeConverter::start_new_shape(string name, bool finalize)
{
	OMediaDataBase	*dbase;
	omt_ChunkType	type;
	omt_FSChunkID	id;

	dbase = shape->get_database();
	if (!dbase) return false;

	// Unlock old shape
	if (finalize) finalize_shape();
	shape->get_chunk_info(type, id);
	if (finalize) shape->db_unlock();

	id = dbase->get_unique_chunkid(type, id);
	shape = (OMedia3DShape*)dbase->get_object(type,id);	
	
	shape->set_chunk_name(name);
	shape->db_unlock();

	// Unlock anim. definition

	if (anim_def)
	{
		anim_def->db_unlock();
		anim_def = NULL;
	}
	
	return true;
}


void OMedia3DShapeConverter::merge_identical_materials(OMedia3DShape	*shape,
													   omt_3DConvertFlags conv_flags)
{
	vector<omt_FSChunkID>::iterator	mi,mni; 
	OMediaDataBase	*dbase;

	dbase = shape->get_database();

	if (!dbase || related_materials.size()<2) return;

	OMedia3DMaterial	*mat,*mat2;

	shape->lock(omlf_Read|omlf_Write);

	for(mi = related_materials.begin();
		mi!=related_materials.end()-2;
		mi++)
	{
		mat = omd_GETOBJECT(dbase,OMedia3DMaterial,(*mi));

		for(mni = mi+1;
			mni!=related_materials.end();
			mni++)
		{
			mat2 = omd_GETOBJECT(dbase,OMedia3DMaterial,(*mni));
			if ( ( (conv_flags&om3dcf_MergeIdenticalMaterials) &&  (*mat)==(*mat2) ) ||				
				((conv_flags&om3dcf_OneMaterialPerTexture) &&
				 mat->get_texture()!=NULL &&
				 mat->get_texture()==mat2->get_texture()) )
			{
				for(omt_PolygonList::iterator	pi =shape->get_polygons()->begin();
					pi!=shape->get_polygons()->end();
					pi++)
				{
					if ((*pi).get_material()==mat2) (*pi).set_material(mat);
				}
			}

			mat2->db_unlock();
		}

		mat->db_unlock();
	}

	shape->unlock();
}

class OMediaImportShapeCanvasPtr
{
public:

	OMediaImportShapeCanvasPtr() {canvas = NULL;imported=false;}

	OMediaCanvas	*canvas;
	bool			imported;
};

string OMedia3DShapeConverter::texture_prefix(string filename)
{
	string str = new_texture_prefix;
	str += filename;
	return str;
}

void OMedia3DShapeConverter::generate_texture_filenames(string path, 
														string &filename, 
														string &filename_noextend)
{
	string str;

	// Extract the file name
	
	str = path;
	short i;
	i = str.length()-1;
	for(;;)
	{
		if (str[i]=='\\' ||
			str[i]=='/' ||
			str[i]==':')
		{
			path = str.c_str()+i+1;
			break;						
		}
		
		if (i==0) break;
		i--;					
	}

	filename = path;

	// Remove extension

	if (path.length()>=5 && path[path.length()-4]=='.')
		path.resize(path.length()-4);		
	
	filename_noextend = path;
	
}

void OMedia3DShapeConverter::import_textures()
{
	OMediaDataBase			*db = shape->get_database();
	OMediaCanvas			*canv = NULL,*dbText;
	typedef	less<string>	less_string;
	string					filesnames[3],raw_filesnames[3];
	int						i;
	bool					texture_found;
	
	unsigned long			storeDBCacheSize;
	
	map<string,OMediaImportShapeCanvasPtr,less_string>	name2text;

	if (texture_conv_mode==omctxtconv_Ignore || db==NULL) return;

	storeDBCacheSize = OMediaDataBase::get_cache_size();	// No purge during texture import
	OMediaDataBase::set_cache_size(0);

	for(vector<OMedia3DShapeConvTFileName>::iterator ti=related_texturefile.begin();
		ti!=related_texturefile.end();
		ti++)
	{
		if ((*ti).texture_filename.size()==0) continue;

		raw_filesnames[0] = (*ti).texture_filename;

		generate_texture_filenames(raw_filesnames[0], raw_filesnames[1], raw_filesnames[2]);
		
		for(i=0;i<3;i++) filesnames[i] = texture_prefix(raw_filesnames[i]);
		
		texture_found = false;
		dbText = NULL;
	
		// Check if there is a database version
		
		for(i=0;i<3;i++)
		{
			if (db->object_exists(OMediaCanvas::db_type, filesnames[i]))
			{
				OMedia3DMaterial *tmat = omd_GETOBJECT(db,OMedia3DMaterial,(*ti).material_id);
				dbText = canv = (OMediaCanvas *) db->get_object(OMediaCanvas::db_type, filesnames[i]);

				if (texture_conv_mode==omctxtconv_ImportIfNotFound) 
				{
					tmat->set_modified();
					tmat->set_texture(canv);
					texture_found = true;
				}
				
				name2text[filesnames[i]].canvas = canv; // DB purge is disabled, I can store pointers
				
				canv->db_unlock();
				tmat->db_unlock();
				
				break;
			}
		}
			
		// Okay, let's try to import
		
		if (!texture_found)
		{
			canv = NULL;
		
			// See if already generated
			
			for(i=0;i<3;i++)
			{
				canv = name2text[filesnames[i]].canvas;			
				if (canv) break;
			}
			
			if (canv==NULL)
			{
				bool				fileFound = false;
				OMediaFilePath		path,streamPath;
				OMediaFileStream	file;
			
				// Okay, let's try to load it.

				path.setpath(raw_filesnames[0]);
				file.setpath(&path);
				fileFound = file.fileexists();
				
				if (!fileFound)
				{
					if (stream->getpath(&streamPath))
					{
						path.setpath(raw_filesnames[1],&streamPath);
						file.setpath(&path);
						fileFound = file.fileexists();
						if (!fileFound)
						{
							string	fext;
						
							// Try with other file extensions						
							
							fext = raw_filesnames[2];	// PNG
							fext += ".png";

							path.setpath(fext,&streamPath);
							file.setpath(&path);
							fileFound = file.fileexists();
							if (!fileFound)
							{
								fext = raw_filesnames[2];	// GIF
								fext += ".gif";

								path.setpath(fext,&streamPath);
								file.setpath(&path);
								fileFound = file.fileexists();
							}						
						}
					}				
				}
				
				if (fileFound)
				{
					try
					{
						if (texture_conv_mode==omctxtconv_Create || dbText==NULL)
						{
							canv = omd_GETOBJECT(db,OMediaCanvas,db->get_unique_chunkid(OMediaCanvas::db_type));
							canv->db_unlock();							
						}
						else 
							canv = dbText;
					
						file.open(omcfp_Read,false,false);
						file>>canv;
						file.close();
						
						name2text[filesnames[2]].canvas = canv;
						name2text[filesnames[2]].imported = true;
						canv->set_chunk_name(filesnames[2]);
						canv->set_modified();
		
						OMedia3DMaterial *tmat = omd_GETOBJECT(db,OMedia3DMaterial,(*ti).material_id);
						tmat->set_texture(canv);
						tmat->set_modified();
						tmat->db_unlock();
					}
					catch(OMediaError err) {}
				}					
			}
		}	
	}
	
	// Generate related textures list
	
	for(map<string,OMediaImportShapeCanvasPtr,less_string>::iterator mi=name2text.begin();
		mi!=name2text.end();
		mi++)
	{
		if ((*mi).second.imported && (*mi).second.canvas)
			related_textures.push_back((*mi).second.canvas->get_chunk_ID());
	}
	
	OMediaDataBase::set_cache_size(storeDBCacheSize);
}

void OMedia3DShapeConverter::generate_world(void)
{
	OMedia3DMorphAnim		*morphE;
	OMedia3DShapeElement	*shapeE;
	OMediaElement			*rootE;
	OMedia3DShape			*cshape;
	OMediaDataBase			*db = shape->get_database();
	string					name;
	vector<OMedia3DShapeConvMorphDef>::iterator ai;
	vector<omt_FSChunkID>::iterator si;
	bool					isAnim;

	if (world_conv_mode ==omcworldconv_None) return;
	
	rootE = omd_GETOBJECT(db,OMediaElement, db->get_unique_chunkid(OMediaElement::db_type));
	shape->get_chunk_name(name);
	rootE->set_chunk_name(name);
	rootE->set_descriptor(name + "(root)");

	related_worldelements.push_back(rootE->get_chunk_ID());
	
	// Now for each shape I create a sub-element
	
	for(si=created_shapes.begin();
		si!=created_shapes.end();
		si++)
	{
		cshape = omd_GETOBJECT(db,OMedia3DShape, (*si));
	
		// Should I create a morph anim or a simple shape element ? 
                
                isAnim = false;
		
		for(ai=related_morphdefs.begin();
			ai!=related_morphdefs.end();
			ai++)
		{
			if ((*ai).its_shape==(*si)) 
			{
				isAnim = true;
				break;
			}
		}
		
		if (isAnim) { shapeE = morphE = new OMedia3DMorphAnim;}
		else shapeE = new OMedia3DShapeElement;
		
		// First take care of the shape:
		
		shapeE->link(rootE);
		shapeE->set_shape(cshape);
		cshape->get_chunk_name(name);
		shapeE->set_descriptor(name);
		
		// Now take care of animation:
		
		if (isAnim)
		{
			OMedia3DMorphAnimDef *morph_def;
			
			morph_def = omd_GETOBJECT(db,OMedia3DMorphAnimDef, (*ai).morph_def);
			morphE->set_anim_def(morph_def);
			morph_def->db_unlock();
		}		
		
		cshape->db_unlock();	
	}
	
	rootE->set_modified();
	rootE->db_unlock();
}

	
// * DXF converter


OMediaDXFConverter::OMediaDXFConverter(OMediaStreamOperators *stream, OMedia3DShape *shape):			
					OMedia3DShapeConverter(stream,shape)

{

}


OMediaDXFConverter::~OMediaDXFConverter() {}

void OMediaDXFConverter::convert(void)
{
	char	*buffer;

	shape->db_lock();
	
	buffer = new char[stream->getsize()];	// Read the DXF file into a buffer
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);
	stream->read(buffer,stream->getsize());
	scan_dxf(buffer,buffer+stream->getsize());
	delete buffer;

	finalize_shape();

	shape->db_unlock();
}

void OMediaDXFConverter::finalize_shape(void) 
{
	shape->prepare(ompfc_PrepareAll&(~ompfc_ComputeVertexNormals));
	shape->compute_normals(0.3f);
	shape->delete_unused_vertices();	
	if ((conv_flags&om3dcf_MergeNormals) && anim_conv_mode==omc3danconv_Ignore) shape->merge_normals(false);
	shape->set_modified();

	OMedia3DShapeConverter::finalize_shape();
}


void OMediaDXFConverter::skip_line(char *&b) {while(*b++!=0xD && b<dxf_end){} if (*b==0x0A) b++;}

void OMediaDXFConverter::skip_space(char *&b) {while(*b==' ' && b<dxf_end) b++;}

void OMediaDXFConverter::get_line(char *&b, char *line)
{
	skip_space(b);
	for(;(*b!=' ' && *b!=0xD && b<dxf_end);b++,line++) *line = *b;

	*line = 0;
	if (*b!=0xD) skip_line(b);
	else b++;

	if (*b==0x0A) b++;
}



void OMediaDXFConverter::scan_dxf(char *dxf, char *dxf_end)
{
	char	line[256];
	char	*olddxf = dxf;
	vector<long>			index_base_list;
	vector<OMedia3DShape*>	shapes_list;
	long	p;
	OMedia3DShape			*base_shape = shape;

	this->dxf_end = dxf_end;

	// First pass for "3DFACE" entries and to store "VERTEX" entries.
	for(;;)
	{
		get_line(dxf,line);
		if (strcmp(line,"3DFACE")==0) scan_3dface(dxf,line);
		else if (strcmp(line,"POLYLINE")==0) 
		{
			skip_line(dxf);
			get_line(dxf,line);

			if (obj_conv_mode!=omc3doconv_MergeObjects)
			{
				if (index_base_list.size()!=0)
				{
					if (start_new_shape(line,false))
					{
						shape->set_chunk_name(line);
						shapes_list.push_back(shape);
						shape->db_lock();
						index_base_list.push_back(0);
					}
					else 
					{
						shapes_list.push_back(NULL);	// Cannot add shape, merge instead
						index_base_list.push_back(shape->get_vertices()->size());
					}
				}
				else
				{
					shape->set_chunk_name(line);
					shapes_list.push_back(shape);
					shape->db_lock();
					index_base_list.push_back(0);
				}			
			}
			else 
			{
				if (index_base_list.size()==0) shape->set_chunk_name(line);		
				index_base_list.push_back(shape->get_vertices()->size());
			}
		}
		else if (strcmp(line,"VERTEX")==0) scan_3dvertex(dxf,line);
		else if (strcmp(line,"EOF")==0) break;

		if (dxf>=dxf_end) omd_EXCEPTION(omcerr_BadDXFFormat);
	}

	// Second pass to build faces with the "VERTEX" entries stored before.
	dxf = olddxf;
	dxf_index_base = 0;
	p=0;

	for(;;)
	{
		get_line(dxf,line);
		if (strcmp(line,"VERTEX")==0) scan_3dvertexface(dxf,line);
		else if (strcmp(line,"POLYLINE")==0) 
		{			
			dxf_index_base = index_base_list[p];
			if (obj_conv_mode!=omc3doconv_MergeObjects) shape = shapes_list[p];
			p++;
		}
		else if (strcmp(line,"EOF")==0) break;

		if (dxf>=dxf_end) omd_EXCEPTION(omcerr_BadDXFFormat);
	}
	
	// Unlock all shapes
	
	for(vector<OMedia3DShape*>::iterator si=shapes_list.begin();
		si!=shapes_list.end();
		si++)
	{
		shape = (*si);
		if (shape)
		{
			if (shape!=base_shape) finalize_shape();
			shape->db_unlock();
		}
	}
	
	shape = base_shape;
}




void OMediaDXFConverter::scan_3dface(char *&dxf, char *line)
{
	long				code;
	OMedia3DPoint		point;
	OMedia3DPolygon		poly;
	long				last_index =-1,index;

	skip_line(dxf);	// '8'
	skip_line(dxf);	// title
	
	for(;;)
	{
		get_line(dxf,line);
	
		sscanf(line,"%ld",&code);
		if (code==0) break;

		get_line(dxf,line);
		sscanf(line,"%f",&point.x);

		skip_line(dxf);
		get_line(dxf,line);
		sscanf(line,"%f",&point.y);

		skip_line(dxf);
		get_line(dxf,line);
		sscanf(line,"%f",&point.z);
		
		point.y = - point.y;

		index = shape->add_vertex(point);
		if (last_index==-1 || index!=last_index)
		{
			OMedia3DPolygonVertex	pv;
			
			pv.vertex_index = index;
			pv.normal_index = 0;
			pv.color_index = 0;
			pv.u = pv.v  = 0.0f;

			poly.get_vertices().push_back(pv);
		}
		
		last_index = index;
	}
	
	shape->get_polygons()->push_back(poly);
}


void OMediaDXFConverter::scan_3dvertex(char *&dxf, char *line)
{
	long				code;
	OMedia3DPoint		point;

	skip_line(dxf);		// '8'
	skip_line(dxf);		// title
	
	skip_line(dxf);		// '10'
	get_line(dxf,line);
	sscanf(line,"%f",&point.x);

	skip_line(dxf);		// '20'
	get_line(dxf,line);
	sscanf(line,"%f",&point.y);

	skip_line(dxf);		// '30'
	get_line(dxf,line);
	sscanf(line,"%f",&point.z);

	skip_line(dxf);		// '70'
	get_line(dxf,line);
	sscanf(line,"%ld",&code);

	point.y = - point.y;
		
	if (code==192)
	{
		// This is a vertex value, not a face information, it's OK. We can store it.
		// We store ALL the vertices, since the polygons rely on the indexes of
		// the vertices. By the way, all the vertices should be different.
		shape->get_vertices()->push_back(point);
	}

}



void OMediaDXFConverter::scan_3dvertexface(char *&dxf, char *line)
{
	long				code;
	OMedia3DPoint		point;

	skip_line(dxf);		// '8'
	skip_line(dxf);		// title
	
	skip_line(dxf);		// '10'
	get_line(dxf,line);
	sscanf(line,"%f",&point.x);

	skip_line(dxf);		// '20'
	get_line(dxf,line);
	sscanf(line,"%f",&point.y);

	skip_line(dxf);		// '30'
	get_line(dxf,line);
	sscanf(line,"%f",&point.z);

	skip_line(dxf);		// '70'
	get_line(dxf,line);
	sscanf(line,"%ld",&code);
		
	if (code==128)
	{
		// This is a face value, not a vertex, it's OK. We can build the polygon.
		long					index1;
		long					index2;
		long					index3;
		OMedia3DPolygon			poly;

		// We read the indexes of the vertices used in this face.
		skip_line(dxf);		// '71'
		get_line(dxf,line);
		sscanf(line,"%ld",&index1);
		if (index1 < 0) index1 = -index1;

		skip_line(dxf);		// '72'
		get_line(dxf,line);
		sscanf(line,"%ld",&index2);
		if (index2 < 0) index2 = -index2;

		skip_line(dxf);		// '73'
		get_line(dxf,line);
		sscanf(line,"%ld",&index3);
		if (index3 < 0) index3 = -index3;

		if (!(index1==index2 ||					// Reject lines
			  index1==index3 ||
			  index2==index3))
		{
			// We store the polygon.
			poly.set_triangle();
			poly.set_point(0,dxf_index_base + (index1-1));
			poly.set_point(1,dxf_index_base + (index2-1));
			poly.set_point(2,dxf_index_base + (index3-1));


			// Check if polygon is valid:

			if (poly.get_point(0)>=0 && poly.get_point(0)<shape->get_vertices()->size() &&
				poly.get_point(1)>=0 && poly.get_point(1)<shape->get_vertices()->size() &&
				poly.get_point(2)>=0 && poly.get_point(2)<shape->get_vertices()->size())
			{
				OMedia3DPoint	&p1 = (*shape->get_vertices())[poly.get_point(0)];
				OMedia3DPoint	&p2 = (*shape->get_vertices())[poly.get_point(1)];
				OMedia3DPoint	&p3 = (*shape->get_vertices())[poly.get_point(2)];

				if (!(p1==p2 || p1==p3 || p2==p3)) // Reject lines
				{
					shape->get_polygons()->push_back(poly);	// Alright, this polygon is valid.
				}
			}
		}
	}
}


// * 3DSMax ASE converter

OMediaASEConverter::OMediaASEConverter(OMediaStreamOperators *stream, OMedia3DShape *shape):
					OMedia3DShapeConverter(stream,shape)
{
	face_uvw_computed = false;
	first_shape = true;
	group_level = 0;
	vertex_color_mode = true;
	transform_matrix.set_identity();
	inv_transform_matrix.set_identity();
}

OMediaASEConverter::~OMediaASEConverter()
{
	unlock_materials(vertex_color_mode);
}

void OMediaASEConverter::convert(void)
{
	shape->db_lock();

	buffer = new char[stream->getsize()];	// Read the ASE file into a buffer
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);

	buffer_end = buffer + stream->getsize();
	
	stream->read(buffer,stream->getsize());

	try
	{	
		scan_ase();
	}
	catch(...) {}

	delete buffer;

	shape->db_unlock();
}

bool OMediaASEConverter::get_next_code(string &cmd, bool &block)
{
	// Search for code start

	for(;;)
	{
		if ( (*ptr)=='*') break;
		else if ( (*ptr)=='"') skip_string();
		else if ( (*ptr)=='}') {advance_ptr(); return false;}
		
		advance_ptr();	
	}

	advance_ptr();
	
	char *cmd_start = ptr;

	for(;;)
	{
		if ( (*ptr) == ' ' || (*ptr)=='\t' )
		{
			char t;
		
			t = (*ptr);
			(*ptr) = 0;
			cmd = cmd_start;
			(*ptr) = t;
			break;
		}
		else if ( (*ptr)=='}') return false;
		
		advance_ptr();
	}
	
	advance_ptr();
	
	block = (*ptr)=='{';

	if ( (*ptr)=='}') {advance_ptr(); return false;}	
	return true;
}

void OMediaASEConverter::get_string(string &str)
{
	move_to_char('"');
	advance_ptr();
	
	char	*str_start;
	
	str_start = ptr;

	move_to_char('"');
	
	(*ptr) = 0;
	str = str_start;
	(*ptr) = '"';	

	advance_ptr();
}

void OMediaASEConverter::get_keyword(string &str)
{
	while((*ptr)==' ' || (*ptr)=='\t' || (*ptr)=='\r' || (*ptr)=='\n') advance_ptr();
	char	*str_start;
	
	str_start = ptr;

	while(!((*ptr)==' ' || (*ptr)=='\t' || (*ptr)=='\r' || (*ptr)=='\n')) advance_ptr();

	char t;
	
	t = (*ptr);
	(*ptr) = 0;
	str = str_start;
	(*ptr) = t;	
}

void OMediaASEConverter::get_number(string &nstr)
{
	char	*n_start;
	char	t;

	for(;;)
	{
		if ( (*ptr)=='.' || (*ptr)=='-') break;
		if ( (*ptr)>='0' && (*ptr) <='9' ) break;

		advance_ptr();
	}

	n_start = ptr;
	advance_ptr();

	for(;;)
	{
		if ( ((*ptr)!='.' && (*ptr)!='-') && ((*ptr)<'0' || (*ptr) >'9')) break;
		advance_ptr();
	}
	
	t = (*ptr);
	(*ptr) = 0;
	nstr = n_start;
	(*ptr) = t;
}

float OMediaASEConverter::get_float(void)
{
	float	f;
	string	str;

	get_number(str);
	
	sscanf(str.c_str(),"%f",&f);
	
	return f;
}

long OMediaASEConverter::get_integer(void)
{
	long	l;
	string	str;

	get_number(str);

	sscanf(str.c_str(),"%ld",&l);

	return l;
}

void OMediaASEConverter::move_to_char(char c)
{
	while((*ptr)!=c) advance_ptr();
}

void OMediaASEConverter::skip_string(void)
{
	for(;;)
	{
		advance_ptr();
		if ((*ptr)=='"')
		{
			advance_ptr();
			break;
		}
	}
}

void OMediaASEConverter::skip_block(void)
{
	long level_count = 0;

	for(;;)
	{
		advance_ptr();
		
		if ( (*ptr)=='"') skip_string();

		else if ((*ptr)=='{') level_count++;

		else if ((*ptr)=='}')
		{
			advance_ptr();
			if (level_count==0) break;
			level_count--;
		}
	}
}



void OMediaASEConverter::scan_ase(void)
{
	string cmd;
	bool block;
	OMedia3DShape	*base_shape = shape;

	ptr = buffer;
	vertex_mesh_base = face_mesh_base = color_mesh_base = 0;

	shape->db_lock();
	
	try
	{
		for(;;)
		{
			get_next_code(cmd,block);
			if (cmd=="MATERIAL_LIST")
			{
				if (shape->get_database() && mat_conv_mode!=omc3dmconv_IgnoreMaterial)
				{
					scan_material_list();
				}
				else skip_block();		
			}
			else if (cmd=="GEOMOBJECT")
			{			
				scan_geomobject();
			}
			else if (cmd=="GROUP")
			{
				scan_group();
			}
			
			else if (block) skip_block();
		}
	}

	catch(...) {}

	finalize_shape();
	shape->db_unlock();

	shape = base_shape;
}

void OMediaASEConverter::scan_group(void)
{
	string cmd,group_name;
	bool block;

	group_level++;
	get_string(group_name);
	
	if (obj_conv_mode==omc3doconv_OneShapePerGroup &&
		group_level==1)
	{
		if (first_shape) first_shape = false;
		else 
		{
			if (start_new_shape(group_name,true)) shape->db_lock();
		}
	}

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;

		if (cmd=="GROUP") scan_group();
		else if (cmd=="GEOMOBJECT")
		{
			scan_geomobject();
		}		
		else if (block) skip_block();
	}

	group_level--;
}

void OMediaASEConverter::finalize_shape(void)
{
	reject_bad_faces();
	prepare_maps();

	omt_PrepareFlags prepare_f =ompfc_PrepareAll;
	if (face_uvw_computed) prepare_f &= ~ompfc_SetDefaultTextureOrientation;
	if (!((conv_flags&om3dcf_MergeNormals) && anim_conv_mode==omc3danconv_Ignore)) prepare_f &= ~ompfc_ComputeVertexNormals;
	shape->prepare(prepare_f);
	merge_duplicated_vertices();
	shape->delete_unused_vertices();
	if ((conv_flags&om3dcf_MergeNormals) && anim_conv_mode==omc3danconv_Ignore) shape->merge_normals(false);
	
	face_uvw_computed = false;
	vertex_mesh_base = face_mesh_base = color_mesh_base = 0;
	polygon_materials.erase(polygon_materials.begin(),polygon_materials.end());
	map_uvw.erase(map_uvw.begin(),map_uvw.end());
	dup_vertices.erase(dup_vertices.begin(),dup_vertices.end());

	if (shape->get_colors()->size()==0) vertex_color_mode = false;

	shape->set_modified();
	OMedia3DShapeConverter::finalize_shape();
}

void OMediaASEConverter::scan_geomobject(void)	
{
	string cmd,str,str2;
	bool block;

	if (obj_conv_mode!=omc3doconv_MergeObjects)
	{
		if (!(obj_conv_mode==omc3doconv_OneShapePerGroup && group_level))
		{	
			if (first_shape) first_shape = false;
			else 
			{
				if (start_new_shape("",true)) shape->db_lock();
			}
		}
	}

	polygon_materials.erase(polygon_materials.begin(),polygon_materials.end());

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;
		if (cmd=="NODE_NAME")
		{
			get_string(str);
			shape->get_chunk_name(str2);
			if (str2.length()==0) shape->set_chunk_name(str);
		}
		else if (cmd=="NODE_TM")
		{
			scan_transform();
		}
		else if (cmd=="MESH")
		{		
			scan_mesh();
		}
		else if (cmd=="MESH_ANIMATION" && anim_conv_mode!=omc3danconv_Ignore && shape->get_database())
		{
			scan_animation();
		}
		else if (cmd=="MATERIAL_REF")
		{
			long	mat_ref = get_integer();
			if (mat_ref<(long)materials.size())
			{
				omt_PolygonList::iterator	pi;
			
				if (materials[mat_ref].mat)
				{
					// Standard material
					
					for(pi=shape->get_polygons()->begin()+face_mesh_base;
						pi!=shape->get_polygons()->end();
						pi++)
					{
						(*pi).set_material(materials[mat_ref].mat);
					}				
				}
				else
				{
					// Multiple material
					
					for(vector<OMediaASEMatPolyID>::iterator pmi = polygon_materials.begin();
						pmi != polygon_materials.end();
						pmi++)
					{
						if ( (*pmi).submat_id<(long)materials[mat_ref].sub_mat.size())
						{
							OMedia3DMaterial *matp = materials[mat_ref].sub_mat[(*pmi).submat_id];
						
							(*shape->get_polygons())[(*pmi).poly].set_material(matp);						
						}
					
					}				
				}			
			}
		}
		else if (block) skip_block();
	}

	vertex_mesh_base += shape->get_vertices()->size()-vertex_mesh_base;
	face_mesh_base += shape->get_polygons()->size()-face_mesh_base;
	color_mesh_base += shape->get_colors()->size()-color_mesh_base;
	
	map_uvw.erase(map_uvw.begin(),map_uvw.end());
}

void OMediaASEConverter::scan_transform(void)
{
	string 			cmd;
	bool 			block,fill_matrix;
	long			row;

	transform_matrix.set_identity();

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;		

		fill_matrix = false;
		if (cmd=="TM_ROW0")
		{
			fill_matrix = true;	row = 0;
		}
		else if (cmd=="TM_ROW1")
		{
			fill_matrix = true;	row = 1;
		}
		else if (cmd=="TM_ROW2")
		{
			fill_matrix = true;	row = 2;
		}
		else if (cmd=="TM_ROW3")
		{
			fill_matrix = true;	row = 3;
		}
		else if (block) skip_block();

		if (fill_matrix)
		{
			transform_matrix.m[row][0] = get_float();	
			transform_matrix.m[row][1] = get_float();	
			transform_matrix.m[row][2] = get_float();
		}
	}		

	transform_matrix.invert(inv_transform_matrix);
}

void OMediaASEConverter::scan_animation(void)
{
	string 	cmd;
	bool 	block;
	float	anim_time_count,last_anim_time_count;
	string	name,name2; 
	long	sequence;
	OMedia3DMorphAnimFrame	*last_frame = NULL;
	OMedia3DShapeConvMorphDef	cmdef;

	anim_time_count = last_anim_time_count = 0.0f;

	if (!anim_def)
	{
		shape->get_chunk_name(name);

		switch (get_anim_conv_mode_mode())
		{
			case omc3danconv_ReplaceMorphAnimDef:
			if (shape->get_database()->object_exists(OMedia3DMorphAnimDef::db_type,name))
			{
				OMediaFSChunk *fs = shape->get_database()->get_chunk(OMedia3DMorphAnimDef::db_type, name);
				if (fs) shape->get_database()->delete_object(OMedia3DMorphAnimDef::db_type,fs->ckid);
			}

			anim_def = omd_GETOBJECT(shape->get_database(),OMedia3DMorphAnimDef, name);
			anim_def->setnsequences(0);
			anim_def->setnsequences(1);
			sequence = 0;
			break;

			case omc3danconv_AlwaysCreateNewMorphAnimDef:
			{
				name2 = name;
				for(long c=1;;c++)
				{
					if (!shape->get_database()->object_exists(OMedia3DMorphAnimDef::db_type,name2)) break;
					name2 = name;
					name2 += ".";
					name2 += omd_L2STR(c);
				}
				name = name2;

				anim_def = omd_GETOBJECT(shape->get_database(),OMedia3DMorphAnimDef, name);
				anim_def->setnsequences(1);
				sequence = 0;
			}
			break;

			case omc3danconv_AddSequence:	
			anim_def = omd_GETOBJECT(shape->get_database(),OMedia3DMorphAnimDef, name);
			anim_def->setnsequences(anim_def->getnsequences()+1);
			sequence = anim_def->getnsequences()-1;
			break;
                        
                        default:break;
		}
	}


	anim_def->set_modified();
	
	cmdef.morph_def = anim_def->get_chunk_ID();
	cmdef.its_shape = shape->get_chunk_ID();
	related_morphdefs.push_back(cmdef);

	timevalue = 0;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;
			
		if (cmd=="MESH")
		{
			morph_frame = (OMedia3DMorphAnimFrame*)anim_def->create_frame(sequence);

			scan_mesh();

			last_anim_time_count = anim_time_count;

			if (last_frame)
			{
				anim_time_count = ((float(timevalue)/4800.0f) * 1000.0f);
				last_frame->setframespeed_tb(1000.0f / (anim_time_count-last_anim_time_count) );
				morph_frame->setframespeed_tb(1000.0f / last_frame->getframespeed_tb());
			}
			else 
			{
				morph_frame->setframespeed_tb(0);
				anim_time_count = 0;
			}
			
			last_frame = morph_frame;
			morph_frame = NULL;
 

		}
		else if (block) skip_block();
	}
}

void OMediaASEConverter::scan_mesh()
{
	string 	cmd;
	bool 	block;
	long	nvertex; 
	long	nfaces;


	try
	{
		for(;;)
		{
			if (!get_next_code(cmd,block)) break;
			
			if (cmd=="MESH_NUMVERTEX")
			{
				nvertex = get_integer();		
			}
			else if (cmd=="TIMEVALUE")
			{
				timevalue = get_integer();		
			}
			else if (cmd=="MESH_NUMFACES")
			{
				nfaces = get_integer();			
			}
			else if (cmd=="MESH_VERTEX_LIST")
			{
				scan_vertex_list();
			}
			else if (cmd=="MESH_FACE_LIST" && !morph_frame)
			{
				scan_face_list();
			}
			else if (cmd=="MESH_NUMTVERTEX")
			{
			}
			else if (cmd=="MESH_NORMALS")
			{
				scan_normal_list();			
			}		
			else if (cmd=="MESH_TVERTLIST" && !morph_frame)
			{
				scan_mapvertex();
			}
			else if (cmd=="MESH_TFACELIST" && !morph_frame)
			{
				scan_facevertex();		
			}
			else if (cmd=="MESH_CVERTLIST")
			{
				scan_colorvertex();			
			}
			else if (cmd=="MESH_CFACELIST" && !morph_frame)
			{
				scan_colorface_list();
			}
			
			else if (block) skip_block();
		}	
	}
	catch(...) {}	
}

void OMediaASEConverter::scan_colorvertex(void)
{
	string 	cmd;
	bool 	block;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;
		
		if (cmd=="MESH_VERTCOL")
		{
			get_integer();		// vnum
			OMediaFARGBColor	argb;
			
			argb.alpha = 1.0f;
			argb.red = get_float();
			argb.green = get_float();
			argb.blue = get_float();
			
			if (morph_frame) morph_frame->get_colors()->push_back(argb);
			else
				shape->get_colors()->push_back(argb);
		}
		else if (block) skip_block();
	}		
}
		
void OMediaASEConverter::scan_colorface_list(void)
{
	string 			cmd;
	bool 			block;
	OMedia3DPolygon	*poly;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;
		
		if (cmd=="MESH_CFACE")
		{
			poly = &(*shape->get_polygons())[get_integer()+face_mesh_base];
			
			poly->get_vertices()[0].color_index = get_integer() + color_mesh_base;
			poly->get_vertices()[1].color_index = get_integer() + color_mesh_base;
			poly->get_vertices()[2].color_index = get_integer() + color_mesh_base;
		}
		else if (block) skip_block();
	}		
}

void OMediaASEConverter::scan_vertex_list(void)
{
	string 	cmd;
	bool 	block;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;
		
		if (cmd=="MESH_VERTEX")
		{
			get_integer();			// vnum
			OMedia3DPoint	p;
			
			p.x = get_float();
			p.z = get_float();
			p.y = get_float();
			
			if (!morph_frame)
				shape->get_vertices()->push_back(p);
			else
				morph_frame->get_vertices()->push_back(p);
		}
		else if (block) skip_block();
	}		
}

void OMediaASEConverter::reject_bad_faces(void)
{
	for(long p=0;p<(long)shape->get_polygons()->size();)
	{
		long	v1,v2,v3;
		
		v1 = (*shape->get_polygons())[p].get_point(0);
		v2 = (*shape->get_polygons())[p].get_point(1);
		v3 = (*shape->get_polygons())[p].get_point(2);
	
		if (v1==v2 || v1==v3 || v2==v3) // Reject lines
		{
			shape->get_polygons()->erase(shape->get_polygons()->begin()+p);
			continue;		
		}

		p++;
	}
}

void OMediaASEConverter::scan_face_list(void)
{
	string 	cmd;
	bool 	block;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;
		
		if (cmd=="MESH_FACE")
		{
			OMedia3DPolygon	p;
			long 			v1,v2,v3;
			
			p.set_triangle();
			
			move_to_char(':');	advance_ptr(); 	// Face number

			move_to_char(':');	advance_ptr();	// A
			
			v1 = get_integer();
			
			move_to_char(':');	advance_ptr();	// B

			v2 = get_integer();
			
			move_to_char(':');	advance_ptr();	// C

			v3 = get_integer();

			p.set_point(0,v1+vertex_mesh_base);
			p.set_point(1,v2+vertex_mesh_base);
			p.set_point(2,v3+vertex_mesh_base);
			shape->get_polygons()->push_back(p);
		}
		else if (cmd=="MESH_SMOOTHING") {}
		else if (cmd=="MESH_MTLID") 
		{
			OMediaASEMatPolyID	matpolyid;
			
			matpolyid.submat_id = get_integer();
			matpolyid.poly = shape->get_polygons()->size()-1;
		
			polygon_materials.push_back(matpolyid);		
		}
		else if (block) skip_block();
	}		
}

class OMediaASENormal
{
	public:
	
	vector<long>	poly_used;
	OMedia3DVector	normal;
};

class OMediaASENormalPerVertex
{
	public:

	vector<OMediaASENormal>	normals;	
};


void OMediaASEConverter::scan_normal_list(void)
{
	string 	cmd;
	bool 	block;

	long	nnewvertices = shape->get_vertices()->size()-vertex_mesh_base;

	long	current_poly = shape->get_polygons()->size();
	vector<OMediaASENormal>::iterator aseni;	
	vector<OMediaASENormalPerVertex>	normals_per_v;
	OMediaASENormalPerVertex	emptynpv;
	bool vertex_normal_computed = false;

	if (!morph_frame) animnorm_base_newvertices = nnewvertices;
	else nnewvertices = animnorm_base_newvertices;
	
	normals_per_v.insert(normals_per_v.end(),nnewvertices, emptynpv);


	for(;;)
	{
		if (!get_next_code(cmd,block)) break;
		
		if (cmd=="MESH_FACENORMAL")
		{		
			current_poly = get_integer()+face_mesh_base;
		}
		else if (cmd=="MESH_VERTEXNORMAL")
		{
			float			tf;
			long			vertindex = get_integer();
			OMedia3DVector	n;
			n.x = get_float();
			n.y = get_float();
			n.z = get_float();
			inv_transform_matrix.normal_transform(n,n);
			tf = n.y;	n.y = n.z;	n.z = tf;

			vertex_normal_computed = true;
			
			if (vertindex<(long)normals_per_v.size())
			{
				OMediaASENormalPerVertex	*npv = &normals_per_v[vertindex];
				
				// Is this normal already inserted?

				if ((conv_flags&om3dcf_MergeVerticesNormals) && anim_conv_mode==omc3danconv_Ignore)
				{
					for(aseni = npv->normals.begin();
						aseni!=npv->normals.end();
						aseni++)
					{
						if ( (*aseni).normal.x==n.x &&
							 (*aseni).normal.y==n.y && 
							 (*aseni).normal.z==n.z )
						{
							(*aseni).poly_used.push_back(current_poly);
							break;
						}				
					}
				}
				else aseni=npv->normals.end();

				if (aseni==npv->normals.end())
				{
					OMediaASENormal newnormal;
					
					newnormal.normal = n;
					newnormal.poly_used.push_back(current_poly);
					
					npv->normals.push_back(newnormal);
				}					
			}
		}
		else if (block) skip_block();
	}
	
	
	// Now resolve vertex normals

	if (vertex_normal_computed)
	{
		vector<OMediaASENormalPerVertex>::iterator npvi;
		vector<OMediaASENormal>::iterator ni;
		
		long			cur_n,new_n;
		OMedia3DVector	emptyvect;
	
		if (morph_frame)
		{
			morph_frame->get_normals()->insert(morph_frame->get_normals()->end(),nnewvertices, emptyvect);

			for(npvi = normals_per_v.begin(),cur_n=vertex_mesh_base;
				npvi != normals_per_v.end();
				npvi++,cur_n++)
			{
				if (! (*npvi).normals.size() ) continue;
				
				ni = (*npvi).normals.begin();
				
				(*morph_frame->get_normals())[cur_n] = (*ni).normal;
				for(;ni!=(*npvi).normals.end();ni++)
				{
					morph_frame->get_normals()->push_back((*ni).normal);
				}
			}
		}
		else
		{
			shape->get_normals()->insert(shape->get_normals()->end(),nnewvertices, emptyvect);
			
			for(npvi = normals_per_v.begin(),cur_n=vertex_mesh_base;
				npvi != normals_per_v.end();
				npvi++,cur_n++)
			{
				if (! (*npvi).normals.size() ) continue;
				
				ni = (*npvi).normals.begin();
				
				(*shape->get_normals())[cur_n] = (*ni).normal;
				
				for(;ni!=(*npvi).normals.end();ni++)
				{
					OMedia3DPoint	p;
					p = (*shape->get_vertices())[cur_n];
					shape->get_vertices()->push_back(p);
					new_n = shape->get_vertices()->size()-1;
					
					(*shape->get_normals()).push_back((*ni).normal);

					OMediaASETempDuplicatedVertex	tdv;
					
					tdv.original = cur_n;
					tdv.copy = new_n;
					dup_vertices.push_back(tdv);
						
					for(vector<long>::iterator polyi = (*ni).poly_used.begin();
						polyi != (*ni).poly_used.end();
						polyi++)
					{
						OMedia3DPolygon	*poly = &(*shape->get_polygons())[(*polyi)];
					
						for(long i=0; i<(long)poly->get_num_points(); i++)
						{
							if ((long)poly->get_point(i)==cur_n) poly->set_point(i,new_n);					
						}
					}			
				}
			}
		}
	}
}

void OMediaASEConverter::scan_material_list(void)
{
	string 	cmd;
	bool 	block;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;

		if (cmd=="MATERIAL")
		{
			super_mat = NULL;
			two_sided = false;
			scan_material();
		}
		else if (block) skip_block();
	}
}

void OMediaASEConverter::scan_material(void)
{
	OMediaASEMatConvert	ase_mat;

	OMediaFARGBColor	ambient,diffuse,specular;
	float				alpha_channel;
	float				gouraud;
	bool				new_material = false;
	
	string 				cmd,mat_name,class_name,str;
	bool 				block;
	bool				texture = false;

	ambient.set(1.0f,  0,0,0);
	diffuse.set(1.0f,  0,0,0);
	specular.set(1.0f, 0,0,0);
	gouraud = false;
	alpha_channel = 0;

	if (!super_mat) 
	{
		materials.push_back(ase_mat);
		submat_count = 0;
	}

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;

		if (cmd=="MATERIAL_NAME")
		{
			get_string(mat_name);
			if (!super_mat) super_mat_name = mat_name;
			else if (two_sided) mat_name = super_mat_name;
		}
		else if (cmd=="MATERIAL_CLASS")
		{
			get_string(class_name);
			if (class_name=="Standard" || super_mat) new_material = true;
			else if (class_name=="Double Sided") two_sided = true;
		}
		else if (cmd=="MATERIAL_AMBIENT")
		{
			ambient.red = get_float();
			ambient.green = get_float();
			ambient.blue = get_float();
		}
		else if (cmd=="MATERIAL_DIFFUSE")
		{
			diffuse.red = get_float();
			diffuse.green = get_float();
			diffuse.blue = get_float();
		}
		else if (cmd=="MATERIAL_SPECULAR")
		{
			specular.red = get_float();
			specular.green = get_float();
			specular.blue = get_float();
		}
		else if (cmd=="MATERIAL_TRANSPARENCY")
		{
			alpha_channel = get_float();
		}
		else if (cmd=="MATERIAL_SHADING")
		{
			get_keyword(str);
			gouraud = (str!="Constant");
		}
		else if (cmd=="SUBMATERIAL" && !super_mat)
		{
			super_mat = &materials.back();
			scan_material();	
			super_mat = NULL;
			submat_count++;
		}
		else if (cmd=="MAP_DIFFUSE" && new_material)
		{
			texture = true;
			scan_map();

			diffuse.set(1.0f,1.0f,1.0f,1.0f);
			float	rgb = (float(ambient.red)+float(ambient.green)+float(ambient.blue))/3.0f;
			ambient.set(1.0f,rgb,rgb,rgb);
		}
		
		else if (block) skip_block();
	}
	
	if (new_material)
	{
		OMedia3DMaterial *mat = NULL;
		OMediaDataBase	 *db = shape->get_database();
		bool			front_material = true;
		bool			update_mat = true;
		
		if (two_sided && submat_count>0)
		{
			mat = materials.back().mat;			
			front_material = false;	//FIXME: Backface material not supported at this time
		}
		else
		{
			
			if (mat_conv_mode==omc3dmconv_CreateNewMaterialIfNotFound ||
				mat_conv_mode==omc3dmconv_ExistingMaterialOnly)
			{
				if (db->object_exists(OMedia3DMaterial::db_type, mat_name))
				{
					mat = (OMedia3DMaterial *)db->get_object(OMedia3DMaterial::db_type, mat_name);
					update_mat = false;
				}
			}
			
			if (!mat && (mat_conv_mode==omc3dmconv_CreateNewMaterialIfNotFound ||
						 mat_conv_mode==omc3dmconv_AlwaysCreateNewMaterial ||
						 mat_conv_mode==omc3dmconv_ReplaceMaterial))
			{			
				string	str = new_material_prefix;
				str += mat_name;
				long num = 2;
				
				if (mat_conv_mode!=omc3dmconv_ReplaceMaterial)
				{
					for(;;)
					{
						if (!db->object_exists(OMedia3DMaterial::db_type, str)) break;
		
						str = new_material_prefix;
						str += mat_name;
						str += " ";
						str += omd_L2STR(num);
						
						num++;
					}
				}
			
				mat = (OMedia3DMaterial *)db->get_object(OMedia3DMaterial::db_type, str);
				mat->set_modified();
			}
		}

		if (mat) related_materials.push_back(mat->get_chunk_ID());

		if (mat && front_material && update_mat) //FIXME: Backface material not supported at this time
		{
			diffuse.alpha = 1.0f-alpha_channel;
			mat->purge();
			mat->set_ambient(ambient);
			mat->set_diffuse(diffuse);
			mat->set_specular(specular);
			mat->set_shade_mode(gouraud?omshademc_Gouraud:omshademc_Flat);
			mat->set_light_mode(ommlmc_Light);
		}
		
		if (super_mat && !two_sided)
		{
			super_mat->sub_mat.push_back(mat);
			super_mat->sub_updated.push_back(update_mat);
		}
		else 
		{
			materials.back().mat = mat;
			materials.back().updated = update_mat;
		}
		
		if (texture && front_material)  //FIXME: Backface material not supported at this time
		{
			maps.back().material = mat;
			maps.back().front_material = front_material;
		}
	}
}


void OMediaASEConverter::unlock_materials(bool set_color_vertex_mode)
{
	for(vector<OMediaASEMatConvert>::iterator mati = materials.begin();
		mati!=materials.end();
		mati++)
	{
		if ( (*mati).mat) 
		{
			if (set_color_vertex_mode && (*mati).updated) (*mati).mat->set_light_mode(ommlmc_VertexColor);
			
			(*mati).mat->db_unlock();
		}

		for(long l=0; l<(long)(*mati).sub_mat.size(); l++)
		{
			OMedia3DMaterial *mat;
			mat = (*mati).sub_mat[l];
			
			if (mat)
			{
				if ((*mati).sub_updated[l] && set_color_vertex_mode)
				{
					mat->set_light_mode(ommlmc_VertexColor);
				}			
				
				mat->db_unlock();
			}
		}	
	}
}

void OMediaASEConverter::scan_map(void)
{
	string 	cmd;
	bool 	block;
	OMediaASEMatMap	new_map;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;

		if (cmd=="MAP_NAME")
		{
			get_string(new_map.map_name);
		}
		else if (cmd=="BITMAP")
		{
			get_string(new_map.map_file);
		}
		else if (cmd=="UVW_U_OFFSET")
		{
			new_map.u_offset = get_float();
		}
		else if (cmd=="UVW_V_OFFSET")
		{
			new_map.v_offset = get_float();
		}
		else if (cmd=="UVW_U_TILING")
		{
			new_map.u_tiling = get_float();
		}
		else if (cmd=="UVW_V_TILING")
		{
			new_map.v_tiling = get_float();
		}		
		else if (block) skip_block();
	}
	
	maps.push_back(new_map);
}

void OMediaASEConverter::prepare_maps(void)
{
	OMedia3DShapeConvTFileName	rel_filename;

	for(vector<OMediaASEMatMap>::iterator im=maps.begin();
		im!=maps.end();
		im++)
	{
		if (!(*im).material) continue;

		rel_filename.material_id = (*im).material->get_chunk_ID();
		rel_filename.texture_filename = (*im).map_file;
		related_texturefile.push_back(rel_filename);
	}
}

void OMediaASEConverter::scan_mapvertex(void)
{
	string 	cmd;
	bool 	block;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;

		if (cmd=="MESH_TVERT")
		{
			long	tn;
			OMediaASEMapUVW	map_v;
			
			tn = get_integer();
			map_v.u = get_float();
			map_v.v = get_float();
			map_v.w = get_float();
			
			map_uvw.push_back(map_v);
		}		
		else if (block) skip_block();
	}
}

void OMediaASEConverter::scan_facevertex(void)
{
	face_uvw_computed = true;

	string 	cmd;
	bool 	block;

	for(;;)
	{
		if (!get_next_code(cmd,block)) break;

		if (cmd=="MESH_TFACE")
		{
			long	face,p[3];
			float	u,v;
			short i;
			
			face = get_integer()+face_mesh_base;
			p[0] = get_integer();
			p[1] = get_integer();
			p[2] = get_integer();
			
			if (face<(long)shape->get_polygons()->size())
			{
				for( i=0;i<3;i++)
				{					
					u = (p[i]<(long)map_uvw.size())?map_uvw[p[i]].u:0.0f;
					v = (p[i]<(long)map_uvw.size())?-map_uvw[p[i]].v:0.0f;

					(*shape->get_polygons())[face].set_text_coord_u(i,u);					
					(*shape->get_polygons())[face].set_text_coord_v(i,v);					
				}						
			}

		}
		else if (block) skip_block();
	}
}

void OMediaASEConverter::merge_duplicated_vertices(void)
{
    vector<OMediaASETempDuplicatedVertex>::iterator vi;
    
    omt_PolygonList				newPolyList;
    omt_VertexList				newVertexList;
    typedef less<int>	less_int;
    map<int,int,less_int>			vertexCopyMap;
    map<int,int,less_int>::iterator	mi;

    // Reset original vertices

    for(vi=dup_vertices.begin();
        vi!=dup_vertices.end();
        vi++)
    {
        vertexCopyMap[(*vi).copy] = (*vi).original;
    }
    
    for(omt_PolygonList::iterator pi=shape->get_polygons()->begin();	
            pi!=shape->get_polygons()->end();
            pi++)	
    {
        for(long p=0;p<(long)(*pi).get_num_points();p++)
        {
            mi = vertexCopyMap.find((*pi).get_point(p));
            if (mi!=vertexCopyMap.end())
            {
                (*pi).set_point(p,(*mi).second,false);
            }
        }
    }	

}


// * SoftImage .xsi converter

OMediaXSIConverter::OMediaXSIConverter(OMediaStreamOperators *stream, OMedia3DShape *shape) : 
					OMedia3DShapeConverter(stream,shape) 
{
	point_base_index=normal_base_index=polygon_base_index=0;
	first_mesh = true;
}


OMediaXSIConverter::~OMediaXSIConverter() {}

void OMediaXSIConverter::convert(void)
{
	base_shape = shape;
	base_shape->db_lock();			// double-lock base shape
	shape->db_lock();

	buffer = new char[stream->getsize()];	// Read the XSI file into a buffer
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);

	buffer_end = buffer + stream->getsize();
	
	stream->read(buffer,stream->getsize());

	try
	{	
		scan_xsi();
	}
	catch(...) {}

	delete [] buffer;

	finalize_shape();

	shape->db_unlock();
	base_shape->db_unlock();
	shape = base_shape;
}

void OMediaXSIConverter::finalize_shape(void)
{
	shape->prepare(ompfc_PrepareAll&(~(ompfc_SetDefaultTextureOrientation|ompfc_ComputeVertexNormals)));

	if (shape->get_normals()->size()==0) shape->compute_normals(0.3f);
	
	shape->delete_unused_vertices();
	if ((conv_flags&om3dcf_MergeNormals) && anim_conv_mode==omc3danconv_Ignore) shape->merge_normals(false);
	shape->set_modified();
	OMedia3DShapeConverter::finalize_shape();
}

void OMediaXSIConverter::move_to_char(char c)
{
	while((*ptr)!=c) advance_ptr();
}

void OMediaXSIConverter::get_string(string &str)
{
	move_to_char('"');
	advance_ptr();
	
	char	*str_start;
	
	str_start = ptr;

	move_to_char('"');
	
	(*ptr) = 0;
	str = str_start;
	(*ptr) = '"';	

	advance_ptr();
}

void OMediaXSIConverter::get_number(string &nstr)
{
	char	*n_start;
	char	t;

	for(;;)
	{
		if ( (*ptr)=='.' || (*ptr)=='-') break;
		if ( (*ptr)>='0' && (*ptr) <='9' ) break;

		advance_ptr();
	}

	n_start = ptr;
	advance_ptr();

	for(;;)
	{
		if ( ((*ptr)!='.' && (*ptr)!='-') && ((*ptr)<'0' || (*ptr) >'9')) break;
		advance_ptr();
	}
	
	t = (*ptr);
	(*ptr) = 0;
	nstr = n_start;
	(*ptr) = t;
}

float OMediaXSIConverter::get_float(void)
{
	float	f;
	string	str;

	get_number(str);
	
	sscanf(str.c_str(),"%f",&f);
	
	return f;
}

long OMediaXSIConverter::get_integer(void)
{
	long	l;
	string	str;

	get_number(str);

	sscanf(str.c_str(),"%ld",&l);

	return l;
}


void OMediaXSIConverter::skip_block(void)
{
	long	hlevel = 0;
	advance_ptr();

	for(;;)
	{
		if ((*ptr)=='}')
		{
			if (hlevel==0) 
			{
				advance_ptr();
				return;
			}

			hlevel--;
		}
		else if ((*ptr)=='{') hlevel++;

		advance_ptr();
	}
}

void OMediaXSIConverter::move_to_block_start(void)
{
	for(;;)
	{
		if ((*ptr)=='{') break;
		advance_ptr();
	}

	advance_ptr();
}


bool OMediaXSIConverter::get_keyword(string &str)
{
	for(;;)
	{
		while((*ptr)==' ' || (*ptr)=='\t' || (*ptr)=='\r' || (*ptr)=='\n') advance_ptr();
		if ((*ptr)=='}') 
		{
			advance_ptr();
			return true;
		}
		if ((*ptr)=='{') skip_block();
		else break;
	}

	char	*str_start;
	
	str_start = ptr;

	while(!((*ptr)==' ' || (*ptr)=='\t' || (*ptr)=='\r' || (*ptr)=='\n' || (*ptr)=='{')) advance_ptr();

	char t;
	
	t = (*ptr);
	(*ptr) = 0;
	str = str_start;
	(*ptr) = t;	

	return false;
}


void OMediaXSIConverter::scan_xsi(void)
{
	string		str;

	ptr = buffer;

	while(ptr<buffer_end)
	{
		get_keyword(str);
		if (str=="Frame")
		{
			get_keyword(str);
			move_to_block_start();
			scan_frame();
		}


	}
}

void OMediaXSIConverter::scan_frame(void)
{
	string				str,shape_name;
	OMediaMatrix_4x4	matrix;

	matrix.set_identity();

	xform_matrix_stack.push_back(matrix);
	inv_xform_matrix_stack.push_back(matrix);

	for(;;)
	{
		if (get_keyword(str)) break;
		
		if (str=="Frame")
		{
			get_keyword(str);
			move_to_block_start();
			scan_frame();			
		}
		else if (str=="FrameTransformMatrix")
		{
			move_to_block_start();
			scan_xform_matrix();
		}
		else if (str=="Mesh")
		{
			get_keyword(shape_name);

			move_to_block_start();
			
			if (first_mesh) first_mesh = false;
			else
			{
				if (obj_conv_mode!=omc3doconv_MergeObjects && start_new_shape(shape_name, true))
				{
					shape->db_lock();
					point_base_index = 0;
					normal_base_index = 0;
					polygon_base_index = 0; 
				}
				else
				{
					point_base_index = shape->get_vertices()->size();
					normal_base_index = shape->get_normals()->size();
					polygon_base_index = shape->get_polygons()->size();
				}
			}

			shape->set_chunk_name(shape_name);

			scan_mesh_geometry();
			scan_mesh_blocks();		
		}

		advance_ptr();
		if ((*ptr)=='}') break;
	}

	list<OMediaMatrix_4x4>::iterator mi;

	mi = xform_matrix_stack.end();		mi--;	xform_matrix_stack.erase(mi);
	mi = inv_xform_matrix_stack.end();	mi--;	inv_xform_matrix_stack.erase(mi);
}

void OMediaXSIConverter::scan_mesh_geometry(void)
{
	OMedia3DPoint	p;
	OMedia3DPolygon	poly;
	long			n,np,i,v;

	// Vertices

	n = get_integer();
	while(n--)
	{
		p.x = get_float();
		p.y = get_float();
		p.z = get_float();
		transform_point(p);
		p.z = -p.z;
		shape->get_vertices()->push_back(p);
	}

	// Polygons

	n = get_integer();
	while(n--)
	{
		np = get_integer();
		i = 0;
		
		poly.get_vertices().erase(poly.get_vertices().begin(),poly.get_vertices().end());

		while(np--)
		{
			v = get_integer() + point_base_index;
			
			OMedia3DPolygonVertex	pv;
			pv.vertex_index = v;
			pv.normal_index = 0;
			pv.color_index = 0;
			pv.u = 0.0f;
			pv.v = 0.0f;

			poly.get_vertices().push_back(pv);
			i++;
		}

		shape->get_polygons()->push_back(poly);
	}
}

void OMediaXSIConverter::scan_mesh_blocks(void)
{
	string		str;

	for(;;)
	{
		if (get_keyword(str)) return;

		if (str=="MeshMaterialList")
		{
			if (mat_conv_mode!=omc3dmconv_IgnoreMaterial &&
				shape->get_database())
			{
				move_to_block_start();
				scan_materials();
			}
		}
		else  if (str=="SI_MeshNormals")
		{
			move_to_block_start();
			scan_normals();
		}
		else if (str=="SI_MeshTextureCoords")
		{
			move_to_block_start();
			scan_text_coords();
		}

		advance_ptr();
		if ((*ptr)=='}') return;
	}
}

void OMediaXSIConverter::scan_materials(void)
{
	bool						eof = false;
	long						nmaterials,npolys,l,mat_n;
	vector<OMedia3DMaterial*>	materials;
	vector<long>				polys_mat;
	OMedia3DMaterial			*mat;
	string						shape_name,mat_name,str;
	OMediaDataBase				*dbase = shape->get_database();
	long						light_mode;

	nmaterials = get_integer();

	// Read polygon materials

	npolys = get_integer();
	while(npolys--)
	{
		l = get_integer();
		polys_mat.push_back(l);
	}

	// Prepare materials

	shape->get_chunk_name(shape_name);
	mat_n = 0;

	try
	{
		for(;;)
		{
			if (get_keyword(str)) break;

			if (str=="SI_Material")
			{
				move_to_block_start();
				OMediaFARGBColor	diffuse,specular,ambient,emission;
				float			shininess;
				string			textname;

				diffuse.red		= get_float();
				diffuse.green	= get_float();
				diffuse.blue	= get_float();
				diffuse.alpha	= get_float();

				shininess		= get_float();
				specular.red	= get_float();
				specular.green	= get_float();
				specular.blue	= get_float();
				specular.alpha	= 1.0f;

				emission.red	= get_float();
				emission.green	= get_float();
				emission.blue	= get_float();
				emission.alpha	= 1.0f;

				light_mode =	get_integer();

				ambient.red		= get_float();
				ambient.green	= get_float();
				ambient.blue	= get_float();
				ambient.alpha	= 1.0f;

				textname = "";

				while(!get_keyword(str))
				{
					if (str=="SI_Texture2D")
					{
						move_to_block_start();
						get_string(textname);
						while(!get_keyword(str)) {}
					}
				}				


				mat_name = shape_name;
				mat_name += ".";
				mat_name += omd_L2STR(mat_n);
				mat = NULL;

				switch(mat_conv_mode)
				{
					case omc3dmconv_CreateNewMaterialIfNotFound:
					{
						bool fillmat;
						fillmat = !dbase->object_exists(OMedia3DMaterial::db_type,mat_name);

						mat = (OMedia3DMaterial*)dbase->get_object(OMedia3DMaterial::db_type, mat_name);
						materials.push_back(mat);

						if (!fillmat) mat = NULL;
					}
					break;

					case omc3dmconv_ExistingMaterialOnly:
					if (dbase->object_exists(OMedia3DMaterial::db_type,mat_name))
					{
						mat = (OMedia3DMaterial*)dbase->get_object(OMedia3DMaterial::db_type,mat_name);
						materials.push_back(mat);
						mat = NULL;
					}
					else materials.push_back(NULL);
					break;

					case omc3dmconv_AlwaysCreateNewMaterial:
					{
						string name_b;
						long	n; 

						name_b = mat_name;
						
						if (dbase->object_exists(OMedia3DMaterial::db_type,name_b))
						{
							n = 0;
							do
							{
								name_b = mat_name;
								name_b += ".";
								name_b += omd_L2STR(n++);
							}
							while(dbase->object_exists(OMedia3DMaterial::db_type,name_b));
						}
						
						mat = (OMedia3DMaterial*)dbase->get_object(OMedia3DMaterial::db_type, name_b);
						materials.push_back(mat);
					}
					break;

					case omc3dmconv_ReplaceMaterial:
					mat = (OMedia3DMaterial*)dbase->get_object(OMedia3DMaterial::db_type, mat_name);
					materials.push_back(mat);
					break;

					default:
					materials.push_back(NULL);
				}

				if (mat)
				{
					related_materials.push_back(mat->get_chunk_ID());

					// Fill material

					OMediaCanvas			*canv;
					
					mat->purge();

					canv = find_map(textname);

					if (textname.length())
					{
						OMedia3DShapeConvTFileName	ctfn;

						ctfn.material_id = mat->get_chunk_ID();
						ctfn.texture_filename = textname;
						related_texturefile.push_back(ctfn);
					}

					mat->set_modified();
					
					if (light_mode==1) specular.set(1.0f,0.0f,0.0f,0.0f);

					mat->set_emission(emission);
					mat->set_diffuse(diffuse);
					mat->set_specular(specular);
					mat->set_ambient(ambient);
					
					mat->set_shade_mode(omshademc_Gouraud);
					mat->set_shininess(shininess);
					mat->set_light_mode((light_mode==0)?ommlmc_Color:ommlmc_Light);

					if (canv)
					{
						mat->set_texture(canv);
					}

					if (canv) canv->db_unlock();
				}

				mat_n++;
			}

			advance_ptr();
			if ((*ptr)=='}') break;
		}
	}
	catch(...)
	{
		eof = true;
	}

	vector<long>::iterator pi;

	for(pi = polys_mat.begin(),l=polygon_base_index;
		pi != polys_mat.end();
		pi++,l++)
	{
		if ((*pi)>=(long)materials.size()) continue;
		if (l>=(long)shape->get_polygons()->size()) break;

		(*shape->get_polygons())[l].set_material(materials[(*pi)]);
	}

	for (vector<OMedia3DMaterial*>::iterator mi = materials.begin();
		mi!=materials.end();
		mi++)
	{
		if ((*mi)) (*mi)->db_unlock();
	}

	if (eof) throw -1;
}

OMediaCanvas *OMediaXSIConverter::find_map(string map_name)
{
	OMediaDataBase	*db = shape->get_database();
	OMediaCanvas 	*canv;
	string			str;

	if (map_name.length()==0) return NULL;


	canv = NULL;

	// First let's see if there is a texture with the same name

	if (map_name.length() && db->object_exists(OMediaCanvas::db_type, map_name))
	{
		canv = (OMediaCanvas *) db->get_object(OMediaCanvas::db_type, map_name);
	}
	else
	{
		// Extract the file name
				
		str = map_name;
		short i;
		i = str.length()-1;
		for(;;)
		{
			if (str[i]=='\\' ||
				str[i]=='/' ||
				str[i]==':')
			{
				map_name = str.c_str()+i+1;
				break;						
			}

			if (i==0) break;
			i--;					
		}
	
		// Remove extension

		if (!db->object_exists(OMediaCanvas::db_type, map_name))
		{
			if (map_name.length()<5) return NULL;
			if (map_name[map_name.length()-4]=='.')
			{
				map_name.resize(map_name.length()-4);

				if (!db->object_exists(OMediaCanvas::db_type, map_name)) return NULL;
			}				
		}

		canv = (OMediaCanvas *) db->get_object(OMediaCanvas::db_type, map_name);
	}

	return canv;
}

class OMedia3DTextureCoordinate
{
	public:
	float u,v;
};

void OMediaXSIConverter::scan_text_coords(void)
{
	string								str;
	OMedia3DTextureCoordinate			uv;
	vector<OMedia3DTextureCoordinate>	coords;
	long								n,p,np,ptxt,i;
	OMedia3DPolygon						*poly;

	n = get_integer();
	while(n--)
	{
		uv.u = get_float();
		uv.v = -get_float();

		coords.push_back(uv);
	}

	n = get_integer();
	while(n--)
	{
		p = get_integer()+polygon_base_index;
		if (p>=(int)shape->get_polygons()->size()) 
		{
			poly = NULL;
		}
		else poly = &(*shape->get_polygons())[p];
		
		np = get_integer();
		i=0;

		while(np--)
		{
			ptxt = get_integer();
			if (poly && i<poly->get_num_points())
			{
				poly->set_text_coord_u(i, coords[ptxt].u);
				poly->set_text_coord_v(i, coords[ptxt].v);
			}

			i++;
		}
	}
}

void OMediaXSIConverter::scan_normals(void)
{
	long			nnormals,npolys,p,nv,v,n;
	OMedia3DPolygon	*poly;

	nnormals = get_integer();

	while(nnormals--)
	{
		OMedia3DVector	v;
		v.x = get_float();
		v.y = get_float();
		v.z = get_float();
		transform_normal(v);
		v.z = -v.z;
		
		shape->get_normals()->push_back(v);
	}

	npolys = get_integer();
	while(npolys--) 
	{
		p = get_integer();
		if (p>=(int)shape->get_polygons()->size())
		{
			poly = NULL;
		}
		else poly = &(*shape->get_polygons())[p+polygon_base_index];
		
		nv = get_integer();
		for(v=0;v<nv;v++)
		{
			n = get_integer()+normal_base_index;
			if (poly && v<poly->get_num_points()) 
				poly->set_vertex_normal(v, n);
				
		}
	}

	string str;
	while(!get_keyword(str)) {}	// wait end of block
}

void OMediaXSIConverter::scan_xform_matrix(void)
{
	short x,y;

	list<OMediaMatrix_4x4>::iterator mi,imi;

	mi = xform_matrix_stack.end();		mi--;
	imi = inv_xform_matrix_stack.end();	imi--;

	for(x=0;x<4;x++)
	for(y=0;y<4;y++)
	{
		(*mi).m[x][y] = get_float();
	}

	if (!(*mi).invert((*imi)))
	{
		(*mi).set_identity();
		(*imi).set_identity();
	}

	string str;
	while(!get_keyword(str)) {}	// wait end of block
}

void OMediaXSIConverter::transform_point(OMedia3DPoint &p)
{
	for(list<OMediaMatrix_4x4>::reverse_iterator mi = xform_matrix_stack.rbegin();
		mi!=xform_matrix_stack.rend();
		mi++)
	{
		(*mi).multiply(p);
	}
}   

void OMediaXSIConverter::transform_normal(OMedia3DVector &v)
{
	for(list<OMediaMatrix_4x4>::reverse_iterator mi = inv_xform_matrix_stack.rbegin();
		mi!=inv_xform_matrix_stack.rend();
		mi++)
	{
		(*mi).normal_transform(v,v);
	}
}
