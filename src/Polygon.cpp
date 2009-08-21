// Polygon.cpp
// Copyright (c) 2009, Perttu "celero55" Ahola
// This program is released under the BSD license. See the file COPYING for details.

/*
This is quite random code right now, and the LineTreeNode binary search
tree isn't used atm. And the UnionPolygons code is just a draft of something
that will never work.
*/

#ifndef UNITTEST_NO_HEEKS
	#include "stdafx.h"
#endif

#include "Polygon.h"

#ifndef DEBUG
	#define DEBUG 1
#endif
#ifndef SLDEBUG
	#define SLDEBUG 0
#endif

const char *PDstr(PolygonDirection d)
{
	switch(d)
	{
	case PolyCW: return "PolyCW";
	case PolyCCW: return "PolyCCW";
	case PolyUndefinedW: return "PolyUndefinedW";
	}
	return "[Erroneous PolyDirection]";
}

double VecAngle(gp_Vec v1, gp_Vec v2)
{
	double v1a = atan2(v1.Y(), v1.X());
	double v2a = atan2(v2.Y(), v2.X());
	double angle = v2a - v1a;
	if(angle <= -PI) angle += PI*2;
	else if(angle > PI) angle -= PI*2;
	return angle;
}


PolygonDirection Polygon::Direction()
{
	if(m_dir != PolyUndefinedW) return m_dir;
	if(points.size() == 0){
		m_dir = PolyUndefinedW;
		return m_dir;
	}
	double angle_sum = 0.;
	std::list<gp_Pnt>::const_iterator ilp = points.begin();
	gp_Pnt p1 = *ilp;
	ilp++;
	gp_Vec v_last(p1, *ilp);
	p1 = *ilp;
	ilp++;
	for(; ilp != points.end(); ilp++)
	{
		if(p1.IsEqual(*ilp, 0.000000001)) continue;
		gp_Vec v_current(p1, *ilp);
		//angle_sum += v_last.Angle(v_current);
		angle_sum += VecAngle(v_last, v_current);
		v_last = v_current;
		p1 = *ilp;
	}
	ilp = points.begin();
	gp_Vec v_current(p1, *ilp);
	//angle_sum += v_last.Angle(v_current);
	angle_sum += VecAngle(v_last, v_current);
	v_last = v_current;
	p1 = *ilp;
	ilp++;
	v_current = gp_Vec(p1, *ilp);
	//angle_sum += v_last.Angle(v_current);
	angle_sum += VecAngle(v_last, v_current);
	/*if(DEBUG)std::cout<<"polygon starting with ("<<points.begin()->X()<<","<<points.begin()->Y()
			<<"): angle_sum = "<<angle_sum<<std::endl;*/
	//we can just look if it's positive or negative
	if(angle_sum < 0.0) m_dir = PolyCW;
	else m_dir = PolyCCW;
	return m_dir;
}

enum SweepEventType
{
	EndpointEvent,
	IntersectionEvent
};

class SweepEvent
{
public:
	SweepEvent(Endpoint p_ep) : t(EndpointEvent), ep(p_ep) {}
	SweepEvent(const LineSegment *p_s1, const LineSegment *p_s2, gp_Pnt p_ip)
			: t(IntersectionEvent), ep(NULL,Left), s1(p_s1), s2(p_s2), ip(p_ip) {}
	void GetPoint(gp_Pnt &p) const
	{
		switch(t){
		case EndpointEvent:
			p = ep.value();
			break;
		case IntersectionEvent:
			p = ip;
			break;
		}
	}
	double x() const
	{
		gp_Pnt p;
		GetPoint(p);
		return p.X();
	}
	double y() const
	{
		gp_Pnt p;
		GetPoint(p);
		return p.Y();
	}
	bool operator < (const SweepEvent &e) const
	{
		if(x() < e.x()) return true;
		if(dabs(x() - e.x()) < TOLERANCE)
			if(y() > e.y()) return true;
		return false;
	}
	bool operator == (const SweepEvent &e) const
	{
		if(t != e.t) return false;
		switch(t){
		case EndpointEvent:
			return ep == e.ep;
			break;
		case IntersectionEvent:
			return (s1 == e.s1 && s2 == e.s2
					&& ip.X() == e.ip.X() && ip.Y() == e.ip.Y());
			break;
		}
	}
	std::string str() const
	{
		std::stringstream ss;
		switch(t){
		case EndpointEvent:
			ss<<"EndpointEvent ("<<ep.value().X()<<","<<ep.value().Y()<<")";
			break;
		case IntersectionEvent:
			ss<<"IntersectionEvent ("<<ip.X()<<","<<ip.Y()<<")";
			break;
		}
		return ss.str();
	}
	SweepEventType t;
	Endpoint ep;
	const LineSegment *s1;
	const LineSegment *s2;
	gp_Pnt ip;
};

