
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <dirent.h>
#include <string>

#include "fileio/TVRImporter.h"
#include "manage/MergingRoomMaker.h"
#include "logic/check.hpp"
#include "space_maker/OnlyWallSpaceMaker.h"

using namespace std;

void test(){
    Point_3 p1(-4.167291, 3.42887, -0.936589);
    Point_3 p2(-4.188357, 3.580666, -0.941703);
    Point_3 p3(-4.185606, 3.592505, -0.942622);

    Vector_3 vc1(p1,p2);
    Vector_3 vc2(p1,p3);
    cout << CGALCalculation::getAngle(vc1, vc2) << endl;
}

int main(int argc, const char * argv[]) {
    string import_version = "1.0.1.2";
    string exportVersion = "1.0.1.2";
    cout << exportVersion << endl;

    string projectPath = "/home/dongmin/dev/TriMeshToIndoor/";
    string resourcePath = projectPath + "Resource/tvr/";
    string resultPath = projectPath + "Result/";
    string fileName;
    cout << "write file name" << endl;
    cin >> fileName;
    const int maxGENperOneCycle = 20;
    string generationWritePath = resultPath + fileName+"/" + exportVersion + "/gen/";

//    TVRImporter::extractMINtvr(string(path) + string(fileName));
//    return 0;

    cout << "select mode" << endl;
    cout << "0 : make new Surfaces" << endl;
    cout << "1 : import Surfaces Generation" <<endl;
    cout << "2 : import and onlyJoin" <<endl;
    int mode; cin >> mode;

    RoomMaker* manager = new MergingRoomMaker();
    manager->setImporter(new TVRImporter());
    manager->setChecker(new Checker(0.0000001));

    cout << "Load TVR File.." << endl;
    if (manager->importMesh( (string(resourcePath) + fileName + ".tvr").c_str()) ){
        cout << "Load File Error";
        return -1;
    }

    // create Result directory
    if (boost::filesystem::exists(resultPath + fileName)){
        if (boost::filesystem::exists(resultPath + fileName + "/" + exportVersion)){
            DIR *theFolder = opendir(string(resultPath + fileName + "/" + exportVersion).c_str());
            struct dirent *next_file;
            char filepath[256];

            while ( (next_file = readdir(theFolder)) != NULL )
            {
                // build the path for each file in the folder
                sprintf(filepath, "%s/%s", string(resultPath + fileName + "/" + exportVersion).c_str(), next_file->d_name);
                remove(filepath);
            }
            closedir(theFolder);
        }
        else{
            boost::filesystem::create_directory(resultPath + fileName + "/" + exportVersion);
        }
    }
    else{
        boost::filesystem::create_directory(resultPath + fileName);
        boost::filesystem::create_directory(resultPath + fileName + "/" + exportVersion);
    }

    manager->setGenerationWriter(new GenerationWriter(generationWritePath));

    switch(mode){
        case 0:{
            if (manager->convertTriangleMeshToSpace(0.0)) return 1;
            //if (manager->exportCombined(exported_bin)) return 2;
            break;
        }
        case 1:{
            // TODO
            /*
            string import_path = string(result_path) + fileName + "/" + exportVersion + "/surfaces.bin";
            if (manager->importGeneration(import_path)) return 3;
            cout << "do simple?(y or n)" << endl;
            char ans_simple; cin >> ans_simple;
            if (ans_simple == 'y'){
                if (manager->makeSimpleSpaces(new OnlyWallSpaceMaker()) == -1) return -1;
                string simple_file = string(result_path) + fileName + exportVersion + "_solid.json";
                manager->exportSpaceJSON(simple_file);
            }
            */
            return 0;
        }
        case 2:{
            // TODO
            /*
            if (manager->importGeneration(imported_bin)) return 4;
            string json_file = string(result_path) + fileName + exportVersion + "_onlyJoin" + ".json";
            manager->exportSpaceJSON(json_file);
            */
            return 0;
        }
        default:{
            cout << "no Mode" << endl;
            return 0;
        }
    }

    cout << "simplify line?(y or n)" << endl;
    char ans_simple_line; cin >> ans_simple_line;
    bool simplifyLine = ans_simple_line == 'y';

    cout << "snap Surface?(y or n)" << endl;
    char ans_snap_surface; cin >> ans_snap_surface;
    bool snapSurface = ans_snap_surface == 'y';

    if (manager->constructSpace(new CombineParameter(maxGENperOneCycle, 1.00, simplifyLine, snapSurface)) == -1) return -1;

    string json_file = string(resultPath) + fileName + "/" + exportVersion + "/" + "surfaces.json";
    manager->exportSpaceJSON(json_file);
//    string export_bin_path = string(result_path) + fileName + "/" + exportVersion + "/surfaces.bin";
//    manager->exportCombined(export_bin_path);

    cout << "do simple?(y or n)" << endl;
    char ans_simple; cin >> ans_simple;
    if (ans_simple == 'y'){
        if (manager->makeSimpleSpaces(new OnlyWallSpaceMaker()) == -1) return -1;
        string simple_file = string(resultPath) + fileName + exportVersion + "_solid.json";
        manager->exportSpaceJSON(simple_file);
    }


    std::cout << "End!\n";
    return 0;
}
