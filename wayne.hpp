
#ifndef __WAYNE_HPP__
#define __WAYNE_HPP__

#include <list>
#include <vector>

namespace waynespace
{

class Point
{
    char vy, vx;
    
public:
    Point();
    Point(const Point &point);
    Point(int y, int x);
    
    void set_pos(Point &point);
    void set_pos(int y, int x);
    
    int y();
    int x();
    
    bool operator==(Point &p);
    bool operator!=(Point &p);
};

class MapPoint : public Point
{
public:
    enum Type  { waypoint, machine, obstacle };
    enum Angle { up, down, left, right };

private:
    // data
    int id;
    Type vtype;
    Angle angle;
    
    // partner points when grouped
    std::vector<MapPoint*> group;
    
    // A* data
    char vg, vh, vc;
    MapPoint* prev;
    
    // used in path
    bool blocked;
    
    void init(int id, Type type, Angle angle);
    void set_partner(MapPoint *point);
    
public:
    MapPoint();
    MapPoint(const MapPoint &point);
    MapPoint(MapPoint point, int id, Type type, Angle angle);
    MapPoint(int y, int x, int id, Type type, Angle angle);
    ~MapPoint();
    
    void set_info(int id, Type type, Angle angle);
    void set_blocked(bool blocked);
    void set_g(int g);
    void set_h(int h);
    void set_c(int h);
    void set_previous(MapPoint* p);
    
    int g();    // distance to point
    int h();    // distance to target
    int c();    // curviness of way to point
    Type type();
    MapPoint* previous();
    
    void connect(MapPoint &point);
    
    bool is_blocked();
};

class Wayne
{
public:
    enum { FIELDHEIGHT = 9, FIELDWIDTH = 9, STRAIGHTSCORE = 2, DIRECTIONALSCORE = 3 };
    
private:
    MapPoint point[FIELDHEIGHT][FIELDWIDTH];
    void setupMap();
    
    int h_score(Point p1, Point p2);                    // distance heuristic
    int c_score(Point p1, Point p2, Point p3);          // curviness
    MapPoint* pop_best();                               // best point from open list
    std::vector<MapPoint*> get_neighbors(MapPoint* p);  // nonblocked waypoints
    
    std::list<MapPoint*> open, close;
    
public:
    Wayne();
    ~Wayne();
    
    std::vector<Point> plan(Point from, Point to);
    void test();
};

} // namespace waynespace

#endif

