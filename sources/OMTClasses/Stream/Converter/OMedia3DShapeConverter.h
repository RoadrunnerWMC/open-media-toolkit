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
#ifndef OMEDIA_3DShapeConverter_H
#define OMEDIA_3DShapeConverter_H

#include "OMediaStreamOperators.h"
#include "OMedia3DShape.h"
#include "OMediaMatrix.h"

#include <vector>

class OMedia3DShape;
class OMedia3DMaterial;
class OMediaCanvas;
class OMedia3DMorphAnimDef;
class OMedia3DMorphAnimFrame;

// * Abstract converter

enum omt_3DMaterialConvertMode
{
	omc3dmconv_IgnoreMaterial,					// Does not associate material
	
	omc3dmconv_CreateNewMaterialIfNotFound,		// Create material if material name cannot
												// be found in the material database.
												// (Default)

	omc3dmconv_ExistingMaterialOnly,			// Look for material in the database.
												// If not found, set NULL.
	
	omc3dmconv_AlwaysCreateNewMaterial,			// Always create new material.

	omc3dmconv_ReplaceMaterial					// Replace material with same name
};

enum omt_3DObjectConvertMode
{
	omc3doconv_MergeObjects,					// All objects are merged into one shape
	omc3doconv_OneShapePerObject,				// One shape is created by object 
	omc3doconv_OneShapePerGroup					// One shape is created by group
};

enum omt_3DAnimConvertMode						
{
	omc3danconv_Ignore,							// Don't import animation (default)
	omc3danconv_ReplaceMorphAnimDef,			// Replace the object if the morph anim exists, else build a new one.
	omc3danconv_AlwaysCreateNewMorphAnimDef,	// Always generate a new morph definition.
	omc3danconv_AddSequence						// Add a new sequence to the morph definition.
};

enum omt_TextureConvertMode
{
	omctxtconv_Ignore,							// Do nothing (default)
	omctxtconv_ImportIfNotFound,				// Import if not found in DB, else use the DB one
	omctxtconv_Create,							// Import and create new canvases in DB
	omctxtconv_Replace							// Import and replace in DB
};

enum omt_WorldConvertMode
{
	omcworldconv_None,							// No world creation
	omcworldconv_3DElement						// Create a hierachy of world element 
};

class OMedia3DShapeConvMorphDef
{
	public:
	
	omt_FSChunkID		morph_def;
	omt_FSChunkID		its_shape;
};


typedef unsigned short omt_3DConvertFlags;
const omt_3DConvertFlags	om3dcf_MergeNormals				= (1<<0);	// Merge duplicated normals (do not use for animation)
const omt_3DConvertFlags	om3dcf_MergeVerticesNormals		= (1<<1);	// Merge duplicated normals per vertex (do not use for animation)
const omt_3DConvertFlags	om3dcf_MergeIdenticalMaterials	= (1<<2);	// Merge similar materials
const omt_3DConvertFlags	om3dcf_OneMaterialPerTexture	= (1<<3);	// Merge materials that have similar texture

class OMedia3DShapeConvTFileName
{
public:

	string			texture_filename;
	omt_FSChunkID	material_id;
};

class OMedia3DShapeConverter
{
	public:

	omtshared OMedia3DShapeConverter(OMediaStreamOperators *stream, OMedia3DShape *shape);
	omtshared virtual ~OMedia3DShapeConverter();	

	static OMedia3DShapeConverter *create_best_converter(OMediaStreamOperators *stream, OMedia3DShape *shape);

	static void set_material_convert_mode(omt_3DMaterialConvertMode cm) {mat_conv_mode = cm;}
	static omt_3DMaterialConvertMode get_material_convert_mode(void) {return mat_conv_mode;}

	static void set_object_convert_mode(omt_3DObjectConvertMode cm) {obj_conv_mode = cm;}
	static omt_3DObjectConvertMode get_object_convert_mode(void) {return obj_conv_mode;}

	static void set_convert_flags(omt_3DConvertFlags cm) {conv_flags = cm;}
	static omt_3DConvertFlags get_convert_flags(void) {return conv_flags;}
	static void add_convert_flags(omt_3DConvertFlags cm) {conv_flags |= cm;}
	static void rem_convert_flags(omt_3DConvertFlags cm) {conv_flags &= ~cm;}