void InsertToStatus(std::list<LineSegment*> &status, LineSegment *s, gp_Pnt &p)
{
	//status has all the lines going underneath the sweep line,
	//the highest y first
	
	if(status.size()==0)
	{
		if(SLDEBUG)std::cout<<"InsertToStatus(status, s="<<s->str()<<", p=("<<p.X()<<","
				<<p.Y()<<")): status was empty"<<std::endl;
		status.push_back(s);
	}
	else
	{
		if(SLDEBUG)std::cout<<"InsertToStatus(status, s="<<s->str()<<", p=("<<p.X()<<","
				<<p.Y()<<"))"<<std::endl;
		
		std::list<LineSegment*>::iterator j = status.begin();

		while(j != status.end())
		{
			if(SLDEBUG)std::cout<<s->str()<<"->IsUpperOrEqualAfterX("<<(*j)->str()
					<<", "<<p.X()<<") == ";
			if(s->IsUpperOrEqualAfterX(*(*j), p.X()))
			{
				if(SLDEBUG)std::cout<<"true"<<std::endl;
				break;
			}
			if(SLDEBUG)std::cout<<"false"<<std::endl;
			j++;
		}
		if(j == status.end()) if(SLDEBUG)std::cout<<"j == status.end()"<<std::endl;

		status.insert(j, s);
	}
	
	if(SLDEBUG)std::cout<<"\tstatus after inserting:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}
}

void FindNewEvent(std::vector<LineSegment> &lines_vector,
		std::set<SweepEvent> &event_queue,
		LineSegment * s1, LineSegment * s2, gp_Pnt &p)
{
	IntersectionInfo info = s1->intersects(*s2);
	if(info.t == NoIntersection) return;
	//TODO: "or on it and below the current event point"
	//      means info.p.Y()<p.Y() or what?
	//if(!(info.p.X() > p.X() || (info.p.X() == p.X() && info.p.Y() < p.Y())))
	if(!(info.p.X() > p.X() || (dabs(info.p.X() - p.X()) < TOLERANCE
			&& info.p.Y() < p.Y())))
		return;
	SweepEvent event(s1, s2, info.p);
	std::set<SweepEvent>::iterator
			j = event_queue.find(event);
	if(j != event_queue.end()) return;
	event_queue.insert(event);
	if(SLDEBUG)std::cout<<"FindNewEvent(): inserted intersection event"<<std::endl
			<<"\t\tof "<<s1->str()<<" and "<<s2->str()<<std::endl
			<<"\t\tat ("<<info.p.X()<<","<<info.p.Y()<<")"<<std::endl;
}

