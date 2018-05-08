/*****************************************************************        O P E N      M E D I A     T O O L K I T              V2.5             Copyright Yves Schmid 1996-2003         See www.garagecube.com for more informations about this library.                Author(s): Yves Schmid         OMT is provided under LGPL:           This library is free software; you can redistribute it and/or          modify it under the terms of the GNU Lesser General Public          License as published by the Free Software Foundation; either          version 2.1 of the License, or (at your option) any later version.          This library is distributed in the hope that it will be useful,          but WITHOUT ANY WARRANTY; without even the implied warranty of          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          Lesser General Public License for more details.          You should have received a copy of the GNU Lesser General Public          License along with this library; if not, write to the Free Software          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          The full text of the license can be found in lgpl.txt          ******************************************************************/#include "CWorldElementEditor.h"#include "AppConstants.h"#include "LOMTWindow.h"#include "LOMTWorldView.h"#include "OMedia3DMaterial.h"#include "OMedia3DShape.h"#include "OMedia3DShapeElement.h"#include "OMediaString.h"CWorldElementEditor::CWorldElementEditor(omt_FSChunkID id, CDBDocument *myDocument):				CAbstractEditor(OMediaElement::db_type,id,myDocument){	updateMode = false;	wv = NULL;	clickedShape = UNDEFINED_SHAPE;		element = (OMediaElement*)object;	window = LWindow::CreateWindow(2004, this );	window->SetThemeData(kThemeBackgroundPlacard, kThemeBackgroundPlacard,							 kThemeTextColorDialogActive, kThemeTextColorDialogInactive);	linkBroacasters();	setTitle();	((LOMTWindow*)window)->init_video_engine();	init_3dview();	updateUI();	window->Show();}CWorldElementEditor::~CWorldElementEditor(){	element->unlink();}void CWorldElementEditor::ListenToMessage(	MessageT		inMessage,							void*			ioParam){	bool modified = false;	if (updateMode) return;		switch(inMessage)	{		case msgViewPortChanged:		updateView();		break;			case msgViewPortPicked:		handleClick((OMediaPickRequest*)ioParam);		break;				case msgMsgEditFieldChanged:		processEditFieldChanged((LEditField*)ioParam);			break;						case 1024:	// Motion mode		switch(getControlVal(11))		{			case 1: wv->set_view_mode(CENTRED_VIEWLIGHT); break;			case 2: wv->set_view_mode(FREEMOTION_VIEW); break;		}		updateView();		break;				case 1025:	// Reset view value		resetView();		break;				case 1200:		if (clickedShape!=UNDEFINED_SHAPE)		{			omt_FSChunkID id = -1;			if (clickedShape) id = clickedShape->get_chunk_ID();						if (myDocument->pickObject(window, OMedia3DShape::db_type, id))			{				OMedia3DShape *sh = omd_GETOBJECT(myDocument->dbase,OMedia3DShape,id);								clickedShapeElement->set_shape(sh);								clickedShape = sh;				updateClickedShape();				sh->db_unlock();				modified = true;			}		}		break;		case 1201:		if (clickedShape!=UNDEFINED_SHAPE)		{			clickedShapeElement->set_shape(NULL);			clickedShape = NULL;					updateClickedShape();			modified = true;		}		break;		case 1202:		if (clickedShape!=UNDEFINED_SHAPE)		{			if (clickedShape) 				myDocument->edit(OMedia3DShape::db_type,								 clickedShape->get_chunk_ID());		}		break;			}	if (modified)	{		if (wv) wv->Refresh();		myDocument->SetModified(true);		element->set_modified(true);	}}void CWorldElementEditor::linkBroacasters(){	int i;	for(i=51;i<=53;i++) addControlListener(i);	for(i=12;i<=18;i++) addMsgEditFieldListener(i);	addControlListener(19);		addControlListener(11);}void CWorldElementEditor::updateUI(){	updateInfo();	updateView();	updateClickedShape();}void CWorldElementEditor::updateInfo(){	long nPolygons, nVertices, nElements;	findInfo(nPolygons, nVertices, nElements);	setCaptionDesc(70, omd_L2STR(nPolygons));	setCaptionDesc(71, omd_L2STR(nVertices));	setCaptionDesc(72, omd_L2STR(nElements));}void CWorldElementEditor::updateView(){	updateMode = true;	updatePosAngleFields();		wv->UpdateViewport();		setEditFieldFloat(12,wv->viewport->getx());	setEditFieldFloat(13,wv->viewport->gety());	setEditFieldFloat(14,wv->viewport->getz());	setEditFieldFloat(15,omd_Angle2DegF(wv->viewport->get_anglex()));	setEditFieldFloat(16,omd_Angle2DegF(wv->viewport->get_angley()));	setEditFieldFloat(17,omd_Angle2DegF(wv->viewport->get_anglez()));	setEditFieldFloat(18,wv->motionSpeed);	updateMode = false;}void CWorldElementEditor::processEditFieldChanged(LEditField *field){	int id = field->GetPaneID();	double d;	getEditFieldFloat(id,d);	bool updateAngle = false;		switch(id)	{		case 12: wv->viewport->setx(d); break;		case 13: wv->viewport->sety(d); break;		case 14: wv->viewport->setz(d); break;		case 15: updateAngle = true; break;		case 16: updateAngle = true; break;		case 17: updateAngle = true; break;		case 18: wv->motionSpeed = d; break;	}		if (updateAngle)	{		double ax,ay,az;		getEditFieldFloat(15,ax);		getEditFieldFloat(16,ay);		getEditFieldFloat(17,az);				ax = omd_Deg2AngleF(ax);		ay = omd_Deg2AngleF(ay);		az = omd_Deg2AngleF(az);				wv->axisShape.reset_axis();		wv->axisShape.get_axis(omc3daxis_X).rotate(ax,ay,az);		wv->axisShape.get_axis(omc3daxis_Y).rotate(ax,ay,az);		wv->axisShape.get_axis(omc3daxis_Z).rotate(ax,ay,az);				wv->axisLight.reset_axis();		wv->axisLight.get_axis(omc3daxis_X).rotate(ax,ay,az);				wv->axisLight.get_axis(omc3daxis_Y).rotate(ax,ay,az);				wv->axisLight.get_axis(omc3daxis_Z).rotate(ax,ay,az);			}		wv->Refresh();}void CWorldElementEditor::resetView(){	if (wv->view_Mode==FREEMOTION_VIEW)	{		setEditFieldFloat(12,0);		setEditFieldFloat(13,0);		setEditFieldFloat(14,0);		setEditFieldFloat(15,0);		setEditFieldFloat(16,0);		setEditFieldFloat(17,0);		setEditFieldFloat(18,2);		wv->axisShape.reset_axis();		wv->axisLight.reset_axis();	}	else	{		wv->vp_distance = findElementBiggestRadius()*-2.0f;		wv->axisShape.reset_axis();		wv->axisLight.reset_axis();	}		wv->Refresh();}void CWorldElementEditor::init_3dview(){	LOMTWindow		*omw = (LOMTWindow*)window;		wv = dynamic_cast<LOMTWorldView*>(window->FindPaneByID(80)); Assert_(wv != nil);	wv->prepare(omw);	wv->pickingOn = true;			element->link(wv->world);		wv->vp_distance = findElementBiggestRadius()*-2.0f;	wv->layer->add_flags(omlayerf_ClearZBuffer|omlayerf_EnableZBufferWrite|omlayerf_EnableZBufferTest);	wv->AddListener(this);}void CWorldElementEditor::objectWillDie(omt_ChunkType dtype, omt_FSChunkID did){	if (dtype==OMedia3DShape::db_type && clickedShape!=NULL && clickedShape!=UNDEFINED_SHAPE)	{		if (clickedShape->get_chunk_ID()==did)		{			clickedShape=UNDEFINED_SHAPE;			updateClickedShape();			if (wv) wv->Refresh();		}	}}void CWorldElementEditor::objectRenamed(omt_ChunkType dtype, omt_FSChunkID did){	if (dtype==OMedia3DShape::db_type && clickedShape!=NULL && clickedShape!=UNDEFINED_SHAPE)	{		if (clickedShape->get_chunk_ID()==did)		{			updateObjectPickZone(50,clickedShape);			if (wv) wv->Refresh();		}	}}void CWorldElementEditor::updateClickedShape(){	int baseID = 50;	if (clickedShape==UNDEFINED_SHAPE)	{		LControl *c;		c = dynamic_cast<LControl*>(window->FindPaneByID(baseID+1)); Assert_(c != nil);		c->Disable();		c = dynamic_cast<LControl*>(window->FindPaneByID(baseID+2)); Assert_(c != nil);		c->Disable();		c = dynamic_cast<LControl*>(window->FindPaneByID(baseID+3)); Assert_(c != nil);		c->Disable();		setCaptionDesc(baseID,"- NO SHAPE PICKED -");			}	else	{		updateObjectPickZone(baseID, clickedShape);	} }void CWorldElementEditor::handleClick(OMediaPickRequest *request){	clickedShape = UNDEFINED_SHAPE;	if (request->closer_hit.type==omptc_Element &&		request->closer_hit.shape!=NULL)	{		clickedShape = request->closer_hit.shape;		if (clickedShape) clickedShapeElement = dynamic_cast<OMedia3DShapeElement*>(request->closer_hit.element);	}		updateClickedShape();}float CWorldElementEditor::findElementBiggestRadius(void){	return findElementBiggestRadius(element->get_element_list(),0);}float CWorldElementEditor::findElementBiggestRadius(omt_ElementList *elist, float r){	float	nr;	for(omt_ElementList::iterator ei=elist->begin();		ei!=elist->end();		ei++)	{		nr = findElementBiggestRadius((*ei)->get_element_list(),r);		if (nr>r) r=nr;				OMedia3DShapeElement	*es;		es = dynamic_cast<OMedia3DShapeElement*>(*ei);		if (es)		{			OMedia3DShape	*sh = es->get_shape();			if (sh)			{				nr = sh->get_radius();				if (nr>r) r=nr;			}				}		}		return r;}void CWorldElementEditor::findInfo(long &nPolygons, long &nVertices, long &nElements){	nPolygons = 0;	nVertices = 0;	nElements = 1;	findInfo(element->get_element_list(),nPolygons, nVertices, nElements);}void CWorldElementEditor::findInfo(omt_ElementList *elist, long &nPolygons, long &nVertices, long &nElements){	nElements+=elist->size();	for(omt_ElementList::iterator ei=elist->begin();		ei!=elist->end();		ei++)	{			findInfo((*ei)->get_element_list(),nPolygons,nVertices,nElements);				OMedia3DShapeElement	*es;		es = dynamic_cast<OMedia3DShapeElement*>(*ei);		if (es)		{			OMedia3DShape	*sh = es->get_shape();			if (sh)			{				nPolygons += sh->get_polygons()->size();				nVertices += sh->get_vertices()->size();			}				}		}	}void CWorldElementEditor::updatePosAngleFields(){	LEditField *e;		for(int i=12;i<=18;i++)	{		e = dynamic_cast<LEditField*>(window->FindPaneByID(i)); Assert_(e != nil);		if (wv->view_Mode==FREEMOTION_VIEW) e->Enable();		else e->Disable();			}	}