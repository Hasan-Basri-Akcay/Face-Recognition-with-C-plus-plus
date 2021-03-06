// ProjeFaceRecognition.cpp: Konsol uygulamasının giriş noktasını tanımlar.
//

#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include <opencv2/face.hpp>
#include <filesystem>

using namespace std;
using namespace cv;
using namespace cv::face;
namespace fs = std::experimental::filesystem;

void drawCircle(Mat frame, vector<Point2f> landmarks, Point &ptr1, Point &ptr2, string &duzen) {
	int xmin = 10000, xmax = 0;
	int ymin = 10000, ymax = 0;
	for (int i = 0; i < landmarks.size(); i++) {
		//circle(frame, Point(landmarks[i].x, landmarks[i].y), 2, Scalar(0, 0, 255), 3, 8, 0);
		if (landmarks[i].x < xmin) {
			xmin = landmarks[i].x;
		}
		else if (landmarks[i].x > xmax) {
			xmax = landmarks[i].x;
		}
		if (landmarks[i].y < ymin) {
			ymin = landmarks[i].y;
		}
		else if (landmarks[i].y > ymax) {
			ymax = landmarks[i].y;
		}
	}
	Point ptr11(xmin, ymin);
	Point ptr22(xmax, ymax);
	rectangle(frame, Point(ptr1.x - 1, ptr1.y - 1), Point(ptr2.x + 1, ptr2.y + 1), Scalar(0, 255, 0, 0), 1, 8, 0);
	const int toleransDuzen = 100;
	if ((landmarks[20].y - landmarks[23].y) > toleransDuzen || (landmarks[20].y - landmarks[23].y) < -toleransDuzen) {
		duzen = "Kafanizi Duz Tutun.";
	}
	else if (((landmarks[16].x- landmarks[26].x) - (landmarks[17].x - landmarks[0].x)) > (toleransDuzen+1) || ((landmarks[16].x - landmarks[26].x) - (landmarks[17].x - landmarks[0].x)) < -(toleransDuzen+1)) {
		duzen = "Kafanizi Cevirmeyin.";
	}
	/*else if ((landmarks[17].y - landmarks[0].y) > (toleransDuzen+2) || (landmarks[17].y - landmarks[0].y) < (toleransDuzen+2)) {
		duzen = "Kafanizi Egmeyin.";
	}*/
	else {
		duzen = "Dogru Pozisyon.";
	}
	ptr1 = ptr11;
	ptr2 = ptr22;
}
void show_histogram(cv::Mat1b const& image, vector<int> &vec)
{
	// Set histogram bins count
	int bins = 256;
	int histSize[] = { bins };
	// Set ranges for histogram bins
	float lranges[] = { 0, 256 };
	const float* ranges[] = { lranges };
	// create matrix for histogram
	cv::Mat hist;
	int channels[] = { 0 };

	// create matrix for histogram visualization
	int const hist_height = 256;
	cv::Mat3b hist_image = cv::Mat3b::zeros(hist_height, bins);

	cv::calcHist(&image, 1, channels, cv::Mat(), hist, 1, histSize, ranges, true, false);

	double max_val = 0;
	minMaxLoc(hist, 0, &max_val);

	// visualize each bin
	for (int b = 0; b < bins; b++) {
		float const binVal = hist.at<float>(b);
		int   const height = cvRound(binVal*hist_height / max_val);
		/*cv::line
		(hist_image
			, cv::Point(b, hist_height - height), cv::Point(b, hist_height)
			, cv::Scalar::all(255)
		);*/
		vec.push_back(height);
	}
	//cv::imshow(name, hist_image);
}
Mat LBP(const Mat imgsrc) {
	Mat img;
	//cout << "1" << endl;
	cvtColor(imgsrc, img, COLOR_BGR2GRAY);
	//cout << "2" << endl;
	Mat dst = Mat::zeros(img.rows - 2, img.cols - 2, CV_8UC1);
	for (int i = 1; i<img.rows - 1; i++) {
		for (int j = 1; j<img.cols - 1; j++) {
			uchar center = img.at<uchar>(i, j);
			unsigned char code = 0;
			code |= ((img.at<uchar>(i - 1, j - 1)) > center) << 7;
			code |= ((img.at<uchar>(i - 1, j)) > center) << 6;
			code |= ((img.at<uchar>(i - 1, j + 1)) > center) << 5;
			code |= ((img.at<uchar>(i, j + 1)) > center) << 4;
			code |= ((img.at<uchar>(i + 1, j + 1)) > center) << 3;
			code |= ((img.at<uchar>(i + 1, j)) > center) << 2;
			code |= ((img.at<uchar>(i + 1, j - 1)) > center) << 1;
			code |= ((img.at<uchar>(i, j - 1)) > center) << 0;
			if (code <= 255 || code >= 0) {
				dst.at<uchar>(i - 1, j - 1) = code;
			}
			else {
				cout << "Beklenmedik deger." << endl;
			}
		}
	}
	return dst;
}