void HandleEvent(std::vector<LineSegment> &lines_vector,
		std::set<SweepEvent> &event_queue,
		std::list<LineSegment*> &status, const SweepEvent &ep)
{
	gp_Pnt p;
	ep.GetPoint(p);
	if(SLDEBUG)std::cout<<"HandleEvent(): "<<ep.str()<<std::endl;
	std::list<LineSegment*> left, right;
	
	if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	for(std::vector<LineSegment>::iterator i = lines_vector.begin();
			i != lines_vector.end(); i++)
	{
		//if(i->left().X() == p.X() && i->left().Y() == p.Y())
		if(i->left().IsEqual(p, TOLERANCE)){
			std::list<LineSegment*>::iterator
				j = find(status.begin(), status.end(), &(*i));
			if(j != status.end()){
				if(SLDEBUG)std::cout<<"EE\t\tline.left==p but line is already in status"
						<<std::endl;
				continue;
			}
			left.push_back(&(*i));
		}
		//TODO: maybe for vertical segments, the right endpoint shouldn't exist
		//if(i->right().X() == p.X() && i->right().Y() == p.Y())
		if(i->right().IsEqual(p, TOLERANCE))
			right.push_back(&(*i));
	}
	
	//search all the segments in status that contain p and add them to contain
	std::list<LineSegment*> contain;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		std::list<LineSegment*>::iterator j;
		
		if((*i)->HasPointInterior(p))
		{
			contain.push_back(*i);
		}
		
		/*//don't take those that have it 
		j = find(left.begin(), left.end(), (*i));
		if(j != left.end()) continue;
		j = find(right.begin(), right.end(), (*i));
		if(j != right.end()) continue;

		if((*i)->HasPoint(p))
		{
			contain.push_back(*i);
		}*/
	}

	if(ep.t == IntersectionEvent && contain.size() < 2)
	{
		if(SLDEBUG)std::cout<<"EE\t\tthis is an intersection event "
				"and contain.size() < 2"<<std::endl;
	}
	
	if(SLDEBUG)std::cout<<"\tleft:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = left.begin();
			i != left.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	if(SLDEBUG)std::cout<<"\tright:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = right.begin();
			i != right.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	if(SLDEBUG)std::cout<<"\tcontain:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = contain.begin();
			i != contain.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	//if (left U right U contain) has more than one segments
	if(left.size() + right.size() + contain.size() > 1)
	{
		//here be intersections!
		if(SLDEBUG)std::cout<<"\tthere are intersections at this point."<<std::endl;
		std::list<LineSegment*> all_at_point;
		all_at_point.insert(all_at_point.end(), left.begin(), left.end());
		all_at_point.insert(all_at_point.end(), right.begin(), right.end());
		all_at_point.insert(all_at_point.end(), contain.begin(), contain.end());
		for(std::list<LineSegment*>::iterator i = all_at_point.begin();
				i != all_at_point.end(); i++)
		{
			for(std::list<LineSegment*>::iterator j = all_at_point.begin();
					j != all_at_point.end(); j++)
			{
				if(*j == *i) continue;
				IntersectionInfo info;
				info.i = (*j)->m_index;
				info.t = SomeIntersection;
				info.p = p;
				(*i)->m_intersections_set.insert(info);
			}
		}
	}

	if(SLDEBUG)std::cout<<"\tremoving right and contain from status"<<std::endl;

	//remove (right U contain) from status
	for(std::list<LineSegment*>::iterator i = right.begin();
			i != right.end(); i++)
	{
		std::list<LineSegment*>::iterator
				j = find(status.begin(), status.end(), *i);
		if(j != status.end()) status.erase(j);
		else if(SLDEBUG)std::cout<<"\t\tdid not find "<<(*i)->str()<<std::endl;
		
		j = find(status.begin(), status.end(), *i);
		if(j != status.end()) if(SLDEBUG)std::cout<<"EE\t\tthere are multiple copies of"
				" the same line in status"<<std::endl;
	}
	for(std::list<LineSegment*>::iterator i = contain.begin();
			i != contain.end(); i++)
	{
		std::list<LineSegment*>::iterator
				j = find(status.begin(), status.end(), *i);
		if(j != status.end()) status.erase(j);
		else if(SLDEBUG)std::cout<<"\t\tdid not find "<<(*i)->str()<<std::endl;

		j = find(status.begin(), status.end(), *i);
		if(j != status.end()) if(SLDEBUG)std::cout<<"EE\t\tthere are multiple copies of"
				" the same line in status"<<std::endl;
	}

	if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	std::set<LineSegment*> left_and_contain;

	for(std::list<LineSegment*>::iterator i = left.begin();
			i != left.end(); i++)
	{
		left_and_contain.insert(*i);
		//left_and_contain.push_back(*i);
	}
	for(std::list<LineSegment*>::iterator i = contain.begin();
			i != contain.end(); i++)
	{
		left_and_contain.insert(*i);
		//left_and_contain.push_back(*i);
	}

	if(SLDEBUG)std::cout<<"\tadding left and contain to status"<<std::endl;

	//add (left U contain) to status
	for(std::set<LineSegment*>::iterator i = left_and_contain.begin();
			i != left_and_contain.end(); i++)
	{
		InsertToStatus(status, *i, p);
	}
	
	//checking intersections of new vertical lines
	for(std::list<LineSegment*>::iterator i = left.begin();
			i != left.end(); i++)
	{
		if(!(*i)->is_vertical()) continue;
		if(SLDEBUG)std::cout<<"\tchecking vertical intersections of "<<(*i)->str()
				<<std::endl;
		double u = (*i)->upper().Y();
		double l = (*i)->lower().Y();
		for(std::list<LineSegment*>::iterator j = status.begin();
				j != status.end(); j++)
		{
			if(*j == *i) continue;

			IntersectionInfo info;
			info.t = SomeIntersection;

			if((*j)->is_vertical()){
				double u2 = (*j)->upper().Y();
				double l2 = (*j)->lower().Y();
				if(l2 > u) continue;
				if(u2 < l) continue;
				if(dabs(u2-u) < TOLERANCE) continue;
				if(dabs(l2-u) < TOLERANCE) continue;
				if(dabs(u2-l) < TOLERANCE) continue;
				if(dabs(l2-l) < TOLERANCE) continue;
				if(SLDEBUG)std::cout<<"\t\tfound at with vertical line "<<(*j)->str()<<std::endl;
				info.p = gp_Pnt(p.X(), u>u2?u:u2, 0);
			}
			else{
				double y = (*j)->y_value_at(p.X());
				if(y > u) continue;
				if(y < l) break;
				if(dabs(y-u) < TOLERANCE) continue;
				if(dabs(y-l) < TOLERANCE) break;
				if(SLDEBUG)std::cout<<"\t\tfound at y="<<y<<" with "<<(*j)->str()<<std::endl;
				info.p = gp_Pnt(p.X(), y, 0);
			}
			
			info.i = (*j)->m_index;
			(*i)->m_intersections_set.insert(info);
			info.i = (*i)->m_index;
			(*j)->m_intersections_set.insert(info);
		}
	}
	
	/*if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}*/

	if(SLDEBUG)std::cout<<"\tfinding events and adding them"<<std::endl;

	//if (left U contain) is empty
	if(left_and_contain.size() == 0)
	{
		if(SLDEBUG)std::cout<<"\tleft_and_contain is empty"<<std::endl;
		LineSegment *su = NULL, *sl = NULL;
		std::list<LineSegment*>::iterator i = status.begin();
		for(; i != status.end(); i++)
		{
			if(!((*i)->y_value_at(p.X()) < p.Y()))
			{
				if(i != status.begin())
				{
					i--;
					sl = (*i);
					i++;
				}
				break;
			}
		}
		for(; i != status.end(); i++)
		{
			if((*i)->y_value_at(p.X()) > p.Y())
			{
				su = (*i);
				break;
			}
		}
		if(sl == NULL){ if(SLDEBUG)std::cout<<"\t\tno lower neighbour"<<std::endl; }
		else if(su == NULL){ if(SLDEBUG)std::cout<<"\t\tno upper neighbour"<<std::endl; }
		else
		{
			FindNewEvent(lines_vector, event_queue, su, sl, p);
		}
	}
	//(left U contain) isn't empty
	else
	{
		if(SLDEBUG)std::cout<<"\tleft_and_contain is not empty"<<std::endl;
		LineSegment *su2 = NULL; //the uppermost segment of left_and_contain in status
		LineSegment *su = NULL; //the upper neighbor of su2 in status
		for(std::list<LineSegment*>::reverse_iterator i = status.rbegin();
				i != status.rend(); i++)
		{
			//std::list<LineSegment*>::iterator
				//j = find(left_and_contain.begin(), left_and_contain.end(), *i);
			std::set<LineSegment*>::iterator
				j = left_and_contain.find(*i);
			if(j != left_and_contain.end())
			{
				su2 = *i;
				if(i != status.rbegin()){
					i--;
					su = *i;
				}
				break;
			}
		}
		if(su2 == NULL) if(SLDEBUG)std::cout<<"EE\t\tsu2 == NULL"<<std::endl;
		if(su != NULL)
		{
			FindNewEvent(lines_vector, event_queue, su, su2, p);
		}
		else if(SLDEBUG)std::cout<<"\t\tsu == NULL"<<std::endl;

		LineSegment *sl2 = NULL; //the lowermost segment of left_and_contain in status
		LineSegment *sl = NULL; //the lower neighbor of sl2 in status
		for(std::list<LineSegment*>::iterator i = status.begin();
				i != status.end(); i++)
		{
			//std::list<LineSegment*>::iterator
				//j = find(left_and_contain.begin(), left_and_contain.end(), *i);
			std::set<LineSegment*>::iterator
				j = left_and_contain.find(*i);
			if(j != left_and_contain.end())
			{
				sl2 = *i;
				if(i != status.begin()){
					i--;
					sl = *i;
				}
				break;
			}
		}
		if(sl2 == NULL) if(SLDEBUG)std::cout<<"EE\t\tsl2 == NULL"<<std::endl;
		if(sl != NULL)
		{
			FindNewEvent(lines_vector, event_queue, sl, sl2, p);
		}
		else if(SLDEBUG)std::cout<<"\t\tsl == NULL"<<std::endl;
	}
	/*if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}*/

}

