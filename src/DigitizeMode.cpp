// DigitizeMode.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "DigitizeMode.h"
#include "../interface/MarkedObject.h"
#include "../interface/PropertyList.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyString.h"
#include "../interface/Tool.h"
#include "SelectMode.h"
#include "MarkedList.h"
#include "PointOrWindow.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"
#include "OptionsCanvas.h"
#include "CoordinateSystem.h"
#include "PropertiesCanvas.h"

#include "DigitizeMode.h"
#include "Drawing.h"

DigitizeMode::DigitizeMode(){
	point_or_window = new PointOrWindow(false);
	m_doing_a_main_loop = false;
	m_callback = NULL;
}

DigitizeMode::~DigitizeMode(void){
	delete point_or_window;
}

static wxString digitize_title_coords_string;

const wxChar* DigitizeMode::GetTitle()
{
	if(m_doing_a_main_loop)
	{
		digitize_title_coords_string = m_prompt_when_doing_a_main_loop;
		wxString xy_str;
		digitize_title_coords_string.append(_T("\n"));
		if(fabs(digitized_point.m_point.Z()) < 0.00000001)digitize_title_coords_string += wxString::Format(_T("X%g Y%g"), digitized_point.m_point.X(), digitized_point.m_point.Y());
		else if(fabs(digitized_point.m_point.Y()) < 0.00000001)digitize_title_coords_string += wxString::Format(_T("X%g Z%g"), digitized_point.m_point.X(), digitized_point.m_point.Z());
		else if(fabs(digitized_point.m_point.X()) < 0.00000001)digitize_title_coords_string += wxString::Format(_T("Y%g Z%g"), digitized_point.m_point.Y(), digitized_point.m_point.Z());
		else digitize_title_coords_string += wxString::Format(_T("X%g Y%g Z%g"), digitized_point.m_point.X(), digitized_point.m_point.Y(), digitized_point.m_point.Z());

		const wxChar* type_str = NULL;
		switch(digitized_point.m_type)
		{
		case DigitizeEndofType:
			type_str = _("end");
			break;
		case DigitizeIntersType:
			type_str = _("intersection");
			break;
		case DigitizeMidpointType:
			type_str = _("midpoint");
			break;
		case DigitizeCentreType:
			type_str = _("centre");
			break;
		case DigitizeScreenType:
			type_str = _("screen");
			break;
		case DigitizeNearestType:
			type_str = _("nearest");
			break;
		case DigitizeTangentType:
			type_str = _("tangent");
			break;
		}

		if(type_str)
		{
			digitize_title_coords_string.append(_T(" ("));
			digitize_title_coords_string.append(type_str);
			digitize_title_coords_string.append(_T(")"));
		}

		return digitize_title_coords_string.c_str();
	}
	return _("Digitize Mode");
}

static wxString digitize_help_text;

const wxChar* DigitizeMode::GetHelpText()
{
	if(!m_doing_a_main_loop)return NULL;
	digitize_help_text.assign(_("Press Esc key to cancel"));
	digitize_help_text.append(_T("\n"));
	digitize_help_text.append(_("Left button to accept position"));
	return digitize_help_text.c_str();
}

void DigitizeMode::OnMouse( wxMouseEvent& event ){
	if(event.MiddleIsDown() || event.GetWheelRotation() != 0)
	{
		wxGetApp().m_select_mode->OnMouse(event);
		return;
	}

	if(event.LeftDown()){
		point_or_window->OnMouse(event);
		lbutton_point = digitize(wxPoint(event.GetX(), event.GetY()));
	}
	else if(event.LeftUp()){
		if(lbutton_point.m_type != DigitizeNoItemType){
			digitized_point = lbutton_point;
			if(m_doing_a_main_loop){
				ExitMainLoop();
			}
		}
	}
	else if(event.Moving()){
		digitize(wxPoint(event.GetX(), event.GetY()));
		point_or_window->OnMouse(event);
		if(m_doing_a_main_loop)
		{
			wxGetApp().m_frame->RefreshInputCanvas();
			wxGetApp().OnInputModeTitleChanged();
		}
		if(m_callback)
		{
			double pos[3];
			extract(digitized_point.m_point, pos);
			(*m_callback)(pos);
		}
	}
}

