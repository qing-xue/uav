
#include <opencv2/opencv.hpp>  
#include <iostream> 
#include <io.h> //文件
#include <fstream>

using namespace std;
using namespace cv;

string filename = "train\\data.txt"; //由后缀名决定文件类型
FileStorage fs;
CvSVM svm;

void getFiles(string path, vector<string>& files);
void writeTrainingData();
void writeToExcel(Mat outputImage, string fileName);
void getFeaturesOfSample(const Mat& img, vector<float>&  eigenvector);
static void trainSVM();
void slidingWnd(Mat& src, Mat& dst, Size& wndSize, double x_percent, double y_percent);
void predictAccuracy(); //测试分类类预测的准确率

int main()
{
	//【1】---------------------- 图像预处理----------------------
	//Mat srcImage = imread("719.jpg");
	//resize(srcImage, srcImage, Size(640, 480), 0, 0); //后期要对图像2做标准化处理；注意resize实现的方法
	//imshow("srcImage", srcImage);
	//imwrite("train//719.jpg", srcImage);

	//【2】---------------------- 开始训练、预测----------------------
	writeTrainingData(); //将训练数据写入文本
	trainSVM(); //使用SVM训练

	//-----------------------【SVM分类】----------------------
	svm.clear(); //清理，开始导入数据
	string modelpath = "train\\svm.xml";
	FileStorage svm_fs(modelpath, FileStorage::READ);
	if (svm_fs.isOpened())
	{
		svm.load(modelpath.c_str());
	}

	//----------------------【测试准确率】------------------------
	//predictAccuracy(); //在内部设置待测试的集合

	//-----------------------【滑窗处理】------------------------
	Mat testImage = imread("test/716.jpg");
	Mat dstImage(testImage.rows, testImage.cols, testImage.type(), Scalar(0));
	slidingWnd(testImage, dstImage, Size(7, 7), 0.3, 0.3); //注意1/4=0；滑窗与分类器、样本框大小耦合

	imshow("testImage", testImage);
	imshow("dstImage", dstImage);

	imwrite("test/result.jpg", dstImage);

	fs.release(); //关闭opencv文件流

	waitKey(0); //用getchar()在结束前似乎显示不了图片
	return 0;
}


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


void writeTrainingData()
{
	fs = FileStorage(filename, FileStorage::WRITE);

	char * filePath;
	vector<string> files;
	int number;

	Mat data;
	vector<int> labels;

	//【1】正样本
	filePath = "train\\+";
	getFiles(filePath, files); 
	number = files.size();
	cout << "共读取 " << number << " 张目标样本图。" << endl << endl;

	for (int i = 0; i < number; i++)
	{
		Mat src = imread(files[i].c_str());
		vector<float> vector; //特征向量
		getFeaturesOfSample(src, vector);
		//cout << "vector size: " << vector.size() << endl;
  
		data.push_back(Mat(vector).reshape(1, 1)); //会变成整型丢失精度？

		labels.push_back(1); //正样本标签
	}

	//【2】负样本
	filePath = "train\\-";
	files.clear(); //初始化
	getFiles(filePath, files);
	number = files.size();
	cout << "共读取 " << number << " 张背景样本图。" << endl << endl;

	for (int i = 0; i < number; i++)
	{
		Mat src = imread(files[i].c_str());
		vector<float> vector; //特征向量
		getFeaturesOfSample(src, vector);

		data.push_back(Mat(vector).reshape(1, 1));

		labels.push_back(0); //负样本标签
	}

	//【3】写入xml文件
	fs << "data" << data; // Write cv::Mat 
	fs << "labels" << Mat(labels);

	//【4】写入excel文件
	writeToExcel(data, "train\\data.xls"); 
	writeToExcel(Mat(labels), "train\\labels.xls");

}

