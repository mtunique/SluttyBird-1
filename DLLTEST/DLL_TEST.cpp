#include <iostream>
#include <stdio.h>
#include <fstream>

#include "cv.h"
#include "highgui.h"
#include "CameraDS.h"

#include "DLL_SAMPLE.h"

using namespace std;




extern "C" _declspec(dllexport) void processEdges(Mat src, Mat &edgesMat, vector<vector<Point>> &contours_out, bool isSmooth, int thresh)
{
	Mat src_gray, src_blur, src_canny;
	vector<vector<Point>> contours, contours_smooth;//edge list using the STL

	cvtColor(src, src_gray, CV_BGR2GRAY);//convert to gray
	blur(src_gray, src_blur, Size(3, 3));//blur for more precise
	Canny(src_blur, src_canny, thresh, thresh * 3, 3);//canny out the edges

	findContours(src_canny, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));//find the contours
	if(isSmooth == true)
	{
		contours_smooth = contours;
		for(int i = 0; i < contours.size(); i++)
		{
			approxPolyDP(contours[i], contours_smooth[i], 2, true);
		}
	}
	Mat out_line_img(src_canny.size(), CV_8UC1, Scalar(255));//generate an img to be the background
	drawContours(out_line_img, contours_smooth, -1, Scalar(0), 1);//draw the contours out on the background

	edgesMat = out_line_img;
	contours_out = contours_smooth;
}


extern "C" _declspec(dllexport) int imageAnalyzing(vector<vector<Point>> &contours_output, bool isSmooth, int thresh)
{
	//get ready for the camera and initialize some basic structures
	CCameraDS cameDS;
	int iCamCount = CCameraDS::CameraCount();
	int iCamNum = 0;


	IplImage *pFrame = 0 ;//used to capture the frame
	Mat sourceImage, edgesMat;//"sourceImage" is used to take over the frame from the pFrame////"edgesMat" is used to receive the outline image for showing on screen
	vector<vector<Point>> contours_smooth;

	//search for an available camera
	while(iCamNum < iCamCount)
	{
		if((!cameDS.OpenCamera(iCamNum, true)) || ((pFrame=cameDS.QueryFrame()) == NULL))
		{
			iCamNum++;
		}
		else
		{
			break;
		}
		cameDS.CloseCamera();
	}

	//if the camera doesn't work,tell the user
	if(iCamNum == iCamCount)
	{
		cout << "Cannot open camera" << endl;
		return -1;
	}	

	namedWindow("EdgesImage Shows Here",CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);//the canny edges image is in this window

	while(pFrame == cameDS.QueryFrame())
	{
		sourceImage = pFrame;

		processEdges(sourceImage, edgesMat, contours_output, isSmooth, thresh);//process every frame with thresh and outputs are an edgesMat & a series of xy

		imshow("EdgesImage Shows Here", edgesMat);
		int c = cvWaitKey(10);
		/*
		if(c == ' ')
		{
			imwrite("F:\\sourceImage.jpg", sourceImage);
			break;
		}
		*/
		if(c == ' ' || c == 27)
		{
			break;
		}
	}
	destroyWindow("EdgesImage Shows Here");

	namedWindow("Here's yours", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
	imshow("Here's yours", edgesMat);
	Sleep(2000);
	destroyWindow("Here's yours");

	cameDS.CloseCamera();
	cout << "Total outlines' number:" << contours_output.size() << endl;
	return 1;
}


extern "C" _declspec(dllexport) int imageRectAnalyzing(int &outlineNumber, char *str, char *filename, int thresh)
{
	//get ready for the camera and initialize some basic structures
	CCameraDS cameDS;
	int iCamCount = CCameraDS::CameraCount();
	int iCamNum = 0;


	IplImage *pFrame = 0 ;//used to capture the frame
	Mat sourceImage, edgesMat;//"sourceImage" is used to take over the frame from the pFrame////"edgesMat" is used to receive the outline image for showing on screen
	vector<vector<Point>> contours_smooth;
	Point2f rePoint[MAX_OUTLINES][4];

	RotatedRect returnRect[MAX_OUTLINES];

	//search for an available camera
	while(iCamNum < iCamCount)
	{
		if((!cameDS.OpenCamera(iCamNum, true)) || ((pFrame=cameDS.QueryFrame()) == NULL))
		{
			iCamNum++;
		}
		else
		{
			break;
		}
		cameDS.CloseCamera();
	}

	//if the camera doesn't work,tell the user
	if(iCamNum == iCamCount)
	{
		cout << "Cannot open camera" << endl;
		return FAILURE;
	}	

	namedWindow("EdgesImage Shows Here",CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);//the canny edges image is in this window

	while(pFrame == cameDS.QueryFrame())
	{
		sourceImage = pFrame;

		processEdges(sourceImage, edgesMat, contours_smooth, true, thresh);//process every frame with thresh and outputs are an edgesMat & a series of xy

		imshow("EdgesImage Shows Here", edgesMat);
		int c = cvWaitKey(10);
		
		if(c == ' ')
		{
			imwrite(str, sourceImage);
			break;
		}
	}
	destroyWindow("EdgesImage Shows Here");

	cout << "start to read from disk" << endl;

	sourceImage = imread(str);
	processEdges(sourceImage, edgesMat, contours_smooth, true, thresh);

	cout<< "process has been done"<<endl;

	outlineNumber = contours_smooth.size();

	if(outlineNumber > MAX_OUTLINES)
	{
		return OUTLINE_OVERFLOW;
	}

	for(int i = 0; i < contours_smooth.size(); i++)
	{
		returnRect[i] = minAreaRect(contours_smooth[i]);		
	}
	for(int i = 0; i < contours_smooth.size(); i++)
	{
		returnRect[i].points(rePoint[i]);
	}

	cout << "ready to write coodinaries to file" << endl;

	ofstream fout(filename);

	for(int i = 0; i < contours_smooth.size(); i++)
	{
		for(int j = 3; j >= 0; j--)
		{
			fout << rePoint[i][j].x << " " << rePoint[i][j].y << endl;
		}
		fout << "##########" << endl;
	}

	cameDS.CloseCamera();

	return SUCCESS;
}


extern "C" _declspec(dllexport) int uploadAnalyzing(int &outlineNumber, char *str, char *filename, int thresh)
{
	Mat sourceImage, edgesMat;
	vector<vector<Point>> contours_smooth;
	Point2f rePoint[MAX_OUTLINES][4];
	RotatedRect returnRect[MAX_OUTLINES];

	sourceImage = imread(str);//open the image to sourceImage
	processEdges(sourceImage, edgesMat, contours_smooth, true, thresh);

	outlineNumber = contours_smooth.size();//return the number of the outlines

	for(int i = 0; i < contours_smooth.size(); i++)
	{
		returnRect[i] = minAreaRect(contours_smooth[i]);//find the minAreaRectangle to the contours		
	}

	for(int i = 0; i < contours_smooth.size(); i++)
	{
		returnRect[i].points(rePoint[i]);//get the vertices of all the rectangles
	}

	ofstream fout(filename);

	for(int i = 0; i < contours_smooth.size(); i++)
	{
		for(int j = 3; j >= 0; j--)
		{
			fout << rePoint[i][j].x << " " << rePoint[i][j].y << endl;
		}
		fout << "##########" << endl;
	}

	return SUCCESS;
}