void DigitizeMode::OnKeyDown(wxKeyEvent& event)
{
	switch(event.KeyCode())
	{
	case WXK_ESCAPE:
		digitized_point.m_type = DigitizeNoItemType;
		if(m_doing_a_main_loop)ExitMainLoop();
		break;
	}
}

static gp_Trsf global_matrix_relative_to_screen;

static const gp_Trsf& digitizing_matrix(bool calculate = false){
	if(calculate){
		if(wxGetApp().digitize_screen){
			gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
			gp_Pnt origin = gp_Pnt(0, 0, 0).Transformed(mat);
			gp_Pnt x1 = origin.XYZ() + gp_XYZ(1, 0, 0);
			gp_Pnt y1 = origin.XYZ() + gp_XYZ(0, 1, 0);
			gp_Pnt po = origin;
			po = wxGetApp().m_current_viewport->m_view_point.glUnproject(po);
			x1 = wxGetApp().m_current_viewport->m_view_point.glUnproject(x1);
			y1 = wxGetApp().m_current_viewport->m_view_point.glUnproject(y1);
			
			global_matrix_relative_to_screen = make_matrix(origin, gp_Vec(po, x1).Normalized(), gp_Vec(po, y1).Normalized());
		}
		else{
			global_matrix_relative_to_screen = wxGetApp().GetDrawMatrix(!wxGetApp().m_sketch_mode);
		}
	}
	return global_matrix_relative_to_screen;
}

bool DigitizeMode::OnModeChange(void){
	point_or_window->reset();
	if(!point_or_window->OnModeChange())return false;
	digitize(wxGetApp().cur_mouse_pos);
	return true;
}

