/*****************************************************************        O P E N      M E D I A     T O O L K I T              V2.5             Copyright Yves Schmid 1996-2003         See www.garagecube.com for more informations about this library.                Author(s): Yves Schmid         OMT is provided under LGPL:           This library is free software; you can redistribute it and/or          modify it under the terms of the GNU Lesser General Public          License as published by the Free Software Foundation; either          version 2.1 of the License, or (at your option) any later version.          This library is distributed in the hope that it will be useful,          but WITHOUT ANY WARRANTY; without even the implied warranty of          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          Lesser General Public License for more details.          You should have received a copy of the GNU Lesser General Public          License along with this library; if not, write to the Free Software          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          The full text of the license can be found in lgpl.txt          ******************************************************************/#include "CCanvasEditor.h"#include "OMediaCanvas.h"#include "OMediaString.h"#include "LCanvasView.h"#include "LMsgEditField.h"#include "LCaption.h"#include "LStdControl.h"#include "AppConstants.h"CCanvasEditor::CCanvasEditor(omt_FSChunkID id, CDBDocument *myDocument):				CAbstractEditor(OMediaCanvas::db_type,id,myDocument){	initMode = true;	canvas = (OMediaCanvas*)object;		window = LWindow::CreateWindow(2000, this );	ThrowIfNil_(window);	window->SetThemeData(kThemeBackgroundPlacard, kThemeBackgroundPlacard,							 kThemeTextColorDialogActive, kThemeTextColorDialogInactive);	view = dynamic_cast<LCanvasView*>(window->FindPaneByID(10));	Assert_(view != nil);	view->SetCanvas(canvas);	setTitle();	initUI();	window->Show();	initMode = false;}CCanvasEditor::~CCanvasEditor(){	}void CCanvasEditor::initUI(){	LMsgEditField		*e;	LCaption		*c;	LStr255			lstr;	LStdPopupMenu	*p;		c = dynamic_cast<LCaption*>(window->FindPaneByID(30)); Assert_(c != nil);	lstr = "Width: ";	lstr += canvas->get_width();	c->SetDescriptor(lstr);	c = dynamic_cast<LCaption*>(window->FindPaneByID(31)); Assert_(c != nil);	lstr = "Height: ";	lstr += canvas->get_width();	c->SetDescriptor(lstr);	e = divwidthEF = dynamic_cast<LMsgEditField*>(window->FindPaneByID(35)); Assert_(c != nil);	lstr = canvas->get_2Dsubdivision_width();	e->SetDescriptor(lstr);	e->AddListener(this);	e = divheightEF = dynamic_cast<LMsgEditField*>(window->FindPaneByID(36)); Assert_(c != nil);	lstr = canvas->get_2Dsubdivision_height();	e->SetDescriptor(lstr);	e->AddListener(this);	minfiltPM = p = dynamic_cast<LStdPopupMenu*>(window->FindPaneByID(32)); Assert_(c != nil);	p->AddListener(this);	switch(canvas->get_filtering_min())	{		case omtfc_Nearest: p->SetValue(1); break;		case omtfc_Linear: p->SetValue(2); break;		case omtfc_Nearest_Mipmap_Nearest: p->SetValue(3); break;		case omtfc_Nearest_Mipmap_Linear: p->SetValue(4); break;		case omtfc_Linear_Mipmap_Nearest: p->SetValue(5); break;		case omtfc_Linear_Mipmap_Linear: p->SetValue(6); break;		}	magfiltPM = p = dynamic_cast<LStdPopupMenu*>(window->FindPaneByID(33)); Assert_(c != nil);	p->AddListener(this);	switch(canvas->get_filtering_mag())	{		case omtfc_Nearest: p->SetValue(1); break;		case omtfc_Linear: p->SetValue(2); break;	}		pixformPM = p = dynamic_cast<LStdPopupMenu*>(window->FindPaneByID(34)); Assert_(c != nil);	p->AddListener(this);	switch(canvas->get_internal_pixel_format())	{		case ompixfc_Best: 				p->SetValue(1); break;		case ompixfc_ResBest: 			p->SetValue(2); break;		case ompixfc_ResBestAlpha: 		p->SetValue(3); break;		case ompixfc_ResBestAlpha1bit: 	p->SetValue(4); break;		default:						p->SetValue(5); break;	}}void CCanvasEditor::ListenToMessage(MessageT		inMessage,									void*			ioParam){	bool modified = false;	if (initMode) return;	switch(inMessage)	{		case msgPixFormatPopupChanged:		{			switch(pixformPM->GetValue())				{				case 1:	canvas->set_internal_pixel_format(ompixfc_Best); break;				case 2:	canvas->set_internal_pixel_format(ompixfc_ResBest); break;				case 3:	canvas->set_internal_pixel_format(ompixfc_ResBestAlpha); break;				case 4:	canvas->set_internal_pixel_format(ompixfc_ResBestAlpha1bit); break;							}						modified = true;		}		break;				case msgMinFilterPopupChanged:		{			switch(minfiltPM->GetValue())			{				case 1:	canvas->set_filtering(canvas->get_filtering_mag(),omtfc_Nearest); break;				case 2:	canvas->set_filtering(canvas->get_filtering_mag(),omtfc_Linear); break;				case 3:	canvas->set_filtering(canvas->get_filtering_mag(),omtfc_Nearest_Mipmap_Nearest); break;				case 4:	canvas->set_filtering(canvas->get_filtering_mag(),omtfc_Nearest_Mipmap_Linear); break;				case 5:	canvas->set_filtering(canvas->get_filtering_mag(),omtfc_Linear_Mipmap_Nearest); break;				case 6:	canvas->set_filtering(canvas->get_filtering_mag(),omtfc_Linear_Mipmap_Linear); break;			}							modified = true;		}		break;					case msgMagFilterPopupChanged:		{			switch(magfiltPM->GetValue())			{				case 1:	canvas->set_filtering(omtfc_Nearest,canvas->get_filtering_min()); break;				case 2:	canvas->set_filtering(omtfc_Linear,canvas->get_filtering_min()); break;			}			modified = true;		}		break;				case msgMsgEditFieldChanged:		{			if ((LEditField*)ioParam==divwidthEF)			{				udapteDivSize((LEditField*)ioParam);						}			else if ((LEditField*)ioParam==divheightEF)			{				udapteDivSize((LEditField*)ioParam);						}						modified = true;		}		break;	}	if (modified)	{		myDocument->SetModified(true);		canvas->set_modified(true);	}}static unsigned long getnearestpow(unsigned long n){	unsigned long bit;	for(bit= 32;;)	{		bit--;				if (n&(1<<bit)) return (1<<bit);		if (bit==0) break;	}		return 0;}void CCanvasEditor::udapteDivSize(LEditField *field){	Str255	pstr;	long	val;	field->GetDescriptor(pstr);		val = omd_STR2L(myDocument->Str255toString(pstr));	if (val<=0)	{		val = -1;		}	else	{		// Must be a power of 2				val = getnearestpow(val);		}		if (field==divwidthEF) canvas->set_2Dsubdivision(val,canvas->get_2Dsubdivision_height());	if (field==divheightEF) canvas->set_2Dsubdivision(canvas->get_2Dsubdivision_width(),val);}