void predictAccuracy() {

	char * filePath;
	vector<string> files;
	int count = 0;

	//【1】正样本
	filePath = "train\\test\\+";
	getFiles(filePath, files);
	cout << "共读取 " << files.size() << " 张目标样本图。" << endl;

	for (int i = 0; i < files.size(); i++)
	{
		Mat src = imread(files[i].c_str());
		vector<float> vector; //特征向量
		getFeaturesOfSample(src, vector);

		//SVM
		if (1 == (int)svm.predict(Mat(vector).reshape(1, 1)))
			count++;
		else
			cout << "Fig." << i << " 分类错误！" << endl;

		////RF
		//result = (int)etrees.predict(Mat(vector).reshape(1, 1));
		////cout << result << endl;
		//if (1 == result) 
		//	count++;
	}
	cout << "正样本预测正确率： " << count << "/" << files.size() << "=" << 1.0 * count / files.size() << endl;

	//【2】负样本
	filePath = "train\\test\\-";
	files.clear(); //初始化
	count = 0; //初始化
	getFiles(filePath, files);
	cout << "共读取 " << files.size() << " 张背景样本图。" << endl;

	for (int i = 0; i < files.size(); i++)
	{
		Mat src = imread(files[i].c_str());
		vector<float> vector; //特征向量
		getFeaturesOfSample(src, vector);

		int result = 0; //初始化为零

		//SVM
		if (0 == (int)svm.predict(Mat(vector).reshape(1, 1)))
			count++;
		else
		    cout << "Fig." << i << " 分类错误！" << endl;

		////RF
		//result = (int)etrees.predict(Mat(vector).reshape(1, 1));
		////cout << result << endl;
		//if (1 == result)
		//	count++;
	}
	cout << "负样本预测正确率： " << count << "/" << files.size() << "=" << 1.0 * count / files.size() << endl;
}

//在此方法里配置所要的特征向量
void getFeaturesOfSample(const Mat& src, vector<float>&  eigenvector) {

	//------------------------- 【1 颜色特征、分量特征】-----------------------------
	//------------------------------【BGR均值、标准差】-----------------------------
	Scalar mean, stddev;
	meanStdDev(src, mean, stddev);
	eigenvector.push_back(mean[0]);
	/*eigenvector.push_back(mean[1]);
	eigenvector.push_back(mean[2]);*/
	eigenvector.push_back(stddev[0]);
	//eigenvector.push_back(stddev[1]);
	eigenvector.push_back(stddev[2]);

	//------------------------ 【2 以下特征基于灰度图提取】---------------------------
	Mat gray; // 把原图像转化成灰度图像
	//cout << "src img's channels: " << src.channels() << endl;
	cvtColor(src, gray, CV_BGR2GRAY);

	//--------------------------------【矩特征】-----------------------------
	Moments mu = moments(gray, false);
	//此处仅添加二阶中心矩；还可以添加更多...
	//eigenvector.push_back(mu.nu20);
	eigenvector.push_back(mu.nu11);
	//eigenvector.push_back(mu.nu02);

}