void SweepLine(std::vector<LineSegment> &lines_vector)
{
	std::set<SweepEvent> event_queue;
	//status has all the lines going underneath the sweep line,
	//the highest y first
	std::list<LineSegment*> status;
	if(SLDEBUG)std::cout<<"SweepLine(): lines_vector:"<<std::endl;
	for(unsigned int i=0; i<lines_vector.size(); i++)
	{
		lines_vector[i].m_index = i;
		if(SLDEBUG)std::cout<<"\t"<<lines_vector[i].str()<<std::endl;
		event_queue.insert(SweepEvent(Endpoint(&lines_vector[i], Left)));
		event_queue.insert(SweepEvent(Endpoint(&lines_vector[i], Right)));
	}
	while(!event_queue.empty())
	{
		if(SLDEBUG)std::cout<<"event_queue:"<<std::endl;
		for(std::set<SweepEvent>::iterator i = event_queue.begin();
				i != event_queue.end(); i++)
		{
			if(SLDEBUG)std::cout<<"\t"<<i->str()<<std::endl;
		}

		HandleEvent(lines_vector, event_queue, status, *event_queue.begin());
		event_queue.erase(event_queue.begin());
		/*if(SLDEBUG)std::cout<<"SweepLine(): status:"<<std::endl;
		for(std::list<LineSegment*>::iterator i = status.begin();
				i != status.end(); i++)
		{
			if(SLDEBUG)std::cout<<"\t"<<(*i)->str()<<std::endl;
		}*/
	}
	if(SLDEBUG)std::cout<<"SweepLine(): lines_vector:"<<std::endl;
	for(unsigned int i=0; i<lines_vector.size(); i++)
	{
		if(SLDEBUG)std::cout<<"\t"<<lines_vector[i].str();
		if(!lines_vector[i].m_intersections_set.empty())
			{ if(SLDEBUG)std::cout<<" intersects with:"<<std::endl; }
		else
			{ if(SLDEBUG)std::cout<<" doesn't intersect."<<std::endl; }
		for(std::set<IntersectionInfo>::const_iterator
				it = lines_vector[i].m_intersections_set.begin();
				it != lines_vector[i].m_intersections_set.end(); it++)
		{
			if(SLDEBUG)std::cout<<"\t\t"<<it->str()<<"  \t"<<lines_vector[it->i].str()<<std::endl;
		}
	}
}