	static void set_anim_conv_mode_mode(omt_3DAnimConvertMode cm) {anim_conv_mode = cm;}
	static omt_3DAnimConvertMode get_anim_conv_mode_mode(void) {return anim_conv_mode;}

	static void set_texture_conv_mode(omt_TextureConvertMode cm) {texture_conv_mode = cm;}
	static omt_TextureConvertMode get_texture_conv_mode(void) {return texture_conv_mode;}

	static void set_new_material_prefix(string str) {new_material_prefix = str;}
	static void get_new_material_prefix(string &str) {str = new_material_prefix;}

	static void set_new_texture_prefix(string str) {new_texture_prefix = str;}
	static void get_new_texture_prefix(string &str) {str = new_texture_prefix;}

	static void set_world_conv_mode(omt_WorldConvertMode cm) {world_conv_mode = cm;}
	static omt_WorldConvertMode get_world_conv_mode(void) {return world_conv_mode;}

	omtshared virtual void convert(void);

	omtshared static vector<omt_FSChunkID>				related_worldelements;
	omtshared static vector<omt_FSChunkID>				created_shapes;
	omtshared static vector<omt_FSChunkID>				related_materials;
	omtshared static vector<OMedia3DShapeConvMorphDef>	related_morphdefs;
	omtshared static vector<omt_FSChunkID>				related_textures;
	omtshared static vector<OMedia3DShapeConvTFileName>	related_texturefile;
	omtshared static bool								converter_used;

	omtshared static void merge_identical_materials(OMedia3DShape	*,omt_3DConvertFlags conv_flags);

	

	protected:

	omtshared virtual void finalize_shape(void);
	omtshared virtual bool start_new_shape(string name, bool finalize);

	omtshared virtual void generate_world(void);
        
        omtshared virtual void import_textures(void);
        omtshared virtual string texture_prefix(string filename);
        omtshared virtual void generate_texture_filenames(string path, string &filename, 
                                                            string &filename_noextend);
        
	omtshared static omt_3DMaterialConvertMode mat_conv_mode;
	omtshared static omt_3DObjectConvertMode obj_conv_mode;
	omtshared static omt_3DConvertFlags conv_flags;	
	omtshared static omt_3DAnimConvertMode	anim_conv_mode;
	omtshared static omt_TextureConvertMode texture_conv_mode;
	omtshared static omt_WorldConvertMode   world_conv_mode;
		
		
		
	omtshared static string new_material_prefix;
	omtshared static string new_texture_prefix;

	OMediaStreamOperators	*stream;
	OMedia3DShape 			*shape;
	OMedia3DMorphAnimDef	*anim_def;
	OMedia3DMorphAnimFrame	*morph_frame;

};



// * DXF converter

class OMediaDXFConverter : public OMedia3DShapeConverter
{
	public:

	omtshared OMediaDXFConverter(OMediaStreamOperators *stream, OMedia3DShape *shape);
	omtshared virtual ~OMediaDXFConverter();

	omtshared virtual void convert(void);

	omtshared virtual void finalize_shape(void);

	void skip_line(char *&b);
	void skip_space(char *&b);
	void get_line(char *&b, char *line);
	void scan_dxf(char *dxf, char *dxf_end);
	void scan_3dface(char *&dxf, char *line);

	void scan_3dvertex(char *&dxf, char *line);
	void scan_3dvertexface(char *&dxf, char *line);

	long dxf_index_base;
	char *dxf_end;
};



// * 3DSMax ASE converter

class OMediaASEMatConvert
{
	public:
	
	OMediaASEMatConvert() {mat=NULL; updated =false;}
	
	bool						updated;
	OMedia3DMaterial 			*mat;
	vector<OMedia3DMaterial*> 	sub_mat;
	vector<bool>			  	sub_updated;
};

class OMediaASEMatPolyID
{
	public:

	long	submat_id;
	long	poly;
};

class OMediaASEMatMap
{
	public:

	OMediaASEMatMap() {material = NULL; front_material = true;}
	
