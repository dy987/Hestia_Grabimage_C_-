// Hestia_Grabimage_C.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "affine.h"
#include "use_opencv.h"
#include "..\..\..\\include\Hestia_C.h"
#pragma comment(lib, "..\\..\\..\\lib\\Hestia_C.lib")
using namespace cv;


#define SQUARE(x)  ((x)*(x))

#define ADA 0  //�� ���� ����
#define RECORD_DATA 1  // ������ CSV���Ϸ� ����
#define CAL_AFFINE 0  //�󱼿��� ��ǥ�� �����ϱ����� AFFINE ��ȯ ���
#define VIEW_RGB 0
#define GRAPH_HEIGHT 600
#define DATA_COUNT 100
#define HEART_RATE_COUNT 5

void PrintInfomation(HESTIACONTEXT context);
int myDFT(const float *data, const int N, const int fps, float low_hz, float high_hz);
void Make_Graph(Mat graph, int *data, BOOL *time);


int _tmain(int argc, _TCHAR* argv[])
{
	HESTIACONTEXT m_HestiaContext = NULL;
	unsigned char* m_ImageData = NULL;
	unsigned short* m_TempData = NULL;
	HestiaError error;
	int nWidth(0), nHeight(0);
	int data[DATA_COUNT] = { 0 };
	float avr_data[DATA_COUNT] = { 0.0 };
	int heartRates[HEART_RATE_COUNT] = { 0 };

	BOOL timeLine[DATA_COUNT] = { FALSE };

	// Hestia Create
	error = Hestia_CreateContext(&m_HestiaContext);

	if (error == HESTIA_ERROR_OK)
		printf("OK : HestiaCreateContext()\r\n");
	else
		printf("Fail : HestiaCreateContext()\r\n");

	// Hestia UI connect
	error = Hestia_UI_Connect(m_HestiaContext);

	if (error == HESTIA_ERROR_OK)
		printf("OK : Hestia_IP_Connect()\r\n");
	else
		printf("Fail : Hestia_IP_Connect()\r\n");

	PrintInfomation(m_HestiaContext);
	Hestia_StartAcquisition(m_HestiaContext);


	//�� ���� ����
#if ADA
	String face_cascade = "C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml";
	String eye_cascade = "C:/opencv/sources/data/haarcascades/haarcascade_eye.xml";
	String img_name = "face.jpg"; // ���� ��

	CascadeClassifier face;
	CascadeClassifier eye;


	if (!face.load(face_cascade) || !eye.load(eye_cascade)) {
		cout << "Cascade ���� ���� ����" << endl;
		return -1;
	}
#endif

	Mat img; // ��ȭ�� ī�޶� ���� ����
	Mat img2; // ��ȭ�� ī�޶� ���� ����
	Mat img3; // ������ �̹��� ����
	Mat img_tmp;
	Mat img_grey; // ��ȭ�� ī�޶� �׷��� �÷� ����
	Mat img_rgb; // RGB ī�޶� ���� ����
	Mat img_tmp_grey;
	Mat gray;


	Mat Graph;
	Mat Graph_temp = Mat(GRAPH_HEIGHT, 1000, CV_8UC1);;

	Graph = Scalar(0);
	Graph_temp = Scalar(0);

	VideoCapture vc(0);
	double matrix[3][3];
	int selectNumber = 0;


#if RECORD_DATA
	// �ð��� ���� ������ ��ȭ�� ����
	FILE * fp_Pixel;
	fp_Pixel = fopen("Temp_Pixel.csv", "wt");// Ư���ʼ� ������ ���
	FILE * fp_aver;
	fp_aver = fopen("Temp_local_aver.csv", "wt"); // Ư�������� ������ ��հ� ���
#endif 

	int aver_x = 150;
	int aver_y = 100;
	int aver_size = 40;
	clock_t time_cur = clock();
	clock_t time_bef = 0;
	int FPS_before = 0;
	int FPS = 0;

	//��� ���͸�
	int aver_filter[5] = { 0 };

	//��Ʈ��Ī�� ���� ��
	int max_value = 1000;
	int min_value = 8000;


	for (int Frame_Count = 0; Frame_Count < 100000; ++Frame_Count)
	{
		//������ ǥ��
		time_cur = clock();
		if (time_cur - time_bef >= 1000) {
			time_bef = time_cur;
			FPS = Frame_Count - FPS_before;
			FPS_before = Frame_Count;
			timeLine[DATA_COUNT - 1] = TRUE;
			cout << "FPS: " << FPS << " " << Frame_Count << " " << FPS_before << endl;
		}
		else
			timeLine[DATA_COUNT - 1] = FALSE;

		error = Hestia_GetData(m_HestiaContext, &nWidth, &nHeight, HestiaType::HESTIA_GREY, 24, &m_ImageData, &m_TempData, false);


		img.release();
		img3.release();
		img_tmp_grey.release();
		gray.release();
		Graph.release();

		Graph = Mat(GRAPH_HEIGHT, 1000, CV_8UC1);
		Graph = Scalar(0);

		img = Mat(nHeight, nWidth, CV_8UC3);
		img3 = Mat(nHeight, nWidth, CV_8UC1);
		img_tmp_grey = Mat(nHeight, nWidth, CV_8UC1);
		gray = Mat(nHeight, nWidth, CV_8UC1);

		//RGB ī�޶� �ʱ� ����
		if (Frame_Count < 5) {
			vc.set(CV_CAP_PROP_FRAME_WIDTH, 640);
			vc.set(CV_CAP_PROP_FRAME_HEIGHT, 540);
		}

		//RGB ī�޶󿵻� �Է�
		vc >> img_rgb;
		if (img_rgb.empty()) {
			cout << "RGB ī�޶� ����" << endl;
			break;
		}
		cvtColor(img_rgb, gray, CV_RGB2GRAY);


		//�� ���� ���� �˰���
#if ADA
		vector<Rect> face_pos; // �� ��ġ ����
		face.detectMultiScale(gray, face_pos, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10)); // �� ����
#endif




#if RECORD_DATA
		FILE * fp;
		FILE * fp2;
		fp = fopen("Data_image1.csv", "wt");
		fp2 = fopen("Data_Temp.csv", "wt");
#endif 

		//�⺻ �׷��� �̹��� Mat �������� ����
		for (int i = 0; i < nHeight; i++)
			for (int j = 0; j < nWidth * 3; j++)
				img.at<Vec3b>(i, j / 3)[j % 3] = m_ImageData[i* nWidth * 3 + j];

		cvtColor(img, img_grey, CV_BGR2GRAY);

		//��ȭ�� ������ Mat �������� ����
		int temp_num = 405;

		for (int i = 0; i < nHeight; i++) {
			for (int j = 0; j < nWidth; j++) {
				img3.at<uchar>(i, j) = (int)m_TempData[i* nWidth + j] / 40;
			}
		}
		int sum = 0;
		int count = 0;
		if (1) {
			//������ �����͸� �̿��� ���� ����
			for (int i = aver_y; i < aver_y + aver_size; i++)
				for (int j = aver_x; j < aver_x + aver_size; j++) {
					int tem_val = (int)m_TempData[i* nWidth + j];
					int val = (tem_val >> 1) << 3;
					count++;
					sum += val;
				}
		}
		else {
			//��� ������ �̿��� ������ ����
			for (int i = aver_y; i < aver_y + aver_size; i++)
				for (int j = aver_x; j < aver_x + aver_size; j++) {
					unsigned char  val = img_grey.at<uchar>(i, j) << 4;
					count++;
					sum += val;
				}

		}

		float aver = (float)((sum / count));

		avr_data[DATA_COUNT - 1] = aver;
		if (timeLine[DATA_COUNT - 1] == TRUE) {
			int averHeartRate = 0;
			//��Ʈ��Ī ���� ���
			min_value = 1000000;
			max_value = -1;

			for (int i = DATA_COUNT - 50; i < DATA_COUNT; i++) {
				if (avr_data[i] > max_value)
					max_value = avr_data[i];

				if (avr_data[i] < min_value)
					min_value = avr_data[i];
			}


			if (avr_data[0] > 0) {
				for (int i = 1; i < HEART_RATE_COUNT; i++)
					heartRates[i - 1] = heartRates[i];

				heartRates[HEART_RATE_COUNT - 1] = myDFT(avr_data, DATA_COUNT, FPS, 0.7, 2.0);

				if (heartRates[0] > 0) {
					for (int i = 0; i < HEART_RATE_COUNT; i++)
						averHeartRate += heartRates[i];
				}
			}

			cout << "Val: " << aver << "  Max: " << max_value << "  Min: " << min_value << endl;
			cout << "Max-Min: " << max_value - min_value << "  STR_val: " << (aver - min_value) << endl << endl;
		}

		//double str_val = (aver - min_value) / ((max_value - min_value) + 1);
		//data[DATA_COUNT-1] = (int)(str_val * (GRAPH_HEIGHT*0.8));
		data[DATA_COUNT - 1] = (int)(aver - min_value) + 150;

		Make_Graph(Graph, data, timeLine);

		for (int i = 1; i < DATA_COUNT; i++)
			avr_data[i - 1] = avr_data[i];

#if RECORD_DATA
		if (Frame_Count > 10 && Frame_Count < 400) {
			fprintf(fp_aver, "%d,", (int)m_TempData[50 * nWidth + 50]);
			fprintf(fp_Pixel, "%d,", (int)img_grey.at<uchar>(50, 50));
		}
		else if (Frame_Count == 400) {
			fclose(fp_aver);
			fclose(fp_Pixel);
		}

		if (Frame_Count == 100) {
			/*for (int i = 0; i < nHeight; i++) {
				for (int j = 0; j < nWidth * 3; j++) {

					if (j % 3 == 0) {
						if (j / 3 == nwidth - 1)
							fprintf(fp, "%d\n", (int)m_imagedata[i* nwidth * 3 + j]);
						else
							fprintf(fp, "%d,", (int)m_imagedata[i* nwidth * 3 + j]);
					}

					if (j == nwidth - 1)
						fprintf(fp2, "%d\n", (int)m_tempdata[i* nwidth + j]);
					else
						fprintf(fp2, "%d,", (int)m_tempdata[i* nwidth + j]);

				}
			}*/

			fclose(fp);
			fclose(fp2);
		}

#endif 
		// ��ȭ�� ���� �����͸� �׷��̿������� ���� �� ������ Ȯ��


		//���� �̹��� ����
		for (int i = aver_x; i < aver_x + aver_size; i++)
			for (int j = aver_y; j < aver_y + aver_size; j++) {
				img_grey.at<uchar>(j, i) = img_grey.at<uchar>(j, i) << 4;
			}


		//RGB �� ��ȭ�� ī�޶� ����Ʈ ��ǥ�� �� ����Ʈ �迭
		int ori_point_x[4] = { 234,530,233,420 };
		int ori_point_y[4] = { 238,234,386,410 };

		int object_point_x[4] = { 194 / 2,534 / 2,198 / 2,420 / 2 };
		int object_point_y[4] = { 150 / 2,158 / 2,326 / 2,410 / 2 };

		Point_xy ori_point[4];
		Point_xy object_point[4];

		//���� �� ����Ʈ ��, ���� ��
		Scalar color(0, 0, 200);
		Scalar color2(255, 255, 255);

		if (Frame_Count < 2)
#if CAL_AFFINE
			for (int i = 0; i < 4; i++) {
				ori_point[i].setPoint(100 + i, 100 + i);
				object_point[i].setPoint(100 + i, 100 + i);
			}
#else			
			for (int i = 0; i < 4; i++) {
				ori_point[i].setPoint(ori_point_x[i], ori_point_y[i]);
				object_point[i].setPoint(object_point_x[i], object_point_y[i]);
			}
#endif



#if CAL_AFFINE
		//AFFINE ��� ���� ����Ʈ �˻�
		KeyInput(selectNumber, ori_point, object_point);

		//RGB �÷� �̹��� �� ǥ��
		for (int i = 0; i < 4; i++) {
			line(img_rgb, Point(ori_point[i].x, ori_point[i].y), Point(ori_point[i].x, ori_point[i].y), color, 1);
			line(img_grey, Point(object_point[i].x, object_point[i].y), Point(object_point[i].x, object_point[i].y), color2, 1);
		}
#endif


#if ADA 
		//�󱼿��� ǥ��
		for (int i = 0; i < (int)face_pos.size(); i++) {
			rectangle(img_rgb, face_pos[i], Scalar(0, 255, 0), 2);
		}
#endif

		//// AFFINE ��� ����ϱ�
		if (Frame_Count < 5)
			matrixcalc(&matrix[0][0], ori_point, object_point);

		for (int i = 0; i < 3; i++) {
			line(img_rgb, Point(ori_point[i].x, ori_point[i].y), Point(ori_point[i].x, ori_point[i].y), color, 1);
			line(img_grey, Point(object_point[i].x, object_point[i].y), Point(object_point[i].x, object_point[i].y), color2, 1);
		}

		//for (int i = 1; i < 4; i++) {
		//	object_point[i] = affinetransform_Point(ori_point[i], &matrix[0][0]);
		//	line(img_grey, Point(ori_point[i].x, ori_point[i].y), Point(ori_point[i].x, ori_point[i].y), color2, 5);
		//}

		//��ǥ���� �簢�� �׸���
		line(img_grey, Point(aver_x, aver_y), Point(aver_x + aver_size, aver_y), color2, 1);
		line(img_grey, Point(aver_x, aver_y), Point(aver_x, aver_y + aver_size), color2, 1);
		line(img_grey, Point(aver_x + aver_size, aver_y), Point(aver_x + aver_size, aver_y + aver_size), color2, 1);
		line(img_grey, Point(aver_x, aver_y + aver_size), Point(aver_x + aver_size, aver_y + aver_size), color2, 1);


		// ���� ��ȭȭ
		//equalizeHist(img3, img_tmp_grey);

		resize(img_grey, img_tmp, Size(nWidth * 2, nHeight * 2));

		//���� ȭ�鿡 ���
		imshow("�׷���", Graph);
		imshow("��ȭ��", img_tmp);
		imshow("RGBī�޶�", img_rgb);

		/*imshow("cam2", img3);
		imshow("cam3", img_tmp_grey);*/

#if RECORD_DATA
		/*	imwrite("img_grey.jpg", img_grey);
			imwrite("img_rgb.jpg", img_rgb);*/
#endif 

			//���� ������ ���� �� ���� ����
		char ch = waitKey(5);
		if (ch == 27) //ESC
			break;

		/*system("cls");*/

		/*for (int i = 0; i < 6; i++) {
			cout << i << ": ";
			if (i < 3)
				ori_point[i].printPoint();
			else
				object_point[i - 3].printPoint();
		}*/

		//error = Hestia_SaveAsBitmap(m_HestiaContext, NULL, filename, 24);
		//if (error == HESTIA_ERROR_OK)
		//	printf("OK : Hestia_SaveAsBitmap()\r\n");
		//else
		//	printf("Fail : Hestia_SaveAsBitmap()\r\n");
	}


	//��ȭ��ī�޶� ���� �ߴ�
	Hestia_StopAcquisition(m_HestiaContext);

	// ��ȭ�� ī�޶� ���� ��ü Disconnect
	error = Hestia_Disconnect(m_HestiaContext);
	if (error == HESTIA_ERROR_OK)
		printf("OK : HestiaDisconnect()\r\n");
	else
		printf("Fail : HestiaDisconnect()\r\n");

	// Hestia Release
	error = Hestia_Release(&m_HestiaContext);
	if (error == HESTIA_ERROR_OK)
		printf("OK : HestiaRelease()\r\n");
	else
		printf("Fail : HestiaRelease()\r\n");

	return 0;
}