class NewWayIntersection
{
public:
	LineSegment *s;
	Direction d;
	gp_Pnt p;
};

class IntersectionPoint
{
public:
	bool operator == (gp_Pnt &i)
	{
		return (i.IsEqual(p, TOLERANCE));
	}
	bool operator == (IntersectionPoint &i)
	{
		return (i.p.IsEqual(p, TOLERANCE));
	}
	//from which line segment one came to this intersection
	LineSegment *from;
	//to what direction one moved along the segment
	Direction dir;
	//the intersection point
	gp_Pnt p;
};

bool PointsCCW(std::list<IntersectionPoint> &points)
{
	if(points.size() == 0){
		return true;
	}
	double angle_sum = 0.;
	std::list<IntersectionPoint>::const_iterator ip = points.begin();
	gp_Pnt p1 = ip->p;
	ip++;
	gp_Vec v_last(p1, ip->p);
	p1 = ip->p;
	ip++;
	for(; ip != points.end(); ip++)
	{
		if(p1.IsEqual(ip->p, TOLERANCE)) continue;
		gp_Vec v_current(p1, ip->p);
		angle_sum += VecAngle(v_last, v_current);
		v_last = v_current;
		p1 = ip->p;
	}
	ip = points.begin();
	if(!p1.IsEqual(ip->p, TOLERANCE)){
		gp_Vec v_current(p1, ip->p);
		angle_sum += VecAngle(v_last, v_current);
		v_last = v_current;
		p1 = ip->p;
		ip++;
		if(!p1.IsEqual(ip->p, TOLERANCE)){
			v_current = gp_Vec(p1, ip->p);
			angle_sum += VecAngle(v_last, v_current);
		}
	}
	//we can just look if it's positive or negative
	if(angle_sum < 0.0) return false;
	return true;
}

