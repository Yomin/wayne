
#include "wayne.hpp"
#include <iostream>
using namespace waynespace;
using namespace std;

int main(int argc, char* argv[])
{
    Wayne w;
    Point from(2, 1);
    Point to(8, 8);
    
    w.test(); // block some points
    vector<Point> v = w.plan(from, to);
    vector<Point>::iterator it;
    
    cout<<"("<<from.y()<<","<<from.x()<<")";
    for(it = v.begin(); it != v.end(); it++)
    {
        cout<<" -> ("<<it->y()<<","<<it->x()<<")";
    }
    cout<<endl;
    
    return 0;
}

