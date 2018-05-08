/*****************************************************************        O P E N      M E D I A     T O O L K I T              V2.5             Copyright Yves Schmid 1996-2003         See www.garagecube.com for more informations about this library.                Author(s): Yves Schmid         OMT is provided under LGPL:           This library is free software; you can redistribute it and/or          modify it under the terms of the GNU Lesser General Public          License as published by the Free Software Foundation; either          version 2.1 of the License, or (at your option) any later version.          This library is distributed in the hope that it will be useful,          but WITHOUT ANY WARRANTY; without even the implied warranty of          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          Lesser General Public License for more details.          You should have received a copy of the GNU Lesser General Public          License along with this library; if not, write to the Free Software          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          The full text of the license can be found in lgpl.txt          ******************************************************************/#include "CMaterialEditor.h"#include "AppConstants.h"#include <LGAColorSwatchControl.h>#include "LOMTWindow.h"#include "LOMTWorldView.h"#include "OMedia3DMaterial.h"#include "OMedia3DShape.h"#include "OMedia3DShapeElement.h"CMaterialEditor::CMaterialEditor(omt_FSChunkID id, CDBDocument *myDocument):				CAbstractEditor(OMedia3DMaterial::db_type,id,myDocument){		updateMode = false;	wv = NULL;		mat = (OMedia3DMaterial*)object;	window = LWindow::CreateWindow(2002, this );	window->SetThemeData(kThemeBackgroundPlacard, kThemeBackgroundPlacard,							 kThemeTextColorDialogActive, kThemeTextColorDialogInactive);	linkBroacasters();	updateUI();	setTitle();	((LOMTWindow*)window)->init_video_engine();	init_3dview();	window->Show();}CMaterialEditor::~CMaterialEditor(){	delete sphere;}void CMaterialEditor::init_3dview(){	LOMTWindow		*omw = (LOMTWindow*)window;		wv = dynamic_cast<LOMTWorldView*>(window->FindPaneByID(80)); Assert_(wv != nil);	wv->prepare(omw);		sphere = new OMedia3DShape;	sphere->make_sphere(40,40,60);	sphere->set_material(mat);//	sphere->set_two_sided(true);		e_sphere = new OMedia3DShapeElement;	e_sphere->link(wv->world);	e_sphere->place(0,0,0);	e_sphere->set_shape(sphere);		wv->mouseRotateShape = e_sphere;}void CMaterialEditor::linkBroacasters(){	int i;	for(i=30;i<=33;i++) addMsgEditFieldListener(i);	for(i=40;i<=43;i++) addMsgEditFieldListener(i);	for(i=50;i<=53;i++) addMsgEditFieldListener(i);	for(i=60;i<=63;i++) addMsgEditFieldListener(i);		addMsgEditFieldListener(70);	addControlListener(11);	addControlListener(12);	addControlListener(13);	addControlListener(14);	addControlListener(15);	addControlListener(20);	addControlListener(21);	addControlListener(22);	addControlListener(23);	addControlListener(24);	addControlListener(34);	addControlListener(44);	addControlListener(54);	addControlListener(64);}void CMaterialEditor::updateARGB(int idBase, OMediaFARGBColor argb){	setEditFieldInt(idBase,argb.alpha*255);	setEditFieldInt(idBase+1,argb.red*255);	setEditFieldInt(idBase+2,argb.green*255);	setEditFieldInt(idBase+3,argb.blue*255);	setColorSwatch(idBase+4,argb);}void CMaterialEditor::updateUI(){	updateMode = true;	updateARGB(30,mat->get_emission_ref());	updateARGB(40,mat->get_diffuse_ref());	updateARGB(50,mat->get_specular_ref());	updateARGB(60,mat->get_ambient_ref());	setEditFieldFloat(70,mat->get_shininess());	updateObjectPickZone(10, mat->get_texture());	setBlendPopup(23, mat->get_blend_src());	setBlendPopup(24, mat->get_blend_dest());		switch(mat->get_fill_mode())	{		case omfillmc_Point:	setControlVal(20,1);	break;		case omfillmc_Line:		setControlVal(20,2);	break;		case omfillmc_Solid:	setControlVal(20,3);	break;	}	switch(mat->get_shade_mode())	{		case omshademc_Flat:	setControlVal(21,1);	break;		case omshademc_Gouraud:	setControlVal(21,2);	break;	}	switch(mat->get_light_mode())	{		case ommlmc_Color:			setControlVal(22,1);	break;		case ommlmc_VertexColor:	setControlVal(22,2);	break;		case ommlmc_Light:			setControlVal(22,3);	break;	}	switch(mat->get_texture_address_mode())	{		case omtamc_Clamp:			setControlVal(14,1);	break;		case omtamc_Wrap:			setControlVal(14,2);	break;	}	switch(mat->get_texture_color_operation())	{		case omtcoc_Replace:		setControlVal(15,1);	break;		case omtcoc_Modulate:		setControlVal(15,2);	break;	}	updateMode = false;}void CMaterialEditor::ListenToMessage(	MessageT		inMessage,								void*			ioParam){	bool modified = false;	if (updateMode) return;	SInt32	*value = (SInt32*)ioParam;		if (inMessage>=1300 && inMessage<=1303)	{		processColorSwatchChanged(inMessage);		modified = true;		}	switch(inMessage)	{		case msgMsgEditFieldChanged:		processEditFieldChanged((LEditField*)ioParam);			modified = true;		break;				case 1024:		switch(getControlVal(20))		{			case 1: mat->set_fill_mode(omfillmc_Point); break;			case 2: mat->set_fill_mode(omfillmc_Line); break;			case 3: mat->set_fill_mode(omfillmc_Solid); break;		}		modified = true;		break;		case 1025:		switch(getControlVal(21))		{			case 1: mat->set_shade_mode(omshademc_Flat); break;			case 2: mat->set_shade_mode(omshademc_Gouraud); break;		}		modified = true;		break;		case 1026:		switch(getControlVal(22))		{			case 1: mat->set_light_mode(ommlmc_Color); break;			case 2: mat->set_light_mode(ommlmc_VertexColor); break;			case 3: mat->set_light_mode(ommlmc_Light); break;		}		modified = true;		break;		case 1027:		mat->set_blend(getBlendPopup(23),mat->get_blend_dest());		modified = true;		break;		case 1028:		mat->set_blend(mat->get_blend_src(),getBlendPopup(24));		modified = true;		break;				case 1200:		{			omt_FSChunkID id = -1;			if (mat->get_texture()) id = mat->get_texture()->get_chunk_ID();						if (myDocument->pickObject(window, OMediaCanvas::db_type, id))			{				OMediaCanvas *canv = omd_GETOBJECT(myDocument->dbase,OMediaCanvas,id);				mat->set_texture(canv);				updateObjectPickZone(10,canv);				canv->db_unlock();				modified = true;			}		}		break;		case 1201:		mat->set_texture(NULL);		updateObjectPickZone(10,NULL);		modified = true;		break;		case 1202:		{			if (mat->get_texture()) 				myDocument->edit(OMediaCanvas::db_type,								 mat->get_texture()->get_chunk_ID());		}		break;		case 1204:		switch(getControlVal(14))		{			case 1:			mat->set_texture_address_mode(omtamc_Clamp);			break;			case 2:			mat->set_texture_address_mode(omtamc_Wrap);			break;					}				modified = true;		break;				case 1205:		switch(getControlVal(14))		{			case 1:			mat->set_texture_color_operation(omtcoc_Replace);			break;			case 2:			mat->set_texture_color_operation(omtcoc_Modulate);			break;					}				modified = true;		break;	}	if (modified)	{		if (wv) wv->Refresh();		myDocument->SetModified(true);		mat->set_modified(true);	}}void CMaterialEditor::processColorSwatchChanged(MessageT inMessage){	LGAColorSwatchControl	*c;	int						id;	RGBColor				rgb;	OMediaFARGBColor		frgb;		switch(inMessage)	{		case 1300: id = 34; break;		case 1301: id = 44; break;		case 1302: id = 54; break;		case 1303: id = 64; break;		default: return;	}		c = dynamic_cast<LGAColorSwatchControl*>(window->FindPaneByID(id)); Assert_(c != nil);	c->GetSwatchColor(rgb);	frgb.red = rgb.red / (float)0xFFFF;	frgb.green = rgb.green / (float)0xFFFF;	frgb.blue = rgb.blue / (float)0xFFFF;		updateMode = true;		switch(inMessage)	{		case 1300: frgb.alpha = mat->get_emission_a(); mat->set_emission(frgb); updateARGB(30,mat->get_emission_ref()); break;		case 1301: frgb.alpha = mat->get_diffuse_a(); mat->set_diffuse(frgb); updateARGB(40,mat->get_diffuse_ref()); break;		case 1302: frgb.alpha = mat->get_specular_a(); mat->set_specular(frgb); updateARGB(50,mat->get_specular_ref()); break;		case 1303: frgb.alpha = mat->get_ambient_a(); mat->set_ambient(frgb); updateARGB(60,mat->get_ambient_ref()); break;	}	updateMode = false;}void CMaterialEditor::processEditFieldChanged(LEditField *field){	int id = field->GetPaneID(),cs;	OMediaFARGBColor	*col;		if (id==70)	{		double d;		getEditFieldFloat(70,d);		mat->set_shininess(d);		return;	}		if (id>=30 && id<=33) {col = &mat->get_emission_ref(); id-=30; cs = 34;}	else if (id>=40 && id<=43) {col = &mat->get_diffuse_ref(); id-=40; cs = 44;}	else if (id>=50 && id<=53) {col = &mat->get_specular_ref(); id-=50; cs = 54;}	else if (id>=60 && id<=63) {col = &mat->get_ambient_ref(); id-=60; cs = 64;}	else return;		float val = field->GetValue();	if (val>255.0f) val = 255.0f;	if (val<-255.0f) val = -255.0f;		switch(id)	{		case 0:	col->alpha = val / 255.0f;		break;		case 1:	col->red = val / 255.0f;		break;		case 2:	col->green = val / 255.0f;		break;		case 3:	col->blue = val / 255.0f;		break;	}		updateMode = true;	setColorSwatch(cs,  *col);	updateMode = false;}void CMaterialEditor::objectWillDie(omt_ChunkType dtype, omt_FSChunkID did){	if (dtype==OMediaCanvas::db_type)	{		if (mat->get_texture() && mat->get_texture()->get_chunk_ID()==did)		{			mat->set_texture(NULL);			updateObjectPickZone(10,NULL);			if (wv) wv->Refresh();		}		}	}void CMaterialEditor::objectRenamed(omt_ChunkType dtype, omt_FSChunkID did){	if (dtype==OMediaCanvas::db_type)	{		if (mat->get_texture() && mat->get_texture()->get_chunk_ID()==did)		{			updateObjectPickZone(10,mat->get_texture());			if (wv) wv->Refresh();		}		}	}