bool FindNextPoint(std::vector<LineSegment> &lines_vector,
		std::list<Polygon> &result_list,
		std::list<IntersectionPoint> &used_points)
{
	std::cout<<"FindNextPoint()"<<std::endl;
	
	IntersectionPoint lastpoint = used_points.back();
	
	std::cout<<"lastpoint.p=("<<lastpoint.p.X()<<","<<lastpoint.p.Y()
			<<") lastpoint.from="<<lastpoint.from->str()
			<<", dir="<<(lastpoint.dir==Left?"Left":"Right")
			<<" -> ("
			<<lastpoint.from->endpoint(lastpoint.dir).X()<<","
			<<lastpoint.from->endpoint(lastpoint.dir).Y()<<")"<<std::endl;
	
	if(lastpoint.from->endpoint(lastpoint.dir).IsEqual(lastpoint.p, TOLERANCE))
	{
		std::cout<<"EE we are coming from the same point as where we are going."<<std::endl;
		return false;
	}

	double lastsegment_angle;
	
	{
		gp_Pnt from = lastpoint.p;
		gp_Pnt to = lastpoint.from->endpoint(lastpoint.dir);
		lastsegment_angle = atan2(to.Y() - from.Y(), to.X() - from.X());
	}
	
	std::cout<<"lastsegment_angle="<<lastsegment_angle<<std::endl;

	//TODO: at some point we should probably check that we aren't making a
	//      polygon we already made

	//This list will contain the segments intersecting lastpoint.from at
	//lastpoint, the one turning the most to left first.
	//Segments, that contain the point in their interior (not endpoint), will
	//be contained in this list two times, one instance going the other way and
	//the other instance the other
	//TODO: Change NewWayIntersection to IntersectionPoint if the from field
	//      in IntersectionPoint won't be used
	std::list<NewWayIntersection> intersections_sorted_list;

	/*std::set<IntersectionInfo>
			intersections_at_point(lastpoint.from->m_intersections_set);*/
			
	std::list<IntersectionInfo> intersections_at_point;

	//add intersections at this point to list
	for(std::set<IntersectionInfo>::iterator
			i = lastpoint.from->m_intersections_set.begin();
			i != lastpoint.from->m_intersections_set.end(); i++)
	{
		if(!i->p.IsEqual(lastpoint.p, TOLERANCE)) continue;
		intersections_at_point.push_back(*i);
	}

	while(!intersections_at_point.empty())
	{
		double max_diff_angle = -PI;
		std::list<IntersectionInfo>::iterator max_i;
		Direction max_d;
		for(std::list<IntersectionInfo>::iterator
				i = intersections_at_point.begin();
				i != intersections_at_point.end(); i++)
		{
			gp_Pnt from = i->p;
			LineSegment *nextsegment = &lines_vector[i->i];
			gp_Pnt to_l = nextsegment->endpoint(Left);
			if(!to_l.IsEqual(from, TOLERANCE)){
				double nextsegment_angle_l = atan2(to_l.Y() - from.Y(), to_l.X() - from.X());
				double diff_angle = -(lastsegment_angle - nextsegment_angle_l);
				/*std::cout<<"nextsegment_angle_l="<<nextsegment_angle_l
						<<" diff_angle="<<diff_angle<<std::endl;*/
				if(diff_angle >= max_diff_angle){
					//std::cout<<"is more than diff_angle="<<diff_angle<<std::endl;
					max_diff_angle = diff_angle;
					max_d = Left;
					max_i = i;
				}
			}
			gp_Pnt to_r = nextsegment->endpoint(Right);
			if(!to_r.IsEqual(from, TOLERANCE)){
				double nextsegment_angle_r = atan2(to_r.Y() - from.Y(), to_r.X() - from.X());
				double diff_angle = -(lastsegment_angle - nextsegment_angle_r);
				/*std::cout<<"nextsegment_angle_r="<<nextsegment_angle_r
						<<" diff_angle="<<diff_angle<<std::endl;*/
				if(diff_angle >= max_diff_angle){
					//std::cout<<"is more than diff_angle="<<diff_angle<<std::endl;
					max_diff_angle = diff_angle;
					max_d = Right;
					max_i = i;
				}
			}
		}
		NewWayIntersection nwi;
		nwi.s = &lines_vector[max_i->i];
		nwi.d = max_d;
		nwi.p = max_i->p;
		intersections_sorted_list.push_back(nwi);
		intersections_at_point.erase(max_i);
	}

	std::cout<<"sorted intersections at lastpoint.p=("<<lastpoint.p.X()
			<<","<<lastpoint.p.Y()<<")"<<std::endl;
	for(std::list<NewWayIntersection>::iterator
			i = intersections_sorted_list.begin();
			i != intersections_sorted_list.end(); i++)
	{
		/*std::cout<<"\ti->p=("<<i->p.X()<<","<<i->p.Y()<<") i->s="<<i->s->str()
				<<" i->d="<<(i->d==Left?"Left":"Right")<<std::endl;*/
	}

	for(std::list<NewWayIntersection>::iterator
			i = intersections_sorted_list.begin();
			i != intersections_sorted_list.end(); i++)
	{
		//get the point we're going to now
		IntersectionPoint newpoint;
		//newpoint.from = lastpoint.from;
		newpoint.p = i->p;
		newpoint.from = i->s;
		newpoint.dir = i->d;

		std::cout<<"Going towards: ("<<newpoint.p.X()<<","<<newpoint.p.Y()
				<<")"<<std::endl;

		//check what is the latest occurrence of newpoint in used_points
		std::list<IntersectionPoint>::iterator latest_newpoint;
		latest_newpoint = used_points.end();
		bool found = false;
		do{
			latest_newpoint--;
			if(*latest_newpoint == newpoint){
				found = true;
				break;
			}
		}
		while(latest_newpoint != used_points.begin());

		//if newpoint was found in used_points
		if(found)
		{
			std::cout<<"newpoint found in used_points"<<std::endl;
			//if the points make a counter-clockwise polygon, we have failed
			//to find a valid one
			if(PointsCCW(used_points)){
				return false;
			}
			//used_points should be a valid polygon now
			Polygon p;
			//add the points starting from the latest occurrence of newpoint
			for(std::list<IntersectionPoint>::iterator i = latest_newpoint;
					i != used_points.end(); i++)
			{
				p.push_back(i->p);
			}
			result_list.push_back(p);
		}
		//if newpoint wasn't found in used_points
		else{
			//add it to there and start finding the next point
			used_points.push_back(newpoint);
			if(FindNextPoint(lines_vector, result_list, used_points))
			{
				//a polygon was made, we're not needed anymore
				return true;
			}
			//now a new point, if an another alternative still exists, will be
			//tried
		}
	}
	//couldn't find a valid polygon
	return false;
}

