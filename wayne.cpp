
#include "wayne.hpp"
#include <algorithm>

using namespace std;

#define ABS(x) ((x)>0 ? (x) : -(x))

namespace waynespace
{

Point::Point()
{
    vy = 0;
    vx = 0;
}

Point::Point(const Point &point)
{
    vy = point.vy;
    vx = point.vx;
}

Point::Point(int y, int x)
{
    vy = y;
    vx = x;
}

void Point::set_pos(Point &point)
{
    vy = point.vy;
    vx = point.vx;
}

void Point::set_pos(int y, int x)
{
    vy = y;
    vx = x;
}

int Point::y()
{
    return vy;
}

int Point::x()
{
    return vx;
}

bool Point::operator==(Point &p)
{
    return vy == p.vy && vx == p.vx;
}

bool Point::operator!=(Point &p)
{
    return ! (*this == p);
}



MapPoint::MapPoint() : Point(0, 0)
{
    init(-1, waypoint, left);
}

MapPoint::MapPoint(const MapPoint &point) : Point(point)
{
    init(point.id, point.vtype, point.angle);
}

MapPoint::MapPoint(MapPoint point, int id, Type type, Angle angle) : Point(point)
{
    init(id, type, angle);
}

MapPoint::MapPoint(int y, int x, int id, Type type, Angle angle) : Point(y, x)
{
    init(id, type, angle);
}

MapPoint::~MapPoint()
{
    
}

void MapPoint::init(int id, Type type, Angle angle)
{
    set_info(id, type, angle);
    blocked = false;
}

void MapPoint::connect(MapPoint &point)
{
    int my_count  = group.size();
    int his_count = point.group.size();
    
    group.push_back(&point);
    point.group.push_back(this);
    
    // set me as partner of all partners of p
    for(int i=0; i<his_count; i++)
    {
        group.push_back(point.group[i]);
        point.group[i]->group.push_back(this);
    }
    // set p as partner of all partners of me
    for(int i=0; i<my_count; i++)
    {
        group[i]->group.push_back(&point);
        point.group.push_back(group[i]);
    }
    // set all partners of p as partners of all partners of me
    for(int i=0; i<my_count; i++)
        for(int j=0; j<his_count; j++)
        {
            group[i]->group.push_back(point.group[j]);
            point.group[j]->group.push_back(group[i]);
        }
}

void MapPoint::set_info(int id, Type type, Angle angle)
{
    this->id = id;
    this->vtype = type;
    this->angle = angle;
}

void MapPoint::set_blocked(bool blocked)
{
    this->blocked = blocked;
    for(unsigned int i=0; i<group.size(); i++)
        group[i]->blocked = blocked;
}

void MapPoint::set_g(int g)
{
    vg = g;
}

void MapPoint::set_h(int h)
{
    vh = h;
}

void MapPoint::set_c(int c)
{
    vc = c;
}

void MapPoint::set_previous(MapPoint* p)
{
    prev = p;
}

int MapPoint::g()
{
    return vg;
}

int MapPoint::h()
{
    return vh;
}

int MapPoint::c()
{
    return vc;
}

MapPoint::Type MapPoint::type()
{
    return vtype;
}

MapPoint* MapPoint::previous()
{
    return prev;
}

bool MapPoint::is_blocked()
{
    return blocked;
}



Wayne::Wayne()
{
    setupMap();
}

Wayne::~Wayne()
{
    
}

void Wayne::setupMap()
{
    for(int y=0; y<FIELDHEIGHT; y++)
        for(int x=0; x<FIELDWIDTH; x++)
            point[y][x].set_pos(y, x);
    
    point[0][2].set_info(0, MapPoint::machine, MapPoint::down);
    point[2][2].set_info(1, MapPoint::machine, MapPoint::left);
    point[6][2].set_info(2, MapPoint::machine, MapPoint::right);
    point[0][4].set_info(3, MapPoint::machine, MapPoint::down);
    point[2][4].set_info(4, MapPoint::machine, MapPoint::left);
    point[4][4].set_info(5, MapPoint::machine, MapPoint::down);
    point[6][4].set_info(6, MapPoint::machine, MapPoint::right);
    point[8][4].set_info(7, MapPoint::machine, MapPoint::right);
    point[2][2].set_info(8, MapPoint::machine, MapPoint::right);
    point[6][6].set_info(9, MapPoint::machine, MapPoint::right);
    
    // connect incoming goods area points
    point[3][0].connect(point[4][0]);
    point[3][0].connect(point[5][0]);
    // connect delivery gates points
    point[3][8].connect(point[4][8]);
    point[3][8].connect(point[5][8]);
}

int Wayne::h_score(Point p1, Point p2)
{
    int y = ABS(p1.y() - p2.y());
    int x = ABS(p1.x() - p2.x());
    int d = ABS(y-x);
    return (max(y, x)-d)*DIRECTIONALSCORE + d*STRAIGHTSCORE;
}

int Wayne::c_score(Point p1, Point p2, Point p3)
{
    int h = h_score(p1, p3);
    
    if(p1 == p2 || p2 == p3)
        return 0;
    if(p1.y() == p2.y() && p2.y() == p3.y())
        return 0;
    if(p1.x() == p2.x() && p2.x() == p3.x())
        return 0;
    if(p1.y() - p2.y() == p2.y() - p3.y() && p1.x() - p2.x() == p2.x() - p3.x())
        return 0;
    if(p1.y() == p3.y() && h == STRAIGHTSCORE*2)
        return 2;
    if(p1.x() == p3.x() && h == STRAIGHTSCORE*2)
        return 2;
    if(p1.y() == p3.y() && h == STRAIGHTSCORE)
        return 3;
    if(p1.x() == p3.x() && h == STRAIGHTSCORE)
        return 3;
    if(p1 == p3)
        return 4;
    return 1;
}

MapPoint* Wayne::pop_best()
{
    list<MapPoint*>::iterator best = open.begin(), it = ++open.begin();
    MapPoint *p, *b;
    for(; it != open.end(); it++)
    {
        p = *it;
        b = *best;
        if(p->g()+p->h() < b->g()+b->h())
            best = it;
        if(p->g()+p->h() == b->g()+b->h() && p->c() < b->c())
            best = it;
    }
    open.erase(best);
    return *best;
}

vector<MapPoint*> Wayne::get_neighbors(MapPoint* p)
{
    vector<MapPoint*> v;
    MapPoint *n;
    if(p->y() > 0)
    {
        n = &point[p->y()-1][p->x()];
        if(n->type() == MapPoint::waypoint && !n->is_blocked())
            v.push_back(n);
        n = &point[p->y()-1][p->x()-1];
        if(p->x() > 0 && n->type() == MapPoint::waypoint && !n->is_blocked())
            v.push_back(n);
        n = &point[p->y()-1][p->x()+1];
        if(p->x() < FIELDWIDTH-1 && n->type() == MapPoint::waypoint && !n->is_blocked())
            v.push_back(n);
    }
    if(p->y() < FIELDHEIGHT-1)
    {
        n = &point[p->y()+1][p->x()];
        if(n->type() == MapPoint::waypoint && !n->is_blocked())
            v.push_back(n);
        n = &point[p->y()+1][p->x()-1];
        if(p->x() > 0 && n->type() == MapPoint::waypoint && !n->is_blocked())
            v.push_back(n);
        n = &point[p->y()+1][p->x()+1];
        if(p->x() < FIELDWIDTH-1 && n->type() == MapPoint::waypoint && !n->is_blocked())
            v.push_back(n);
    }
    n = &point[p->y()][p->x()-1];
    if(p->x() > 0 && n->type() == MapPoint::waypoint && !n->is_blocked())
        v.push_back(n);
    n = &point[p->y()][p->x()+1];
    if(p->x() < FIELDWIDTH-1 && n->type() == MapPoint::waypoint && !n->is_blocked())
        v.push_back(n);
    return v;
}

vector<Point> Wayne::plan(Point from, Point to)
{
    MapPoint* p = &point[from.y()][from.x()];
    p->set_g(0);
    p->set_h(h_score(from, to));
    p->set_c(0);
    p->set_previous(p);
    open.push_back(p);
    
    MapPoint *current;
    vector<MapPoint*> neighbors;
    vector<MapPoint*>::iterator it1;
    list<MapPoint*>::iterator it2;
    int g, h, c;
    
    while(open.size() > 0)
    {
        current = pop_best();
        if(*current == to)
            break;
        close.push_back(current);
        neighbors = get_neighbors(current);
        for(it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
        {
            it2 = find(close.begin(), close.end(), *it1);
            if(it2 != close.end())
                continue;
            
            it2 = find(open.begin(), open.end(), *it1);
            if(current->y() == (*it1)->y() || current->x() == (*it1)->x())
                g = current->g() + STRAIGHTSCORE;
            else
                g = current->g() + DIRECTIONALSCORE;
            c = current->c() + c_score(*current->previous(), *current, **it1);
            
            if(it2 == open.end())
            {
                h = h_score(**it1, to);
                open.push_back(*it1);
                (*it1)->set_previous(current);
                (*it1)->set_g(g);
                (*it1)->set_h(h);
                (*it1)->set_c(c);
            }
            else if(g < (*it2)->g() || (g == (*it2)->g() && c < (*it2)->c()))
            {
                (*it1)->set_previous(current);
                (*it1)->set_g(g);
                (*it1)->set_c(c);
            }
        }
    }
    vector<Point> v;
    
    if(*current != to)
        return v;
    
    while(*current != from)
    {
        v.push_back(Point(current->y(), current->x()));
        current = current->previous();
    }
    reverse(v.begin(), v.end());
    
    open.clear();
    close.clear();
    
    return v;
}

void Wayne::test()
{
    point[5][1].set_blocked(true);
    point[5][2].set_blocked(true);
    point[5][3].set_blocked(true);
    point[4][3].set_blocked(true);
    point[3][3].set_blocked(true);
    point[3][4].set_blocked(true);
}

} // namespace waynespace

