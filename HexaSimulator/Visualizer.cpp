#include "Visualizer.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// uchar* ,255=white,0=black CV_8UC1
constexpr double SpecimenRadious = 1500 * EM6;
constexpr unsigned int PicSize = SpecimenRadious * 1000 * 100 * 2;
//*1000 m->mm; *100 100 units per mm;*2 radius->diameter;

void SavePic(Node_t** Nodes,unsigned int const& NodeCount)
{
	

	uchar(*Pic)[PicSize + 3] = new uchar[PicSize + 3][PicSize + 3];
	unsigned int PaintX = 0;
	unsigned int PaintY = 0;

	for (unsigned int x = 0; x < PicSize + 3; x++)
		for (unsigned int y = 0; y < PicSize + 3; y++)
			Pic[x][y] = 0;

	for (unsigned int NodeI = 1; NodeI < NodeMax; NodeI++)
	{
		Node_t*& NodeNow = Nodes[NodeI];
		

		PaintX = (NodeNow->x + SpecimenRadious) * 1000 * 100;
		PaintY = (NodeNow->y + SpecimenRadious) * 1000 * 100;

		//printf("%u,%u\n", PaintX, PaintY);

		Pic[PaintX][PaintY] = 255;
		Pic[PaintX - 1][PaintY] = 255;
		Pic[PaintX + 1][PaintY] = 255;
		Pic[PaintX][PaintY - 1] = 255;
		Pic[PaintX][PaintY + 1] = 255;

	}
	cv::Mat OutPic(PicSize + 3, PicSize + 3, CV_8UC1, Pic);
	//cv::imshow("Fig",OutPic);
	cv::imwrite("CrossSection.tiff", OutPic);
	/*unsigned char FigGt[3][3] = {255,255,0,255,0,255,0,0,0};
	cv::Mat OutPicT(3, 3, CV_8UC1, FigGt);
	cv::imwrite("stdpic.tiff", OutPicT);*/

	//printf("%u\n", PicSize);
	delete[] Pic;
	return;
}

void ShowTheta(Node_t** Nodes,unsigned int const & NodeCount)
{
	
	for (unsigned int NodeI = 1; NodeI < NodeMax; NodeI++)
	{
		Node_t*& NodeNow = Nodes[NodeI];


		printf("%.2lf\n", NodeNow->OffsetAngle);

	}
	return;
}
