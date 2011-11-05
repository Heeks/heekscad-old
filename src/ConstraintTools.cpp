// ConstraintTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#ifdef MULTIPLE_OWNERS
#include "ConstraintTools.h"
#include "MarkedList.h"
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "Sketch.h"
#include "SolveSketch.h"
#include "MultiPoly.h"
#include "DimensionDrawing.h"
#include "HAngularDimension.h"

class SetLinesPerpendicular:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				obj->SetPerpendicularConstraint(last);
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Perpendicular");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be perpendicular");}
};

class SetLineTangent:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		ConstrainedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			ConstrainedObject* obj = (ConstrainedObject*)*It;
			if(last)
			{
				if(last->GetType() == LineType)
					obj->SetTangentConstraint(last);
				else
					last->SetTangentConstraint(obj);
			}
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Toggle Tangent");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set line tangent to arc");}
};


class SetLinesParallel:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				if(obj->SetParallelConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Parallel");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be parallel");}
};

class SetLinesEqualLength:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				if(obj->SetEqualLengthConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Equal Length");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be equal length");}
};

class SetLinesColinear:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				if(obj->SetColinearConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Colinear");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be colinear");}
};

class SetEqualRadius:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		ConstrainedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			ConstrainedObject* obj = (ConstrainedObject*)*It;
			if(last)
				if(obj->SetEqualRadiusConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Equal Radius");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these arcs to have equal radii");}
};

class SetConcentric:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		ConstrainedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			ConstrainedObject* obj = (ConstrainedObject*)*It;
			if(last)
				if(obj->SetConcentricConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Concentric");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these arcs to be concentric");}
};

class SetPointsCoincident:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		HPoint* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			HPoint* obj = (HPoint*)*It;
			if(last)
			{
				obj->SetCoincidentPoint(last,true);
				break;
			}
			last=obj;
		}
		SolveSketch((CSketch*)last->HEEKSOBJ_OWNER->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Coincident");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these points to be coincident");}
};

class AddDimension:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		HPoint* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			HPoint* obj = (HPoint*)*It;
			if(last)
			{
				HDimension* dimension = new HDimension(gp_Trsf(), last->m_p, obj->m_p, gp_Pnt(), TwoPointsDimensionMode, PythagoreanDimensionTextMode, DimensionUnitsGlobal, &wxGetApp().current_color);
				last->SetCoincidentPoint(dimension->A,true);
				obj->SetCoincidentPoint(dimension->B,true);
				dimension_drawing.StartOnStep3(dimension);
				break;
			}
			last=obj;
		}
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Add Dimension");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add a dimension object");}
};

class AddAngularDimension:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		HLine* line1=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			HLine* line2 = (HLine*)*It;
			if(line1)
			{
				HAngularDimension* dimension = new HAngularDimension(wxString(), line1->A->m_p, line1->B->m_p, line2->A->m_p, line2->B->m_p, gp_Pnt(1,1,0), DegreesAngularDimensionTextMode, &wxGetApp().current_color);
				line1->A->SetCoincidentPoint(dimension->m_p0,true);
				line1->B->SetCoincidentPoint(dimension->m_p1,true);
				line2->A->SetCoincidentPoint(dimension->m_p2,true);
				line2->B->SetCoincidentPoint(dimension->m_p3,true);
				
				//dimension_drawing.StartOnStep3(dimension);
				line1->HEEKSOBJ_OWNER->Add(dimension,NULL);
				break;
			}
			line1=line2;
		}
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Add Angular Dimension");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add an angular dimension object");}
};


class AddRadiusDimension:public Tool{
public:
	void Run(){
		HCircle *circle = (HCircle*)*wxGetApp().m_marked_list->list().begin();
		HDimension* dimension = new HDimension(gp_Trsf(), circle->C->m_p, circle->C->m_p.XYZ() + circle->m_radius * gp_XYZ(1,0,0), gp_Pnt(), TwoPointsDimensionMode, PythagoreanDimensionTextMode, DimensionUnitsGlobal, &wxGetApp().current_color);
		circle->C->SetCoincidentPoint(dimension->A,true);
		circle->SetPointOnCircleConstraint(dimension->B);
		dimension_drawing.StartOnStep3(dimension);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Add Dimension");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add a dimension object");}
};

class AddArcRadiusDimension:public Tool{
public:
	void Run(){
		HArc *arc = (HArc*)*wxGetApp().m_marked_list->list().begin();
		HDimension* dimension = new HDimension(gp_Trsf(), arc->C->m_p, arc->C->m_p.XYZ() + arc->m_radius * gp_XYZ(1,0,0), gp_Pnt(), TwoPointsDimensionMode, PythagoreanDimensionTextMode, DimensionUnitsGlobal, &wxGetApp().current_color);
		arc->C->SetCoincidentPoint(dimension->A,true);
		arc->SetPointOnArcConstraint(dimension->B);
		dimension_drawing.StartOnStep3(dimension);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Add Dimension");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add a dimension object");}
};

class SetPointOnLine:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnLineConstraint(point);

		SolveSketch((CSketch*)line->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Point On Line");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on line");}
};

class SetPointOnMidpoint:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnLineMidpointConstraint(point);

		SolveSketch((CSketch*)line->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Point On Midpoint");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on the midpoint of line");}
};

class SetPointOnArcMidpoint:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnArcMidpointConstraint(point);

		SolveSketch((CSketch*)line->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Point On Midpoint");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on the midpoint of arc");}
};

class SetPointOnArc:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnArcConstraint(point);

		SolveSketch((CSketch*)line->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Point On Arc");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on arc");}
};

class SetPointOnCircle:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		HCircle* circle=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			HCircle* obj = dynamic_cast<HCircle*>(*It);
			if(obj)
				circle=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		circle->SetPointOnCircleConstraint(point);

		SolveSketch((CSketch*)circle->HEEKSOBJ_OWNER);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _("Set Point On Circle");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on circle");}
};