bool UnionPolygons(std::vector<LineSegment> &lines_vector,
		std::list<Polygon> &result_list)
{
	std::cout<<"UnionPolygons(): "<<lines_vector.size()<<" lines"<<std::endl;

	SweepLine(lines_vector);
	
	std::cout<<"UnionPolygons(): lines_vector:"<<std::endl;
	for(unsigned int i=0; i<lines_vector.size(); i++)
	{
		std::cout<<"\t"<<lines_vector[i].str();
		if(!lines_vector[i].m_intersections_set.empty())
			{ std::cout<<" intersects with:"<<std::endl; }
		else
			{ std::cout<<" doesn't intersect."<<std::endl; }
		for(std::set<IntersectionInfo>::const_iterator
				it = lines_vector[i].m_intersections_set.begin();
				it != lines_vector[i].m_intersections_set.end(); it++)
		{
			std::cout<<"\t\t"<<it->str()<<"  \t"<<lines_vector[it->i].str()<<std::endl;
		}
	}
#if 0
	std::cout<<"handling lines_vector:"<<std::endl;
	for(std::vector<LineSegment>::iterator i = lines_vector.begin();
			i != lines_vector.end(); i++)
	{
		/*std::cout<<"-----------------"<<std::endl;
		std::cout<<i->str()<<std::endl;
		std::list<IntersectionPoint> used_points;
		IntersectionPoint ip;
		ip.p = i->a;
		ip.from = &(*i);
		used_points.push_back(ip);
		bool ret = FindNextPoint(lines_vector, result_list, used_points);
		std::cout<<"FindNextPoint() returned "<<ret<<std::endl;*/

		if(i->m_intersections_set.empty())
		{
			std::cout<<"a separate line found: "<<i->str()<<std::endl;
		}
		//we'll be turning always as much left as possible
		std::set<IntersectionInfo>::const_iterator j;
		for(j = i->m_intersections_set.begin();
				j != i->m_intersections_set.end(); j++)
		{
			std::cout<<"-----------------"<<std::endl;
			std::cout<<i->str()<<std::endl;
			std::list<IntersectionPoint> used_points;
			IntersectionPoint ip;
			ip.p = j->p;
			ip.from = &(*i);
			ip.dir = Right;
			used_points.push_back(ip);
			bool ret = FindNextPoint(lines_vector, result_list, used_points);
			std::cout<<"FindNextPoint() returned "<<ret<<std::endl;
		}
	}
#endif
	return true;
}

bool UnionPolygons(std::list<Polygon> &polygons_list, std::list<Polygon> &result_list)
{
	std::vector<LineSegment> lines_vector;
	for(std::list<Polygon>::const_iterator i = polygons_list.begin();
			i != polygons_list.end(); i++)
	{
		for(std::list<gp_Pnt>::const_iterator j = i->begin();
				j != i->end(); j++)
		{
			lines_vector.push_back(i->GetLine(j));
		}
	}

	return UnionPolygons(lines_vector, result_list);
}

