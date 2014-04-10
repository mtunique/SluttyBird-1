#ifndef _DLL_SAMPLE_H
#define _DLL_SAMPLE_H

#include "cv.h"
using namespace cv;

#define MAX_OUTLINES 30
#define SUCCESS 0
#define FAILURE 1
#define OUTLINE_OVERFLOW 2


extern "C" _declspec(dllexport) void processEdges(Mat src, Mat &edgesMat, vector<vector<Point>> &contours_out, bool isSmooth = true, int thresh = 50);

extern "C" _declspec(dllexport) int imageAnalyzing(vector<vector<Point>> &contours_output, bool isSmooth = true, int thresh = 50);

extern "C" _declspec(dllexport) int imageRectAnalyzing(int &outlineNumber, char *str = "D:\\src_save.jpg", char *filename = "D:\\pointdata_A.txt",int thresh = 50);

extern "C" _declspec(dllexport) int uploadAnalyzing(int &outlineNumber, char *str = "D:\\src_up.jpg", char *filename = "D:\\pointdata_B.txt", int thresh = 50);

#endif