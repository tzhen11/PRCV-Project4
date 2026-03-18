/*
  Aafi Mansuri, Terry
  Mar 2026
  CS 5330 - Project 4: Calibration and Augmented Reality
  Feature detection program - detect and display robust features
*/

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    cv::VideoCapture cap;
    if (argc > 1) cap.open(argv[1]);
    else cap.open(0);

    if (!cap.isOpened()) {
        std::cout << "Failed to open camera" << std::endl;
        return -1;
    }

    cv::Ptr<cv::ORB> detector = cv::ORB::create(100);

    cv::Mat frame;
    for (;;) {
        cap >> frame;
        if (frame.empty()) break;

        std::vector<cv::KeyPoint> keypoints;
        detector->detect(frame, keypoints);

        cv::drawKeypoints(frame, keypoints, frame, cv::Scalar(0, 255, 0),
            cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        cv::imshow("Features", frame);
        char key = cv::waitKey(10);
        if (key == 'q') break;
        if (key == 's') {
            cv::imwrite("features.jpg", frame);
            std::cout << "Saved features.jpg" << std::endl;
        }
    }
    return 0;
}