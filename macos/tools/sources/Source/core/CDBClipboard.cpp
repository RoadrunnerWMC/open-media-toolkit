/*****************************************************************        O P E N      M E D I A     T O O L K I T              V2.5             Copyright Yves Schmid 1996-2003         See www.garagecube.com for more informations about this library.                Author(s): Yves Schmid         OMT is provided under LGPL:           This library is free software; you can redistribute it and/or          modify it under the terms of the GNU Lesser General Public          License as published by the Free Software Foundation; either          version 2.1 of the License, or (at your option) any later version.          This library is distributed in the hope that it will be useful,          but WITHOUT ANY WARRANTY; without even the implied warranty of          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          Lesser General Public License for more details.          You should have received a copy of the GNU Lesser General Public          License along with this library; if not, write to the Free Software          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          The full text of the license can be found in lgpl.txt          ******************************************************************/#include "CDBClipboard.h"#include "CTempStream.h"#include "UScrap.h"#include "CDBDocument.h"#include "AppConstants.h"#include "OMediaString.h"void CDBClipboard::reset(){	delete clip;	delete stream;	clip = NULL;	stream = NULL;	UScrap::ClearData();}void CDBClipboard::rebuild(){	delete clip;	delete stream;		clip = NULL;	stream = NULL;	stream = CTempStream::createTempStream();	clip = new OMediaFormattedStream(stream);}void CDBClipboard::copy(LDBHierTable *table, CDBDocument *doc){	cdbblock_Set	blockSet;	if (!table->hasSelection()) return;	reset();	rebuild();		// Build the list of IDs to copy		doc->selectionToBlockSet(blockSet);	// Copy now!	OMediaDataBase	*dbase = table->dbase;	OMediaFSChunk	*chk;	string			textDesc;	string			chkName;		for(cdbblock_Set::iterator si=blockSet.begin();		si!=blockSet.end();		si++)	{		chk = dbase->get_chunk((*si).ctype,(*si).id);		if (dbase->object_loaded((*si).ctype, (*si).id))		{			OMediaDBObject *obj;						obj = dbase->get_object((*si).ctype, (*si).id);			if (obj->get_modified()) obj->db_update();			obj->db_unlock();		}				clip->open_chunk((*si).ctype, (*si).id);		dbase->export_chunk((*si).ctype, (*si).id,clip);		dbase->get_chunk_name((*si).ctype, (*si).id, chkName);		clip->set_chunk_name((*si).ctype, (*si).id, chkName);				textDesc += (*si).desc.size()==0?"Untitled":(*si).desc;		textDesc += "\n";	}			// Fill the scrap with the TEXT and the OMTd code		unsigned long code = OMTDB_SCRAP_FLAVOR_TYPE;	UScrap::SetData(kScrapFlavorTypeText,textDesc.c_str(),textDesc.size());	UScrap::SetData(OMTDB_SCRAP_FLAVOR_TYPE,&code,4,false);}//++++	inline void set_chunk_name(omt_ChunkType type, omt_FSChunkID id, string newname)void CDBClipboard::cut(LDBHierTable *table, CDBDocument *doc){	copy(table,doc);	erase(table,doc);}void CDBClipboard::paste(LDBHierTable *table, CDBDocument *doc){	omt_ChunkTypeList::iterator ti;	omt_ChunkList::iterator ci;	OMediaFSChunk 			*dci;	bool					replaceAll = false,newIDAll=false;	omt_ChunkType			dtype;	omt_FSChunkID			did;	MessageT 				res;	string					s,d;	bool					cancelled;	bool					skip,killdest;	OMediaDataBase			*dbase;	if (!hasData()) return;	cancelled = false;	dbase = table->dbase;	for(ti=clip->get_chunk_type_list()->begin();		ti!=clip->get_chunk_type_list()->end() && !cancelled;		ti++)	{		for(ci=(*ti).get_chunk_list()->begin();			ci!=(*ti).get_chunk_list()->end();			ci++)		{			dtype = (*ti).get_chunk_type();			did = (*ci).id;						dci = dbase->get_chunk(dtype,did);						skip = killdest = false;					// Check if the dest. already exists			if (!replaceAll && (dci!=NULL) && !newIDAll)			{				string sn,dn;								sn = ((*ci).name.length()==0)?"Untitled":(*ci).name;				dn = ((*dci).name.length()==0)?"Untitled":(*dci).name;							s = omd_L2STR((*ci).id)+" - "+sn+" - "+doc->LStr255toString(doc->chunkTypeToString(dtype));				d = omd_L2STR((*dci).id)+" - "+dn+" - "+doc->LStr255toString(doc->chunkTypeToString(dtype));							res = AskForReplace(table,1101,s,d);				switch(res)				{					case msg_Cancel:					cancelled = true;					break;															case msgReplaceAll:					replaceAll = true;					case msgReplace:					killdest = true;					break;					case msgNewIDAll:					newIDAll = true;					case msgNewID:					did = dbase->get_unique_chunkid(dtype);					break;										case msgSkip:					skip = true;					break;				}			}			else if (dci!= NULL)			{				if (replaceAll) killdest = true;				if (newIDAll) did = dbase->get_unique_chunkid(dtype);				}						if (cancelled) break;			if (skip) continue;						if (killdest)			{				doc->objectWillDie(dtype,did);				dbase->delete_object(dtype,did);			}						clip->open_chunk(dtype,(*ci).id);			dbase->import_chunk(clip, clip->getsize(),dtype,did);			dci = dbase->get_chunk(dtype,did);			dbase->set_chunk_name(dtype,did,ci->name);		}		}	doc->SetModified(true);	table->rebuildTable();	doc->updateInfoCaptions();}void CDBClipboard::erase(LDBHierTable *table, CDBDocument *doc){	cdbblock_Set	blockSet;	if (!table->hasSelection()) return;		// Build the list of IDs to copy		doc->selectionToBlockSet(blockSet);	// Delete objects	for(cdbblock_Set::iterator si=blockSet.begin();		si!=blockSet.end();		si++)	{		doc->objectWillDie((*si).ctype,(*si).id);		table->dbase->delete_object((*si).ctype,(*si).id);	}	doc->SetModified(true);	table->rebuildTable();	doc->updateInfoCaptions();}bool CDBClipboard::hasData(){	return (UScrap::HasData(OMTDB_SCRAP_FLAVOR_TYPE) && clip!=NULL);}OMediaStream *CDBClipboard::stream;OMediaFormattedStream *CDBClipboard::clip;MessageT CDBClipboard::AskForReplace(LCommander*	inSuper,									ResIDT			inDialogID,									string			srcDesc,									string			destDesc){	StDialogHandler	theHandler(inDialogID, inSuper);	LWindow*		theDialog = theHandler.GetDialog();	LCaption		*src,*dest;	LStr255			pstr;	theDialog->SetThemeData(kThemeBackgroundPlacard, kThemeBackgroundPlacard,							 kThemeTextColorDialogActive, kThemeTextColorDialogInactive);	src = dynamic_cast<LCaption*>(theDialog->FindPaneByID(10));	dest = dynamic_cast<LCaption*>(theDialog->FindPaneByID(11));		src->SetDescriptor(pstr=srcDesc.c_str());	dest->SetDescriptor(pstr=destDesc.c_str());	theDialog->Show();	Boolean		entryOK = false;	MessageT	hitMessage;	for(;;)	{		hitMessage = theHandler.DoDialog();		if (hitMessage!=msg_Nothing) break;	}		return hitMessage;}