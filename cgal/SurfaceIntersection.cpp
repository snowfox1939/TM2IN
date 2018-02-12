//
// Created by dongmin on 18. 1. 29.
//

#include <compute/SurfaceComputation.h>
#include <compute/VertexListComputation.h>
#include "SurfaceIntersection.h"
#include "PolygonComputation.h"
#include <bitset>

using namespace std;

std::vector<Surface *> SurfaceIntersection::resolveSelfIntersection(Surface * &pSurface) {
    int rec = 0;
    vector<Surface*> newSurfaceList;

    int number = pSurface->getVerticesSize();
    SurfaceIntersection::resolveEasySelfIntersection(pSurface);
    SurfaceComputation::removeStraight(pSurface);

    while (true){
        int result = makeNewIntersectionVertex(pSurface);
        if (result == 1) {
            break;
        }
        if (newSurfaceList.size() == 0){
            newSurfaceList.push_back(pSurface);
        }

        if (result == 2){
            cout << "Special case " << endl;
            break;
        }
        rec ++;
        if (rec % 20 == 0) cout << "rec : " << rec << endl;
        if (rec > number ) {
            cout << "Too much recursive? " << endl;
            break;
        }

        SurfaceIntersection::resolveEasySelfIntersection(pSurface);
        //SurfaceComputation::removeConsecutiveDuplicationIndex(pSurface);
    }

    return newSurfaceList;
}


/**
 *
 * @param pSurface
 * @return
 * 0 : made New Vertex.
 * 1 : no special Intersection
 * 2 : special case.
 */