DigitizedPoint DigitizeMode::digitize1(const wxPoint &input_point){
	gp_Lin ray = wxGetApp().m_current_viewport->m_view_point.SightLine(input_point);
	std::list<DigitizedPoint> compare_list;
	MarkedObjectManyOfSame marked_object;
	if(wxGetApp().digitize_end || wxGetApp().digitize_inters || wxGetApp().digitize_centre || wxGetApp().digitize_midpoint || wxGetApp().digitize_nearest || wxGetApp().digitize_tangent){
		point_or_window->SetWithPoint(input_point);
		wxGetApp().m_marked_list->ignore_coords_only = true;
		wxGetApp().m_marked_list->ObjectsInWindow(point_or_window->box_chosen, &marked_object);
		wxGetApp().m_marked_list->ignore_coords_only = false;
	}
	if(wxGetApp().digitize_end){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				std::list<GripData> vl;
				object->GetGripperPositionsTransformed(&vl, true);
				std::list<gp_Pnt> plist;
				convert_gripdata_to_pnts(vl, plist);
				for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
				{
					gp_Pnt& pnt = *It;
					compare_list.push_back(DigitizedPoint(pnt, DigitizeEndofType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(wxGetApp().digitize_inters){
		if(marked_object.m_map.size()>0){
			std::list<HeeksObj*> object_list;
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				object_list.push_back(object);
				object = marked_object.Increment();
			}

			if(object_list.size() > 1)
			{
				for(std::list<HeeksObj*>::iterator It = object_list.begin(); It != object_list.end(); It++)
				{
					HeeksObj* object = *It;
					std::list<HeeksObj*>::iterator It2 = It;
					It2++;
					for(; It2 != object_list.end(); It2++)
					{
						HeeksObj* object2 = *It2;
						std::list<double> rl;
						if(object->Intersects(object2, &rl))
						{
							std::list<gp_Pnt> plist;
							convert_doubles_to_pnts(rl, plist);
							for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
							{
								gp_Pnt& pnt = *It;
								compare_list.push_back(DigitizedPoint(pnt, DigitizeIntersType));
							}
						}
					}
				}
			}
		}
	}
	if(wxGetApp().digitize_midpoint){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				double p[3];
				if(object->GetMidPoint(p)){
					compare_list.push_back(DigitizedPoint(make_point(p), DigitizeMidpointType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(wxGetApp().digitize_nearest){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfEverything();
			while(object){
				double ray_start[3], ray_direction[3];
				extract(ray.Location(), ray_start);
				extract(ray.Direction(), ray_direction);
				double p[3];
				if(object->FindNearPoint(ray_start, ray_direction, p)){
					compare_list.push_back(DigitizedPoint(make_point(p), DigitizeNearestType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(wxGetApp().digitize_tangent){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfEverything();
			while(object){
				double ray_start[3], ray_direction[3];
				extract(ray.Location(), ray_start);
				extract(ray.Direction(), ray_direction);
				double p[3];
				if(object->FindPossTangentPoint(ray_start, ray_direction, p)){
					compare_list.push_back(DigitizedPoint(make_point(p), DigitizeTangentType, object));
				}
				object = marked_object.Increment();
			}
		}
	}
	double min_dist = -1;
	double best_dp = 0;
	DigitizedPoint *best_digitized_point = NULL;
	if(compare_list.size() >0){
		std::list<DigitizedPoint>::iterator It;
		double dist;
		double dp;
		for(It = compare_list.begin(); It != compare_list.end(); It++){
			DigitizedPoint *this_digitized_point = &(*It);
			dist = ray.Distance(this_digitized_point->m_point);
			dp = gp_Vec(ray.Direction()) * gp_Vec(this_digitized_point->m_point.XYZ()) - gp_Vec(ray.Direction()) * gp_Vec(ray.Location().XYZ());
			if(dist * wxGetApp().GetPixelScale() < 2)dist = 2/wxGetApp().GetPixelScale();
			if(dist * wxGetApp().GetPixelScale()>10)continue;
			bool use_this = false;
			if(best_digitized_point == NULL)use_this = true;
			else if(this_digitized_point->importance() > best_digitized_point->importance())use_this = true;
			else if(this_digitized_point->importance() == best_digitized_point->importance() && dist<min_dist)use_this = true;
			if(use_this){
				min_dist = dist;
				best_digitized_point = this_digitized_point;
				best_dp = dp;
			}
		}
	}
	if(wxGetApp().digitize_centre && (min_dist == -1 || min_dist * wxGetApp().GetPixelScale()>5)){
		gp_Pnt pos;
		for(HeeksObj* object = marked_object.GetFirstOfEverything(); object != NULL; object = marked_object.Increment()){
			double p[3], p2[3];
			int num = object->GetCentrePoints(p, p2);
			if(num == 1)
			{
				compare_list.push_back(DigitizedPoint(make_point(p), DigitizeCentreType));
			}
			else if(num == 2)
			{
				double dist1 = ray.Distance(make_point(p));
				double dist2 = ray.Distance(make_point(p2));
				compare_list.push_back(DigitizedPoint(make_point((dist1 < dist2) ? p:p2), DigitizeCentreType));
			}
			else
				continue;

			best_digitized_point = &(compare_list.back());
			break;
		}
	}
	DigitizedPoint point;
	if(best_digitized_point){
		point = *best_digitized_point;
	}
	else if(wxGetApp().digitize_coords){
		point = Digitize(ray);
	}
	
	return point;
}

DigitizedPoint DigitizeMode::Digitize(const gp_Lin &ray){
	gp_Pln pl(gp_Pnt(0, 0, 0), gp_Vec(0, 0, 1));
	pl.Transform(digitizing_matrix(true));
	gp_Pnt pnt;
	if(!intersect(ray, pl, pnt)){
		pl = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, -1, 0));
		if(!intersect(ray, pl, pnt))DigitizedPoint();

		pl = gp_Pln(gp_Pnt(0, 0, 0), gp_Vec(1, 0, 0));
		if(!intersect(ray, pl, pnt))DigitizedPoint();
	}

	DigitizedPoint point(pnt, DigitizeCoordsType);

	if(wxGetApp().draw_to_grid){
		gp_Vec plane_vx = gp_Vec(1, 0, 0).Transformed(digitizing_matrix());
		gp_Vec plane_vy = gp_Vec(0, 1, 0).Transformed(digitizing_matrix());
		gp_Pnt datum = gp_Pnt(0, 0, 0).Transformed(digitizing_matrix());

		double a = gp_Vec(datum.XYZ()) * plane_vx;
		double b = gp_Vec(point.m_point.XYZ()) * plane_vx;
		double c = b - a;
		double extra1 = c > -0.00000001 ? 0.5:-0.5;
		c = (int)(c / wxGetApp().digitizing_grid + extra1) * wxGetApp().digitizing_grid;

		double datum_dotp_y = gp_Vec(datum.XYZ()) * plane_vy;
		double rp_dotp_y = gp_Vec(point.m_point.XYZ()) * plane_vy;
		double d = rp_dotp_y - datum_dotp_y;
		double extra2 = d > -0.00000001 ? 0.5:-0.5;
		d = (int)(d / wxGetApp().digitizing_grid + extra2) * wxGetApp().digitizing_grid;

		if(wxGetApp().m_sketch_mode)
			point.m_point = gp_XYZ(1,0,0) * c + gp_XYZ(0,1,0) * d;
		else
			point.m_point = datum.XYZ() + plane_vx.XYZ() * c + plane_vy.XYZ() * d;
	}

	return point;
}

DigitizedPoint DigitizeMode::digitize(const wxPoint &point){
	digitized_point = digitize1(point);
	return digitized_point;
}

void DigitizeMode::OnFrontRender(){
	point_or_window->OnFrontRender();
}

static void set_x(double value, HeeksObj* object){wxGetApp().m_digitizing->digitized_point.m_point.SetX(value); wxGetApp().m_frame->RefreshInputCanvas();}
static void set_y(double value, HeeksObj* object){wxGetApp().m_digitizing->digitized_point.m_point.SetY(value); wxGetApp().m_frame->RefreshInputCanvas();}
static void set_z(double value, HeeksObj* object){wxGetApp().m_digitizing->digitized_point.m_point.SetZ(value); wxGetApp().m_frame->RefreshInputCanvas();}
static void set_offset(const wxChar *value, HeeksObj* object)
{
	wxStringTokenizer tokens(value,_T(" :,\t\n"));

	// The reference_point is the last coordinate that was used in the Drawing::AddPoint() method.  i.e. the
	// last point clicked by the operator during a drawing operation.

	gp_Pnt location(wxGetApp().m_digitizing->reference_point.m_point);
	for (int i=0; i<3; i++)
	{
		if (tokens.HasMoreTokens())
		{
			double offset = 0.0;
			wxString token = tokens.GetNextToken();
			if (token.ToDouble(&offset))
			{
				offset *= wxGetApp().m_view_units;
				switch(i)
				{
				case 0: 
					wxGetApp().m_digitizing->digitized_point.m_point.SetX( location.X() + offset );
					break;

				case 1:
					wxGetApp().m_digitizing->digitized_point.m_point.SetY( location.Y() + offset );
					break;

				case 2:
					wxGetApp().m_digitizing->digitized_point.m_point.SetZ( location.Z() + offset );
					break;
				}
				
			}
		}
	}

	wxGetApp().m_frame->RefreshInputCanvas();
}

void DigitizeMode::GetProperties(std::list<Property *> *list){
	list->push_back(new PropertyLength(_("X"), digitized_point.m_point.X(), NULL, set_x));
	list->push_back(new PropertyLength(_("Y"), digitized_point.m_point.Y(), NULL, set_y));
	list->push_back(new PropertyLength(_("Z"), digitized_point.m_point.Z(), NULL, set_z));

	Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
	if (pDrawingMode != NULL)
	{
		// The wxGetApp().m_digitizing->reference_point is only valid while in one of the
		// Drawing input modes.  If we want to expand its role then we need to make sure
		// its value is set in other circumstances as well.  At the moment, it's only set
		// within the Drawing::AddPoint() method.

		list->push_back(new PropertyString(_("Offset (from last point)"), _T("0,0,0"), NULL, set_offset));
	}
}

class EndPosPicking:public Tool{
public:
	void Run(){
		if(wxGetApp().m_digitizing->m_doing_a_main_loop)
		{
			ExitMainLoop();
		}
		else{
			wxMessageBox(_("Error! The 'Stop Picking' button shouldn't have been available!"));
		}
	}
	const wxChar* GetTitle(){return _("Stop Picking");}
	wxString BitmapPath(){return _T("endpospick");}
	const wxChar* GetToolTip(){return _("Finish picking");}
};

static EndPosPicking end_pos_picking;

void DigitizeMode::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	if(m_doing_a_main_loop)t_list->push_back(&end_pos_picking);
}

void DigitizeMode::SetOnlyCoords(HeeksObj* object, bool onoff){
	if(onoff)m_only_coords_set.insert(object);
	else m_only_coords_set.erase(object);
}

bool DigitizeMode::OnlyCoords(HeeksObj* object){
	if(m_only_coords_set.find(object) != m_only_coords_set.end())return true;
	return false;
}
