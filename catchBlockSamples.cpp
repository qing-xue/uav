#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <stdio.h> 
#include <io.h> //文件
#include <iostream>
using namespace cv;
using namespace std;


//-----------------------------------【宏定义部分】--------------------------------------------
//  描述：定义一些辅助宏 
//------------------------------------------------------------------------------------------------ 
#define WINDOW_NAME  "【程序窗口】"    


//-----------------------------------【全局函数声明部分】------------------------------------
//		描述：全局函数的声明
//------------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param);
void saveBlock(cv::Mat& img, cv::Point point);


//-----------------------------------【全局变量声明部分】-----------------------------------
//		描述：全局变量的声明
//-----------------------------------------------------------------------------------------------
Rect g_rectangle; //用作存储滑块
int g_rectangle_length = 3; //小矩形框的大小;最好取奇数
//int g_rectangle_width = 7; 
//int g_rectangle_height = 28;
bool g_bDrawing = false;//是否进行绘制
int lineType = CV_AA; // change it to 8 to see non-antialiased graphics
char g_filename[4000]; //输出图片至文件夹
int g_imcount = 1; //输出图片至文件夹图片计数
Mat g_srcImage; //保留一份原图


void getFiles(string path, vector<string>& files)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
}

void resizeBatch(char* filePath) {

	vector<string> files;
	int number;
	getFiles(filePath, files);
	number = files.size();
	cout << "共读取 " << number << " 张目标样本图。" << endl << endl;

	char imgname[40];
	for (int i = 0; i < number; i++)
	{
		Mat src = imread(files[i].c_str());
		resize(src, src, Size(480, 320));
		sprintf_s(imgname, "%s%d%s", "output images\\", i, ".jpg"); //保存的图片名;注意g_filename[]可能溢出
		imwrite(imgname, src);
	}
}


//-----------------------------------【main( )函数】--------------------------------------------
//		描述：控制台应用程序的入口函数，我们的程序从这里开始执行
//-------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	//【0】批量归一化图片
	resizeBatch("F:\\workplace\\visual studio\\无人机\\datasets\\1\\src"); //图片文件夹

	////【1】准备参数
	//Mat tempImage;
	//g_srcImage   = imread("F:\\workplace\\visual studio\\无人机\\datasets\\1\\src\\org_01aa86052ff8b042_1527381319000.jpg");
	//if (g_srcImage.data == NULL) {
	//	printf("%s", "null img!");
	//}
	//g_srcImage.copyTo(tempImage);

	////【2】设置鼠标操作回调函数
	//namedWindow(WINDOW_NAME); //绘图窗口
	//setMouseCallback(WINDOW_NAME, on_MouseHandle, (void*)&tempImage);

	////【3】程序主循环，当进行绘制的标识符为真时，进行绘制
	//while (1)
	//{
	//	imshow(WINDOW_NAME, tempImage);
	//	if (waitKey(10) == 27) break;//按下ESC键，程序退出
	//}
	return 0;
}



//--------------------------------【on_MouseHandle( )函数】-----------------------------
//		描述：鼠标回调函数，根据不同的鼠标事件进行不同的操作；左键绘制，右键擦除
//-----------------------------------------------------------------------------------------------
void on_MouseHandle(int event, int x, int y, int flags, void* param)
{

	Mat& image = *(cv::Mat*) param;
	switch (event)
	{
	//左键按下消息
	case EVENT_LBUTTONDOWN:
	{
		g_bDrawing = true;
	}
	break;

	//左键抬起消息
	case EVENT_LBUTTONUP:
	{
		//调用函数进行绘制
		if (g_bDrawing) {
			//正方形框
			circle(image, cv::Point(x, y), g_rectangle_length / 2, cv::Scalar(0, 255, 0), 2, lineType); //在tempImage上进行绘制

			//矩形框
			//Rect rect(x - g_rectangle_width / 2 + 1, y - g_rectangle_height / 2 + 1, g_rectangle_width, g_rectangle_height);
			//rectangle(image, rect, Scalar(0, 0, 255), 1, lineType);

			saveBlock(g_srcImage, cv::Point(x, y)); //从原图截取样本框
		}
		
		g_bDrawing = false;
	}
	break;

	}
}


//-----------------------------------【saveBlock( )函数】------------------------------
//		描述：自定义的提取滑块函数
//-----------------------------------------------------------------------------------------------
void saveBlock(cv::Mat& img, cv::Point point)
{
	//转变为以当前点为【中心】绘制矩形
	int x0 = point.x - g_rectangle_length / 2 + 1;
	int y0 = point.y - g_rectangle_length / 2 + 1;
	if (x0 < 0 || x0 > img.cols - g_rectangle_length ||
		y0 < 0 || y0 > img.rows - g_rectangle_length)
	{
		return; // 下标越界则退出
	}
	g_rectangle = Rect(x0, y0, g_rectangle_length, g_rectangle_length); //记录起始点

	//int x0 = point.x - g_rectangle_width / 2 + 1;
	//int y0 = point.y - g_rectangle_height / 2 + 1;
	//if (x0 < 0 || x0 > img.cols - g_rectangle_width ||
	//	y0 < 0 || y0 > img.rows - g_rectangle_height)
	//{
	//	return; // 下标越界则退出
	//}
	//g_rectangle = Rect(x0, y0, g_rectangle_width, g_rectangle_height); //记录起始点
	
	//存放滑块
	Mat block = img(g_rectangle);
	sprintf_s(g_filename, "%s%d%s", "output images\\", g_imcount++, ".jpg"); //保存的图片名;注意g_filename[]可能溢出
	imwrite(g_filename, block);
}