int SurfaceIntersection::makeNewIntersectionVertex(Surface *&pSurface){
    vector<Point_2> pointsList = SurfaceComputation::to2D(pSurface, pSurface->planeRef);
    vector<Segment_2> segmentList = VertexListComputation::makeSegment2List(pointsList);
    // Intersection Point
    for (int i = 0 ; i < segmentList.size() - 2; i++){
        for (int j = i + 2 ; j < segmentList.size() ; j++){
            if (i == 0 && j == segmentList.size() - 1) continue;
            CGAL::cpp11::result_of<Intersect_2(Segment_2, Segment_2)>::type
                    result = CGAL::intersection(segmentList[i], segmentList[j]);
            if (result){
                if (const Point_2* p = boost::get<Point_2>(&*result)){
                    //cout << "point Intersect" << endl;

                    int pi = 0;
                    Point_3 point3 = pSurface->planeRef.to_3d(*p);
                    Vertex* vt = new Vertex(point3.x(), point3.y(), point3.z());
                    for (;pi < 2 ; pi++){
                        double distI = CGAL::squared_distance(*p, segmentList[i][pi]);
                        if (distI < Checker::squaredDistanceOfSamePoint2D){
                            break;
                        }
                    }
                    int pj = 0;
                    for (;pj < 2 ; pj++){
                        double distJ = CGAL::squared_distance(*p, segmentList[j][pj]);
                        if (distJ < Checker::squaredDistanceOfSamePoint2D){
                            break;
                        }
                    }

                    int pointI = pi + i == segmentList.size() ? 0 : pi + i;
                    int pointJ = pj + j == segmentList.size() ? 0 : pj + j;

                    if (pi != 2 && pj != 2){
                        if (pSurface->getVertex(pointI) != pSurface->getVertex(pointJ)){
                            pSurface->setVertex(pointI, vt);
                            pSurface->setVertex(pointJ, vt);
                        }
                        continue;
                    }

                    // put new Vertex. i < j 이므로 j 부터 넣기.
                    if (pj == 2){
                        pSurface->setVertex(pointI, vt);
                        int insertIndex = j + 1;
                        pSurface->insertVertex(insertIndex, vt);
                    }

                    if (pi == 2){ // put Vertex
                        pSurface->setVertex(pointJ, vt);
                        int insertIndex = i + 1;
                        pSurface->insertVertex(insertIndex, vt);
                    }

                    if (pi == 2 || pj == 2){
                        return 0;
                    }

                }
                else if (const Segment_2* seg = boost::get<Segment_2>(&*result)){
                    Vector_2 iVector = segmentList[i].to_vector();
                    Vector_2 jVector = segmentList[j].to_vector();

                    int angle = (int)CGALCalculation::getAngle(jVector, iVector);
                    if (angle == 0) {
                        printf ("0 degree : %d %d\n", i, j);
                        return 2;
                    }

                    Vector_2 segVector = seg->to_vector();
                    angle = (int)CGALCalculation::getAngle(iVector, segVector);
                    CGAL_assertion(angle == 180 || angle == 0);
                    if (angle == 180){
                        Segment_2 newSeg = seg->opposite();
                        seg = &newSeg;
                    }

                    Vector_2 segVector2 = seg->to_vector();
                    angle = (int)CGALCalculation::getAngle(iVector, segVector2);
                    CGAL_assertion(angle == 0);

                    //cout << "line Intersect" << endl;
                    Point_3 source = pSurface->planeRef.to_3d(seg->source());
                    Point_3 target = pSurface->planeRef.to_3d(seg->target());

                    // j
                    int sourceJ = 0, targetJ = 0;
                    for (; sourceJ < 2 ; sourceJ++){
                        double distJ = CGAL::squared_distance(seg->vertex(0), segmentList[j][sourceJ]);
                        if (distJ < Checker::squaredDistanceOfSamePoint2D)
                            break;
                    }

                    for (; targetJ < 2; targetJ++){
                        double distJ = CGAL::squared_distance(seg->vertex(0), segmentList[j][targetJ]);
                        if (distJ < Checker::squaredDistanceOfSamePoint2D)
                            break;
                    }

                    // i
                    int sourceI = 0, targetI = 0;
                    for (; sourceI < 2 ; sourceI++){
                        double distJ = CGAL::squared_distance(seg->vertex(0), segmentList[j][sourceI]);
                        if (distJ < Checker::squaredDistanceOfSamePoint2D)
                            break;
                    }

                    for (; targetI < 2; targetI++){
                        double distJ = CGAL::squared_distance(seg->vertex(0), segmentList[j][targetI]);
                        if (distJ < Checker::squaredDistanceOfSamePoint2D)
                            break;
                    }

                    int indexSourceI = sourceI + j == segmentList.size() ? 0 : sourceI + j;
                    int indexTargetI = targetI + j == segmentList.size() ? 0 : targetI + j;
                    int indexSourceJ = sourceJ + j == segmentList.size() ? 0 : sourceJ + j;
                    int indexTargetJ = targetJ + j == segmentList.size() ? 0 : targetJ + j;

                    if (sourceI == 1) return 2;
                    if (sourceJ == 0) return 2;
                    if (targetI == 0) return 2;
                    if (targetJ == 1) return 2;

                    CGAL_assertion(sourceI == 2 || sourceI == 0);
                    CGAL_assertion(sourceJ == 2 || sourceJ == 1);
                    CGAL_assertion(targetI == 2 || targetI == 1);
                    CGAL_assertion(targetJ == 2 || targetJ == 0);

                    bool sourceVertexOfIntersectionPointExist = (sourceJ != 2 && sourceI != 2);
                    bool targetVertexOfIntersectionPointExist = (targetJ != 2 && targetI != 2);
                    if (sourceVertexOfIntersectionPointExist){
                        if (pSurface->getVertex(indexSourceI) != pSurface->getVertex(indexSourceJ)){
                            pSurface->setVertex(indexSourceI, pSurface->getVertex(indexSourceJ));
                        }
                    }

                    if (targetVertexOfIntersectionPointExist){
                        if (pSurface->getVertex(indexTargetI) != pSurface->getVertex(indexTargetJ)){
                            pSurface->setVertex(indexTargetI, pSurface->getVertex(indexTargetJ));
                        }
                    }

                    if (sourceVertexOfIntersectionPointExist && targetVertexOfIntersectionPointExist){
                        CGAL_assertion(sourceJ != targetJ);
                        CGAL_assertion(sourceI != targetI);
                        continue;
                    }

                    // j
                    if (targetJ == 2 || sourceJ == 2){
                        if (targetJ == 2 && sourceJ == 2){
                            pSurface->insertVertex(j + 1, pSurface->getVertex(indexSourceI));
                            pSurface->insertVertex(j + 1, pSurface->getVertex(indexTargetI));
                        }
                        else if (targetJ == 2){
                            pSurface->insertVertex(j + 1, pSurface->getVertex(indexTargetI));
                        }
                        else if (sourceJ == 2){
                            pSurface->insertVertex(j + 1, pSurface->getVertex(indexSourceI));
                        }
                        return 0;
                    }

                    // i
                    if (targetI == 2 || sourceI == 2){
                        if (targetI == 2 && sourceI == 2){
                            pSurface->insertVertex(i + 1, pSurface->getVertex(indexTargetJ));
                            pSurface->insertVertex(i+1, pSurface->getVertex(indexSourceJ));
                        }
                        else if (targetI == 2){
                            pSurface->insertVertex(i + 1, pSurface->getVertex(indexTargetJ));
                        }
                        else if (sourceI == 2){
                            pSurface->insertVertex(i + 1, pSurface->getVertex(indexSourceJ));
                        }
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

#define GAP_FOR_SNAP 3

void SurfaceIntersection::resolveEasySelfIntersection(Surface *&pSurface) {
    vector<Point_2> pointsList = SurfaceComputation::to2D(pSurface, pSurface->planeRef);
    for (int i = 0 ; i < pointsList.size() - 1; i++){
        for (int j = 2 ; j <= GAP_FOR_SNAP ; j++){
            int nextIndex = i + j >= pointsList.size() ? i + j - pointsList.size() : i + j;
            Point_2 next = pointsList[nextIndex];
            if (CGAL::squared_distance(next, pointsList[i]) <= Checker::squaredDistanceOfSamePoint2D){
                if (nextIndex < i){
                    pointsList.erase(pointsList.begin() + i + 1, pointsList.end());
                    pointsList.erase(pointsList.begin(), pointsList.begin() + nextIndex + 1);

                    pSurface->removeVertexByIndex(i + 1, pSurface->getVerticesSize());
                    pSurface->removeVertexByIndex(0, nextIndex + 1);
                }
                else{
                    pointsList.erase(pointsList.begin() + i + 1, pointsList.begin() + nextIndex + 1);
                    pSurface->removeVertexByIndex(i + 1, nextIndex + 1);
                }
                break;
            }
        }
    }

}



/*
int SurfaceComputation::intersectionCount = 0;

void SurfaceComputation::resolveIntersectionByCGAL(Surface *&pSurface) {
    bool hasIntersection = true;
    while (hasIntersection){
        hasIntersection = false;
        vector<HalfEdge* > edges = pSurface->getboundaryEdgesList();
        vector<Segment_3> segments;
        for (int i = 0 ; i < edges.size() ; i++){
            segments.push_back(HalfEdgeComputation::getCGALSegment_3(edges[i]));
            delete edges[i];
        }

        for (int i = 0 ; i < segments.size() - 2; i++){
            for (int j = i + 2 ; j < segments.size() ; j++){
                if (i == 0 && j == segments.size() - 1) continue;
                if (CGAL::do_intersect(segments[i], segments[j])){
                    CGAL::cpp11::result_of<Intersect_3(Segment_3, Segment_3)>::type
                            result = CGAL::intersection(segments[i], segments[j]);
//                    if (result){
//                        if (const Point_3* p = boost::get<Point_3>(&*result)){
//                            std::cout << *p << endl;
//                        }
//                    }
                    intersectionCount++;
                    hasIntersection = true;

                    // remove Vertex.
                    pSurface->removeVertexByIndex(i + 1);
                    i = segments.size();
                    break;
                }
            }
        }
    }
    return;
}

Surface * SurfaceComputation::resolveIntersectionAndMakeNewSurface(Surface *&pSurface) {
    bool hasIntersect = true;
    while (hasIntersect){
        vector<Vertex*> vertexList = pSurface->getVerticesList();
        hasIntersect = false;
        for (int i = 0 ; i < vertexList.size() - 2 ; i++){
            for (int j = i + 1; j < vertexList.size() ; j++){
                if (vertexList[i] == vertexList[j]){
                    if ( j - i == 1){
                        //erase j
                        pSurface->removeVertexByIndex(j);
                    }
                    else if (j - i == 2){
                        //erase i + 1, j
                        pSurface->removeVertexByIndex(j);
                        pSurface->removeVertexByIndex(i+1);
                    }
                    else if (j - i > 2){
                        vector<Vertex*> newVertices;
                        for (int id = j ; id > i; id--){
                            newVertices.push_back(vertexList[id]);
                            pSurface->removeVertexByIndex(id);
                        }
                        Surface* newSurface = new Surface(newVertices);
                        return newSurface;
                    }
                    else{
                        exit(-1);
                    }
                    hasIntersect = true;
                    j = vertexList.size();
                    i = j;
                }
                else if (Checker::isSameVertex(vertexList[i], vertexList[j])){
                    cout << i <<" " <<  j << endl;
                    cout << vertexList[i]->toJSON() << endl;
                    cout << vertexList[j]->toJSON() << endl;
                    if ( j - i == 1){
                        //erase j
                        pSurface->removeVertexByIndex(j);
                    }
                    else if (j - i == 2){
                        //erase i + 1, j
                        pSurface->removeVertexByIndex(j);
                        pSurface->removeVertexByIndex(i+1);
                    }
                    else if (j - i > 2){
                        vector<Vertex*> newVertices;
                        for (int id = j ; id > i; id--){
                            newVertices.push_back(vertexList[id]);
                            pSurface->removeVertexByIndex(id);
                        }
                        Surface* newSurface = new Surface(newVertices);
                        return newSurface;
                    }
                    else{
                        exit(-1);
                    }
                    hasIntersect = true;
                    j = vertexList.size();
                    i = j;
                }
            }
        }
    }
    return NULL;
}


void SurfaceComputation::snapping(Surface *&pSurface) {
    std::vector<Vertex*> vertexList = pSurface->getVerticesList();
    vector<HalfEdge* > edges = pSurface->getboundaryEdgesList();
    vector<Segment_3> segments;
    for (int i = 0 ; i < edges.size() ; i++){
        segments.push_back(HalfEdgeComputation::getCGALSegment_3(edges[i]));
        delete edges[i];
    }

    for (int i = 0 ; i < vertexList.size() ; i++){
        int frontEdgeIndex = i - 1 >= 0? i-1: vertexList.size() - 1;
        int nextEdgeIndex = i;
        for (int ed = 0 ; ed < segments.size() ; ed++){
            if (ed == frontEdgeIndex || ed == nextEdgeIndex) continue;
            Point_3 point3 = VertexComputation::getCGALPoint(vertexList[i]);
            if (CGAL::squared_distance(point3, segments[ed]) < pow(Checker::threshold_vertex, 2)){
                Point_3 projected = segments[ed].supporting_line().projection(point3);
                vertexList[i]->setCoords(projected.x(), projected.y(), projected.z());
                break;
            }
        }

    }
}
 */