void Test2DNearMap();

class RunTest:public Tool{
public:
	void Run(){
		std::list<CSketch*> list;
		list.push_back((CSketch*)(*wxGetApp().m_marked_list->list().begin())->HEEKSOBJ_OWNER);
		MultiPoly(list);
	//	Test2DNearMap();
	//	TestMatrixFac();
	}
	const wxChar* GetTitle(){return _("Run Test");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Run Test");}
};


static SetEqualRadius set_equal_radius;
static SetConcentric set_concentric;
static SetLinesParallel set_lines_parallel;
static SetLinesEqualLength set_lines_equal_length;
static SetLinesColinear set_lines_colinear;
static SetLinesPerpendicular set_lines_perpendicular;
static SetLineTangent set_line_tangent;
static SetPointOnLine set_point_on_line;
static SetPointOnMidpoint set_point_on_midpoint;
static SetPointsCoincident set_points_coincident;
static AddDimension add_dimension;
static AddAngularDimension add_angular_dimension;
static AddRadiusDimension add_radius_dimension;
static AddArcRadiusDimension add_arc_radius_dimension;
static SetPointOnArc set_point_on_arc;
static SetPointOnCircle set_point_on_circle;
static SetPointOnArcMidpoint set_point_on_arc_midpoint;
static RunTest run_test;

void GetConstraintMenuTools(std::list<Tool*>* t_list){
	int line_count = 0;
	int arc_count = 0;
	int point_count = 0;
	int circle_count = 0;
	int ellipse_count = 0;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case PointType:
				point_count++;
				break;
			case LineType:
				line_count++;
				break;
			case ArcType:
				arc_count++;
				break;
			case CircleType:
				circle_count++;
				break;
			case EllipseType:
				ellipse_count++;
				break;
			default:
				return;
		}
	}
#ifdef MULTIPOLYTEST
	t_list->push_back(&run_test);
#endif

	int total_count = line_count + arc_count + point_count + circle_count + ellipse_count;

	if(total_count == 1 && circle_count == 1)
		t_list->push_back(&add_radius_dimension);

	if(total_count == 1 && arc_count == 1)
		t_list->push_back(&add_arc_radius_dimension);


	if(total_count < 2)
		return;

	if(line_count == 2 && total_count == 2)
	{
		t_list->push_back(&set_lines_perpendicular);
		t_list->push_back(&add_angular_dimension);
	}

	if(line_count == total_count)
	{
		t_list->push_back(&set_lines_parallel);
		t_list->push_back(&set_lines_equal_length);
		t_list->push_back(&set_lines_colinear);
	}

	if(point_count == total_count)
	{
		t_list->push_back(&set_points_coincident);
		t_list->push_back(&add_dimension);
	}

	if(line_count == 1 && point_count == 1 && total_count == 2)
	{
		t_list->push_back(&set_point_on_line);
		t_list->push_back(&set_point_on_midpoint);
	}

	if(point_count == 1 && arc_count == 1 && total_count == 2)
	{
		t_list->push_back(&set_point_on_arc);
		t_list->push_back(&set_point_on_arc_midpoint);
	}

	if(point_count == 1 && circle_count == 1 && total_count == 2)
	{
		t_list->push_back(&set_point_on_circle);
	}

	if((arc_count + circle_count == 2) && total_count == 2)
	{
		t_list->push_back(&set_concentric);
		t_list->push_back(&set_equal_radius);
	}

	if(line_count == 1 && (circle_count + arc_count  + ellipse_count == 1) && total_count == 2)
	{
		t_list->push_back(&set_line_tangent);
	}
}

void ApplyCoincidentConstraints(HeeksObj* extobj, std::list<HeeksObj*> list)
{
	list.push_back(extobj);

	std::list<HeeksObj*>::iterator it;
	std::list<HeeksObj*>::iterator it2;

	//Search for A's matching
	for(it = list.begin(); it!= list.end(); ++it)
	{
		EndedObject* eobj = (EndedObject*)*it;
		if(eobj)
		{
			for(it2 = it; it2!= list.end(); ++it2)
			{
				EndedObject* eobj2 = (EndedObject*)*it2;
				if(eobj2 && eobj != eobj2)
				{
					bool shared_points = false;
					//Check if these two objects share any points
					if(eobj->A->m_p.Distance(eobj2->A->m_p) < wxGetApp().m_geom_tol)
					{
						//A's coincidant
						eobj->A->SetCoincidentPoint(eobj2->A,false);
						shared_points = true;
					}
					if(eobj->A->m_p.Distance(eobj2->B->m_p) < wxGetApp().m_geom_tol)
					{
						//A to B coincidant
						eobj->A->SetCoincidentPoint(eobj2->B,false);
						shared_points = true;
					}
					if(eobj->B->m_p.Distance(eobj2->A->m_p) < wxGetApp().m_geom_tol)
					{
						//B to A coincidant
						eobj->B->SetCoincidentPoint(eobj2->A,false);
						shared_points = true;
					}
					if(eobj->B->m_p.Distance(eobj2->B->m_p) < wxGetApp().m_geom_tol)
					{
						//B's coincidant
						eobj->B->SetCoincidentPoint(eobj2->B,false);
						shared_points = true;
					}

					if(shared_points)
					{
						if(eobj->GetType() == LineType && eobj2->GetType() == ArcType)
						{
							eobj2->SetTangentConstraint(eobj);
						}
						if(eobj2->GetType() == LineType && eobj->GetType() == ArcType)
						{
					//		eobj->SetTangentConstraint(eobj2);
						}
					}
				}
			}
		}
	}
}


#endif