void PrintInfomation(HESTIACONTEXT context)
{
	HestiaError error;

	char* Manufacturer = NULL;
	char* ModelName = NULL;
	char* IPAddress = NULL;
	char* MACAdress = NULL;
	int nWidth;
	int nHeight;

	error = Hestia_GetFLIRInformation(context, &Manufacturer, &ModelName, &IPAddress, &MACAdress, &nWidth, &nHeight);

	if (error == HESTIA_ERROR_OK)
		printf("OK : Hestia_GetFLIRInfomation()\r\n");
	else
		printf("Fail : Hestia_GetFLIRInfomation()\r\n");

	printf("Manufacturer :\t%s\r\n", Manufacturer);
	printf("Model        :\t%s\r\n", ModelName);
	printf("IP address   :\t%s\r\n", IPAddress);
	printf("MAC address  :\t%s\r\n", MACAdress);
	printf("Width        :\t%d\r\n", nWidth);
	printf("Height       :\t%d\r\n", nHeight);
}


void KeyInput(int& selectNumber, Point_xy* ori_point, Point_xy* object_point) {
	if (GetAsyncKeyState(VK_F1) || GetAsyncKeyState(VK_F2) || GetAsyncKeyState(VK_F3) || GetAsyncKeyState(VK_F4) || GetAsyncKeyState(VK_F5) || GetAsyncKeyState(VK_F6)) {
		if (GetAsyncKeyState(VK_F1))
			selectNumber = 0;
		if (GetAsyncKeyState(VK_F2))
			selectNumber = 1;
		if (GetAsyncKeyState(VK_F3))
			selectNumber = 2;
		if (GetAsyncKeyState(VK_F4))
			selectNumber = 3;
		if (GetAsyncKeyState(VK_F5))
			selectNumber = 4;
		if (GetAsyncKeyState(VK_F6))
			selectNumber = 5;
	}

	if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(0x41)) {

		if (GetAsyncKeyState(VK_UP)) {
			if (selectNumber < 3) {
				ori_point[selectNumber].y -= 4;
			}
			else {
				object_point[selectNumber - 3].y -= 4;
			}

		}
		if (GetAsyncKeyState(VK_DOWN)) {
			if (selectNumber < 3) {
				ori_point[selectNumber].y += 4;
			}
			else {
				object_point[selectNumber - 3].y += 4;
			}
		}
		if (GetAsyncKeyState(VK_LEFT)) {
			if (selectNumber < 3) {
				ori_point[selectNumber].x -= 4;
			}
			else {
				object_point[selectNumber - 3].x -= 4;
			}
		}
		if (GetAsyncKeyState(VK_RIGHT)) {
			if (selectNumber < 3) {
				ori_point[selectNumber].x += 4;
			}
			else {
				object_point[selectNumber - 3].x += 4;
			}
		}
		if (GetAsyncKeyState(0x41)) {

		}
	}
}

