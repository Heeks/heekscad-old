// Solid.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Shape.h"
#include <TopoDS_Solid.hxx>

class CSolid:public CShape{
protected:
	static wxIcon* m_icon;

public:
	SolidTypeEnum m_type; // so the solid can be stretched in specific ways, if it's still a primitive solid

	CSolid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col);
	CSolid(const HeeksColor& col);
	~CSolid();

	virtual const CSolid& operator=(const CSolid& s){ return *this;}

	int GetType()const{return SolidType;}
	long GetMarkingMask()const{return MARKING_FILTER_SOLID;}
	const wxChar* GetTypeString(void)const{return _("Solid");}
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/icons/solid");}
	HeeksObj *MakeACopy(void)const;
	void SetColor(const HeeksColor &col){m_color = col;}
	const HeeksColor* GetColor()const{return &m_color;}
	bool ModifyByMatrix(const double* m);
	void OnApplyProperties();

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	virtual SolidTypeEnum GetSolidType(){return SOLID_TYPE_UNKNOWN;}
};