int main()
{
	string komut = "",name = "",duzen="";
	cout << "ekle/bul ?" << endl;
	cin >> komut;
	if (komut=="ekle") {
		cout << "isim ?" << endl;
		cin >> name;
	}
	cout << endl;
	CascadeClassifier faceDetector("haarcascade_frontalface_alt2.xml");
	Ptr<Facemark> facemark = FacemarkLBF::create();
	facemark->loadModel("lbfmodel.yaml");

	Mat frame, gray;

	Point ptrSrcMin, ptrSrcMax;
	vector<int> histSrc;
	vector<Mat> imagesSrc;
	vector<vector<int>> ozniteliklerSrc;
	vector<int> histAna;
	int width, height;
	const int karesayisi = 7;
	Mat outputSrc;

	VideoCapture cam(0);
	while (cam.read(frame)) {
		vector<Rect> faces;
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		faceDetector.detectMultiScale(gray, faces);

		vector< vector<Point2f> > landmarks;
		bool success = facemark->fit(frame, faces, landmarks);
		if (success) {
			for (int i = 0; i < landmarks.size(); i++)
			{
				drawCircle(frame, landmarks[i], ptrSrcMin, ptrSrcMax,duzen);
				if (duzen == "Dogru Pozisyon.") {

					cvtColor(frame, frame, COLOR_BGR2YCrCb);
					vector<Mat> vec_channels;
					split(frame, vec_channels);
					equalizeHist(vec_channels[0], vec_channels[0]);
					merge(vec_channels, frame);
					cvtColor(frame, frame, COLOR_YCrCb2BGR);

					height = static_cast<int>((ptrSrcMax.y - ptrSrcMin.y) / karesayisi);
					width = static_cast<int>((ptrSrcMax.x - ptrSrcMin.x) / karesayisi);
					Rect orimage = Rect(ptrSrcMin.x, ptrSrcMin.y, ptrSrcMax.x - ptrSrcMin.x, ptrSrcMax.y - ptrSrcMin.y);
					outputSrc = frame(orimage);
					if ((height != 0) && (width != 0)) {
						for (int j = 0; j < karesayisi; j++) {
							for (int k = 0; k < karesayisi; k++) {
								Rect croppedRect = Rect((ptrSrcMin.x + j * width), (ptrSrcMin.y + k * height), width, height);
								Mat tempimage = frame(croppedRect);
								imagesSrc.push_back(tempimage);
							}
						}
					}
				}
			}
			if (duzen == "Dogru Pozisyon.") {
				for (int i = 0; i < (karesayisi*karesayisi); i++) {
					Mat temp = LBP(imagesSrc[i]);
					norm(temp,temp);
					show_histogram(temp, histAna);
					ozniteliklerSrc.push_back(histAna);
					histAna.clear();
				}
			}
			show_histogram(outputSrc,histSrc);
			int aydınlık = 0;
			for (int q = 0; q < histSrc.size(); q++) {
				aydınlık += histSrc[q];
			}
			stringstream ss;
			ss << aydınlık;
			string newMetin = ss.str();
			string text = "Aydinlik : " + newMetin;
			putText(frame, text, Point(15, 15), FONT_HERSHEY_COMPLEX_SMALL, 0.8, CV_RGB(0, 0, 200), 0.8);
			putText(frame, duzen, Point(15, 30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, CV_RGB(0, 0, 200), 0.8);
			if (komut=="ekle" && duzen=="Dogru Pozisyon.") {
				imwrite("Gallery Test/"+name+".jpg",frame);
				ofstream myfile;
				myfile.open("Labels Test/" + name + ".txt");
				for (int i = 0; i < ozniteliklerSrc.size(); i++) {
					myfile << i << endl;
					for (int j = 0; j < ozniteliklerSrc[i].size(); j++) {
						myfile << ozniteliklerSrc[i][j] << ",";
					}
					myfile << endl;
				}
				myfile.close();
			}
			else if (komut == "bul" && duzen == "Dogru Pozisyon.") {
				string pathDst = "Labels Test/";
				vector<int> HistfromFile;
				vector<vector<int>> ozniteliklerDst;
				string minFarkPath = "";
				const int toleransFark = 10500;
				unsigned int minFark = toleransFark;
				for (auto &p : fs::directory_iterator(pathDst)) {
					string imagePathDst = "";
					try {
						imagePathDst = pathDst + p.path().filename().string();
						ifstream myfileDst(imagePathDst);
						string line;
						if (myfileDst.is_open()) {
							while (getline(myfileDst, line))
							{
								if (line.length() > 5) {
									int oncekiyer = 0;
									for (int i = 0; i < line.length(); i++) {
										if (line[i] == ',') {
											string newvalue = line.substr(oncekiyer, (i - oncekiyer));
											HistfromFile.push_back(stoi(newvalue));
											oncekiyer = i + 1;
										}
									}
									ozniteliklerDst.push_back(HistfromFile);
									HistfromFile.clear();
								}
							}
							HistfromFile.clear();
							myfileDst.close();
							vector<int> tolerans;
							for (int i = 0; i < ozniteliklerDst.size(); i++) {
								if (i == 49) {
									break;
								}
								int temptolerans = 0;
								for (int j = 0; j < ozniteliklerDst[i].size(); j++) {
									int temptemp = ozniteliklerDst[i][j] - ozniteliklerSrc[i][j];
									if (temptemp < 0) {
										temptolerans = temptolerans - temptemp;
									}
									else {
										temptolerans = temptolerans + temptemp;
									}
								}
								tolerans.push_back(temptolerans);
							}
							unsigned int Topfark = 0;
							for (int i = 0; i < tolerans.size(); i++) {
								Topfark += tolerans[i]* tolerans[i];
							}
							Topfark = sqrt(Topfark);
							if (Topfark < minFark) {
								minFark = Topfark;
								minFarkPath = imagePathDst;
							}
						}
					}
					catch (char e) {

					}
					ozniteliklerDst.clear();
				}
				if (minFark < toleransFark) {
					cout << endl << "Eslestirme Bitti Sonuc: " << minFarkPath << endl;
					cout << "Fark : " << minFark << endl;
					putText(frame, minFarkPath.substr(12,minFarkPath.length()-15), Point(15, 45), FONT_HERSHEY_COMPLEX_SMALL, 0.8, CV_RGB(0, 0, 200), 0.8);
				}
				else {
					cout << endl << "Eslestirme Bitti Sonuc: BULUNAMADI..." << endl;
					putText(frame, "BULUNAMADI.", Point(15, 45), FONT_HERSHEY_COMPLEX_SMALL, 0.8, CV_RGB(0, 0, 200), 0.8);
				}
			}
			imshow("Facial Landmark Detection", frame);
		}
		// Exit loop if ESC is pressed
		if (waitKey(1) == 27) break;
		histSrc.clear();
		histAna.clear();
		ozniteliklerSrc.clear();
		imagesSrc.clear();
	}

	system("pause");
    return 0;
}

