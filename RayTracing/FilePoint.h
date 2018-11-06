#pragma once
#include <qlist.h>
#include "point.h"
#include <list>
#include <vector>
using namespace std;
class FilePoint
{
public:
	vector<int> index;
	double Xmin = 0.0;
	double Ymin = 0.0;
	double Xmax = 0.0;
	double Ymax = 0.0;
	double Zmin = 0.0;
	double Zmax = 0.0;
    double *hightArr = new double[20000];
	vector<Point*> allPointList;
public:
	FilePoint();
	~FilePoint();
	void expendIndex(int num) {
		index.push_back(num);
	}
	void expendPointList(Point *point) {
		allPointList.push_back(point);
	}
	void uniformlize() {
		for (int i = 0; i < allPointList.size(); i++) {
			allPointList[i]->x = (allPointList[i]->x - Xmin) / (Xmax - Xmin);
			allPointList[i]->y = (allPointList[i]->y - Ymin) / (Xmax - Xmin);
			allPointList[i]->z = allPointList[i]->z / 600.0;
		}
	}
	void uniformlize(double shift, double shiftZ) {
		for (int i = 0; i < allPointList.size(); i++) {
			allPointList[i]->x = (allPointList[i]->x - (Xmax + Xmin) / 2) / (Xmax - Xmin)*shift;
			allPointList[i]->y = (allPointList[i]->y - (Ymax + Ymin) / 2) / (Xmax - Xmin)*shift;
            allPointList[i]->z = allPointList[i]->z / 800*shiftZ;
		}
	}
};