int myDFT(const float *data, const int N, const int fps, float low_hz, float high_hz)
{
	int padN = N * 10;
	float sampling_time = padN / (float)fps;
	float freq_resolution = 1 / sampling_time;
	int max_freq = padN / 2;
	int low = static_cast<int>(low_hz * sampling_time); // 42 bpm
	int high = static_cast<int>(high_hz * sampling_time); // 240 bpm

	cv::Mat planes[2] = { cv::Mat::zeros(padN, 1, CV_32F), cv::Mat::zeros(padN, 1, CV_32F) };
	cv::Mat complex;

	// ��հ� ���
	float sum = .0f;
	for (size_t i = 0; i < N; ++i)
		sum += data[i];

	float avg = sum / (float)N;

	// DC ���� ����
	for (int i = 0; i < N; ++i)
		planes[0].at<float>(i) = data[i] - avg;

	cv::merge(planes, 2, complex);

	// DFT ����
	cv::dft(complex, complex);
	split(complex, planes);

	// magnitude ���
	float *magnitude = new float[max_freq];
	for (int i = 0; i < max_freq; i++)
		magnitude[i] = sqrt(SQUARE(planes[0].at<float>(i) * planes[0].at<float>(i)) + SQUARE(planes[1].at<float>(i) * planes[1].at<float>(i)));

	// [low, high] ���� �� max magnitude Ž��
	int max_idx;
	float max_power = 0;
	for (int i = low; i <= high; ++i) {
		if (magnitude[i] > max_power) {
			max_power = magnitude[i];
			max_idx = i;
		}
	}

	delete[] magnitude;

	// �ɹڼ� ���
	int heartRates = static_cast<int>(max_idx * freq_resolution * 60.0f);

	return heartRates;
}

void Make_Graph(Mat graph, int *data, BOOL *time) {

	for (int i = 1; i < DATA_COUNT; i++) {

		line(graph, Point((i - 1) * 10, GRAPH_HEIGHT - data[i - 1]), Point(i * 10, GRAPH_HEIGHT - data[i]), Scalar(255, 255, 255), 3);

		if (time[i] == TRUE)
			line(graph, Point((i - 1) * 10, GRAPH_HEIGHT - 0), Point((i - 1) * 10, 0), Scalar(255, 0, 0), 1);
	}

	for (int i = 1; i < DATA_COUNT; i++) {
		data[i - 1] = data[i];
		time[i - 1] = time[i];
	}
	time[DATA_COUNT - 1] = FALSE;
}