//写入excel表格
//来源：http://livezingy.com/write-image-to-excel/
void writeToExcel(Mat outputImage, string fileName)
{
	ofstream Fs(fileName);
	if (!Fs.is_open())
	{
		cout << "error!" << endl;
		return;
	}

	int channels = outputImage.channels();            //获取图像channel  
	int nrows = outputImage.rows;                     //矩阵的行数  
	int ncols = outputImage.cols*channels;            //矩阵的总列数=列数*channel分量数  

	//循环用变量
	int i = 0;
	int j = 0;

	if (outputImage.depth() == CV_8U)//uchar
	{
		for (i = 0; i<nrows; i++)
		{
			for (j = 0; j<ncols; j++)
			{
				int tmpVal = (int)outputImage.ptr<uchar>(i)[j];
				Fs << tmpVal << '\t';
			}
			Fs << endl;
		}
	}
	else if (outputImage.depth() == CV_16S)//short
	{
		for (i = 0; i<nrows; i++)
		{
			for (j = 0; j<ncols; j++)
			{
				Fs << (short)outputImage.ptr<short>(i)[j] << '\t';
			}
			Fs << endl;
		}
	}
	else if (outputImage.depth() == CV_16U)//unsigned short
	{
		for (i = 0; i<nrows; i++)
		{
			for (j = 0; j<ncols; j++)
			{
				Fs << (unsigned short)outputImage.ptr<unsigned short>(i)[j] << '\t';
			}
			Fs << endl;
		}
	}
	else if (outputImage.depth() == CV_32S)//int 
	{
		for (i = 0; i<nrows; i++)
		{
			for (j = 0; j<ncols; j++)
			{
				Fs << (int)outputImage.ptr<int>(i)[j] << '\t';
			}
			Fs << endl;
		}
	}
	else if (outputImage.depth() == CV_32F)//float
	{
		for (i = 0; i<nrows; i++)
		{
			for (j = 0; j<ncols; j++)
			{
				Fs << (float)outputImage.ptr<float>(i)[j] << '\t';
			}
			Fs << endl;
		}
	}
	else//CV_64F double
	{
		for (i = 0; i < nrows; i++)
		{
			for (j = 0; j < ncols; j++)
			{
				Fs << (double)outputImage.ptr<double>(i)[j] << '\t';
			}
			Fs << endl;
		}
	}

	Fs.close();

}

static void trainSVM() {
	//获取训练数据
	Mat data, labels;
	fs.open(filename, FileStorage::READ);
	fs["data"] >> data; // Read cv::Mat  
	fs["labels"] >> labels; // Read cv::Mat  

	//配置SVM训练器参数
	CvSVMParams SVM_params;
	SVM_params.svm_type = CvSVM::C_SVC; //表示SVM分类器
	SVM_params.kernel_type = CvSVM::LINEAR;
	SVM_params.degree = 0;
	SVM_params.gamma = 1;
	SVM_params.coef0 = 0;
	SVM_params.C = 1;
	SVM_params.nu = 0;
	SVM_params.p = 0;
	SVM_params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 0.01);

	//训练
	svm.train(data, labels, Mat(), Mat(), SVM_params);

	//保存模型
	svm.save("train\\svm.xml");
	cout << "SVM分类器训练好了！！！" << endl;
}

//基于矩形窗口的图像滑动窗口操作,返回值为滑动窗口的数目
//参考：http://blog.csdn.net/jiamuju84/article/details/52893320
//@src 输入图像
//@dst 输出结果
//@wndSize 滑动窗口的大小
//@ x_percent 滑动窗口在x方向步长的百分比，x_step=x_percent*wndSize.width
//@ y_percent 滑动窗口在y方向步长的百分比，y_step=y_percent*wndSize.height
void slidingWnd(Mat& src, Mat& dst, Size& wndSize, double x_percent, double y_percent)
{
	int x_step = cvCeil(x_percent * wndSize.width); //向上取整？
	int y_step = cvCeil(y_percent * wndSize.height);

	int64 count1 = getTickCount();
	double freq = getTickFrequency();

	//利用窗口对图像进行遍历 
	for (int i = 0; i < src.rows - wndSize.height; i += y_step)
	{
		for (int j = 0; j < src.cols - wndSize.width; j += x_step)
		{
			Rect roi(Point(j, i), wndSize);
			Mat ROI = src(roi).clone();

			//得到滑框的特征、向量
			vector<float> vector; //特征向量
			getFeaturesOfSample(ROI, vector);

			//SVM
			if (1 == (int)svm.predict(Mat(vector).reshape(1, 1))) {
				ROI.copyTo(dst(roi));
			}
		}
	}
	int64 count2 = getTickCount();
	double time = (count2 - count1) / freq;
	cout << "\n滑框Time = " << time * 100 << "ms" << endl;
}

