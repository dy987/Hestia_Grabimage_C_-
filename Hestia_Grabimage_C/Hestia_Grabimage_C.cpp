// Hestia_Grabimage_C.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "affine.h"
#include "use_opencv.h"

#include "..\..\..\\include\Hestia_C.h"
#pragma comment(lib, "..\\..\\..\\lib\\Hestia_C.lib")
using namespace cv;

#define ADA 0
#define RECORD_DATA 1
#define CAL_AFFINE 0
#define VIEW_RGB 0
#define GRAPH_HEIGHT 1000
void PrintInfomation(HESTIACONTEXT context);


void Make_Graph(Mat graph, int *data, BOOL *time) {

	for (int i = 1; i < 100; i++) {

		line(graph, Point((i - 1) * 10, GRAPH_HEIGHT - data[i - 1]), Point(i * 10, GRAPH_HEIGHT - data[i]), Scalar(255, 255, 255), 1);

		if (time[i] == TRUE)
			line(graph, Point((i - 1) * 10, GRAPH_HEIGHT - 0), Point((i - 1) * 10, 0), Scalar(255, 0, 0), 1);
	}

	for (int i = 1; i < 100; i++) {
		data[i - 1] = data[i];
		time[i - 1] = time[i];
	}
	time[99] = FALSE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	HESTIACONTEXT m_HestiaContext = NULL;
	unsigned char* m_ImageData = NULL;
	unsigned short* m_TempData = NULL;
	HestiaError error;
	int nWidth(0), nHeight(0);
	int data[100] = { 0 };
	BOOL timeLine[100] = { FALSE };

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


	//얼굴 영역 검출
#if ADA
	String face_cascade = "C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml";
	String eye_cascade = "C:/opencv/sources/data/haarcascades/haarcascade_eye.xml";
	String img_name = "face.jpg"; // 파일 명

	CascadeClassifier face;
	CascadeClassifier eye;


	if (!face.load(face_cascade) || !eye.load(eye_cascade)) {
		cout << "Cascade 파일 열기 실패" << endl;
		return -1;
	}
#endif

	Mat img; // 열화상 카메라 원본 저장
	Mat img2; // 열화상 카메라 원본 저장
	Mat img3; // 열정보 이미지 저장
	Mat img_tmp;
	Mat img_grey; // 열화상 카메라 그레이 컬러 변경
	Mat img_rgb; // RGB 카메라 영상 저장
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
	FILE * fp_local;
	fp_local = fopen("Temp_image_local.csv", "wt");
	FILE * fp_aver;
	fp_aver = fopen("Temp_local_aver.csv", "wt");
	int aver_x = 150;
	int aver_y = 100;
	int aver_size = 30;
#endif 

	clock_t time_cur = clock();
	clock_t time_bef = 0;
	int FPS_before = 0;
	int FPS = 0;

	//평균 필터링
	int aver_filter[5] = { 0 };



	//스트레칭을 위한 값
	int max_value = 1000;
	int min_value = 8000;


	for (int imageFrame = 0; imageFrame < 100000; ++imageFrame)
	{
		time_cur = clock();
		if (time_cur - time_bef >= 1000) {
			time_bef = time_cur;
			FPS = imageFrame - FPS_before;
			FPS_before = imageFrame;
			timeLine[99] = TRUE;
		}
		else
			timeLine[99] = FALSE;

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

		//RGB 카메라 초기 세팅
		if (imageFrame < 5) {
			vc.set(CV_CAP_PROP_FRAME_WIDTH, 640);
			vc.set(CV_CAP_PROP_FRAME_HEIGHT, 540);
		}

		//RGB 카메라영상 입력
		vc >> img_rgb;
		if (img_rgb.empty()) {
			cout << "RGB 카메라 에러" << endl;
			break;
		}
		cvtColor(img_rgb, gray, CV_RGB2GRAY);


		//얼굴 영역 검출 알고리즘
#if ADA
		vector<Rect> face_pos; // 얼굴 위치 저장
		face.detectMultiScale(gray, face_pos, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10)); // 얼굴 검출
#endif




#if RECORD_DATA
		/*FILE * fp;
		FILE * fp2;
		FILE * fp_local;
		fp = fopen("Data_image1.csv", "wt");
		fp2 = fopen("Data_Temp.csv", "wt");
		fp_local = fopen("Temp_image_local.csv", "wt");*/
#endif 

		//열화상 데이터 Mat 형식으로 변경
		int temp_num = 405;

#if RECORD_DATA
		for (int i = 0; i < nHeight; i++) {
			for (int j = 0; j < nWidth; j++) {
				img3.at<uchar>(i, j) = (int)m_TempData[i* nWidth + j] / 40;

				if (imageFrame < temp_num && i == 200 && j == 50) {
					fprintf(fp_local, "%d,", (int)m_TempData[i* nWidth + j]);
				}
				/*if (j == nWidth - 1)
					fprintf(fp2, "%d\n", (int)m_TempData[i* nWidth + j]);
				else
					fprintf(fp2, "%d,", (int)m_TempData[i* nWidth + j]);*/
			}
		}
		int sum = 0;
		int count = 0;


		for (int i = aver_x; i < aver_x + aver_size; i++)
			for (int j = aver_y; j < aver_y + aver_size; j++) {
				int val = (int)m_TempData[i* nWidth + j];
				count++;
				sum += val;
			}

		int aver = (int)((sum / count));



		//스트레칭 범위 계산
		if (imageFrame > 30 && imageFrame < 50) {
			if (min_value > aver);
			min_value = aver;
		}


		data[99] = (aver - 3700) * 8;

		if (timeLine[99] == TRUE) {
			cout << " FPS: " << FPS << " 평균 : " << aver << endl;

			for (int i = 0; i < nHeight; i++) {
				for (int j = 0; j < nWidth; j++) {

					if (min_value > (int)m_TempData[i* nWidth + j]);
					min_value = (int)m_TempData[i* nWidth + j];

					if (max_value > (int)m_TempData[i* nWidth + j]);
					max_value = (int)m_TempData[i* nWidth + j];
				}
			}

		}

		Make_Graph(Graph, data, timeLine);


		if (imageFrame > 10 && imageFrame < temp_num)
			fprintf(fp_aver, "%d,", (int)(sum / count));
		if (imageFrame == temp_num)
			fclose(fp_aver);

		if (imageFrame == temp_num)
			fclose(fp_local);
#endif 
		for (int i = 0; i < nHeight; i++) {
			for (int j = 0; j < nWidth * 3; j++) {
				img.at<Vec3b>(i, j / 3)[j % 3] = m_ImageData[i* nWidth * 3 + j];

#if RECORD_DATA
				/*if (j % 3 == 0) {
					if (j / 3 == nWidth - 1)
						fprintf(fp, "%d\n", (int)m_ImageData[i* nWidth * 3 + j]);
					else
						fprintf(fp, "%d,", (int)m_ImageData[i* nWidth * 3 + j]);
				}*/
#endif 
			}
		}


#if RECORD_DATA

		/*fclose(fp);
		fclose(fp2);*/

#endif 
		// 열화상 원본 데이터를 그레이영상으로 변경 및 사이즈 확대
		cvtColor(img, img_grey, CV_BGR2GRAY);


		//RGB 및 열화상 카메라 포인트 좌표값 및 포인트 배열
		int ori_point_x[4] = { 234,530,233,420 };
		int ori_point_y[4] = { 238,234,386,410 };

		int object_point_x[4] = { 194 / 2,534 / 2,198 / 2,420 / 2 };
		int object_point_y[4] = { 150 / 2,158 / 2,326 / 2,410 / 2 };

		Point_xy ori_point[4];
		Point_xy object_point[4];

		//영상 내 포인트 점, 라인 색
		Scalar color(0, 0, 200);
		Scalar color2(255, 255, 255);

		if (imageFrame < 2)
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
		//AFFINE 행렬 계산용 포인트 검색
		KeyInput(selectNumber, ori_point, object_point);

		//RGB 컬러 이미지 점 표시
		for (int i = 0; i < 4; i++) {
			line(img_rgb, Point(ori_point[i].x, ori_point[i].y), Point(ori_point[i].x, ori_point[i].y), color, 1);
			line(img_grey, Point(object_point[i].x, object_point[i].y), Point(object_point[i].x, object_point[i].y), color2, 1);
		}
#endif


#if ADA 
		//얼굴영역 표시
		for (int i = 0; i < (int)face_pos.size(); i++) {
			rectangle(img_rgb, face_pos[i], Scalar(0, 255, 0), 2);
		}
#endif

		//// AFFINE 행령 계산하기
		if (imageFrame < 5)
			matrixcalc(&matrix[0][0], ori_point, object_point);

		for (int i = 0; i < 3; i++) {
			line(img_rgb, Point(ori_point[i].x, ori_point[i].y), Point(ori_point[i].x, ori_point[i].y), color, 1);
			line(img_grey, Point(object_point[i].x, object_point[i].y), Point(object_point[i].x, object_point[i].y), color2, 1);
		}

		//for (int i = 1; i < 4; i++) {
		//	object_point[i] = affinetransform_Point(ori_point[i], &matrix[0][0]);
		//	line(img_grey, Point(ori_point[i].x, ori_point[i].y), Point(ori_point[i].x, ori_point[i].y), color2, 5);
		//}

		//목표지점 사각형 그리기
		line(img_grey, Point(aver_x, aver_y), Point(aver_x + aver_size, aver_y), color2, 1);
		line(img_grey, Point(aver_x, aver_y), Point(aver_x, aver_y + aver_size), color2, 1);
		line(img_grey, Point(aver_x + aver_size, aver_y), Point(aver_x + aver_size, aver_y + aver_size), color2, 1);
		line(img_grey, Point(aver_x, aver_y + aver_size), Point(aver_x + aver_size, aver_y + aver_size), color2, 1);


		// 영상 평화화
		//equalizeHist(img3, img_tmp_grey);

		resize(img_grey, img_tmp, Size(nWidth * 2, nHeight * 2));

		//영상 화면에 출력
		imshow("그래프", Graph);
		imshow("열화상", img_tmp);
		imshow("RGB카메라", img_rgb);

		/*imshow("cam2", img3);
		imshow("cam3", img_tmp_grey);*/

#if RECORD_DATA
		/*	imwrite("img_grey.jpg", img_grey);
			imwrite("img_rgb.jpg", img_rgb);*/
#endif 

			//영상 프레임 조절 및 종료 조건
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


	//열화상카메라 연결 중단
	Hestia_StopAcquisition(m_HestiaContext);

	// 열화상 카메라 연결 해체 Disconnect
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