	OMedia3DMaterial	*material;
	
	bool	front_material;

	string	map_name,map_file;
	float	u_offset,v_offset;
	float	u_tiling,v_tiling;
};

class OMediaASEMapUVW
{
	public:
	
	float	u,v,w;
};

class OMediaASETempDuplicatedVertex
{
	public:
	long		original,copy;
	
	bool operator<(const OMediaASETempDuplicatedVertex &x) const
	{
		return copy<x.copy;
	}
};

class OMediaASEConverter : public OMedia3DShapeConverter
{
	public:

	omtshared OMediaASEConverter(OMediaStreamOperators *stream, OMedia3DShape *shape);
	omtshared virtual ~OMediaASEConverter();

	omtshared virtual void convert(void);

	omtshared virtual void finalize_shape(void);
	
	bool get_next_code(string &cmd, bool &block);

	void skip_string(void);
	void skip_block(void);
	void move_to_char(char c);

	void scan_ase(void);
	void scan_geomobject(void);
	void scan_mesh(void);
	void scan_vertex_list(void);
	void scan_face_list(void);
	void scan_normal_list(void);
	void scan_material_list(void);
	void scan_material(void);
	void scan_map(void);
	void scan_mapvertex(void);
	void scan_facevertex(void);
	void scan_group(void);
	void scan_colorvertex(void);
	void scan_colorface_list(void);
	void scan_animation(void);
	void scan_transform(void);

	
	void unlock_materials(bool set_color_vertex);
	void reject_bad_faces(void);
	void prepare_maps(void);
	void merge_duplicated_vertices(void);

	void get_number(string &nstr);
	float get_float(void);
	long get_integer(void);
	void get_string(string &str);
	void get_keyword(string &str);


	inline void advance_ptr(void)
	{
		ptr++;	if (ptr>=buffer_end) throw -1;
	}
	
	char	*buffer,*buffer_end;
	char	*ptr;
	
	long	vertex_mesh_base,face_mesh_base,color_mesh_base;
	bool	face_uvw_computed,normals_computed;
			
	vector<OMediaASEMatConvert>				materials;
	vector<OMediaASEMatPolyID>				polygon_materials;
	vector<OMediaASEMatMap>					maps;
	vector<OMediaASEMapUVW>					map_uvw;
	vector<OMediaASETempDuplicatedVertex>	dup_vertices;
	
	OMediaMatrix_4x4			transform_matrix,inv_transform_matrix;
	OMediaASEMatConvert			*super_mat;
	bool						two_sided;
	bool						first_shape;

	long						submat_count;
	long						group_level;
	string						super_mat_name;
	bool						vertex_color_mode;
	long						timevalue;
	long						animnorm_base_newvertices;

};


// * SoftImage .xsi converter

class OMediaXSIConverter : public OMedia3DShapeConverter
{
	public:

	omtshared OMediaXSIConverter(OMediaStreamOperators *stream, OMedia3DShape *shape);
	omtshared virtual ~OMediaXSIConverter();

	omtshared virtual void convert(void);

	omtshared virtual void finalize_shape(void);

	void move_to_block_start(void);
	bool get_keyword(string &str);
	void skip_block(void);
	float get_float(void);
	long get_integer(void);
	void get_number(string &nstr);
	void get_string(string &str);
	void move_to_char(char c);

	void scan_xform_matrix(void);
	void scan_normals(void);
	void scan_materials(void);
	void scan_text_coords(void);
	void scan_mesh_blocks(void);
	void scan_mesh_geometry(void);
	void scan_frame(void);
	void scan_xsi(void);

	void transform_point(OMedia3DPoint &p);
	void transform_normal(OMedia3DVector &v);

	OMediaCanvas *find_map(string map_name);

	inline void advance_ptr(void)
	{
		ptr++;	if (ptr>=buffer_end) throw -1;
	}

	char	*buffer,*buffer_end;
	char	*ptr;

	long	point_base_index,normal_base_index,polygon_base_index;

	list<OMediaMatrix_4x4>	xform_matrix_stack,inv_xform_matrix_stack;

	OMedia3DShape	*base_shape;
	long			first_mesh;
};



#endif
