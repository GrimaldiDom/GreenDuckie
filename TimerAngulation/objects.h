#ifndef OBJECTS_H
#define OBJECTS_H

#include <QString>

struct object
{
  QString MAC;
  double lenA,
         lenB,
         lenC,
         lenD;
  bool isA=false,
       isB=false,
       isC=false,
       isD=false;
  double x,y;
  bool isCords=false;
};


#endif // OBJECTS_H
