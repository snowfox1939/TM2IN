//
// Created by dongmin on 18. 3. 13.
//

#include <features/Space.h>

#ifndef TRIANGLEMESHTOCLEARSOLID_EXPORTER_H
#define TRIANGLEMESHTOCLEARSOLID_EXPORTER_H

class SpaceExporter{
public:
    virtual int exportSpace(vector<Space *> spaceList, const char *filePath) =0;
};


#endif //TRIANGLEMESHTOCLEARSOLID_EXPORTER_H