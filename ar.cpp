/*
  Aafi Mansuri, Terry
  Mar 2026
  CS 5330 - Project 4: Calibration and Augmented Reality
  AR program - pose estimation, projection, and virtual object rendering
*/

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    // Load intrinsics
    cv::FileStorage fs("intrinsics.xml", cv::FileStorage::READ);
    cv::Mat camera_matrix, distortion_coefficients;
    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> distortion_coefficients;
    fs.release();

    std::cout << "Camera matrix:\n" << camera_matrix << std::endl;
    std::cout << "Distortion coefficients:\n" << distortion_coefficients << std::endl;

    // Open camera
    cv::VideoCapture cap;
    if (argc > 1) cap.open(argv[1]);
    else cap.open(0);

    if (!cap.isOpened()) {
        std::cout << "Failed to open camera" << std::endl;
        return -1;
    }

    // Board setup
    cv::Size boardSize(9, 6);
    std::vector<cv::Vec3f> point_set;
    for (int i = 0; i < boardSize.height; i++) {
        for (int j = 0; j < boardSize.width; j++) {
            point_set.push_back(cv::Vec3f(j, -i, 0));
        }
    }

    cv::Mat frame, rvec, tvec;
    bool haspose = false;
    for (;;) {
        cap >> frame;
        if (frame.empty()) break;

        std::vector<cv::Point2f> corner_set;
        bool found = cv::findChessboardCorners(frame, boardSize, corner_set,
            cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

        if (found) {
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::cornerSubPix(gray, corner_set, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));
            
            // Pose estimation
            cv::solvePnP(point_set, corner_set, camera_matrix, distortion_coefficients, rvec, tvec);
            cv::drawChessboardCorners(frame, boardSize, corner_set, found);
            haspose = true;

            // Define 3D axis points
            std::vector<cv::Vec3f> axisPoints = {
                {0, 0, 0},   // origin
                {3, 0, 0},   // X axis
                {0, -3, 0},  // Y axis
                {0, 0, 3}    // Z axis (pointing up out of the board)
            };

            // Project to 2D
            std::vector<cv::Point2f> imagePoints;
            cv::projectPoints(axisPoints, rvec, tvec, camera_matrix, distortion_coefficients, imagePoints);

            // Draw axes
            cv::line(frame, imagePoints[0], imagePoints[1], cv::Scalar(255, 255, 0), 4); // X cyan
            cv::line(frame, imagePoints[0], imagePoints[2], cv::Scalar(200, 0, 255), 4); // Y purple
            cv::line(frame, imagePoints[0], imagePoints[3], cv::Scalar(0, 110, 255), 4); // Z orange

            // Flag pole
            std::vector<cv::Vec3f> flagPoints = {
                {1, -1, 0},   // pole base
                {1, -1, 5},   // pole top

                // Red strip (top)
                {1, -1, 5},   // top-left
                {5, -1, 5},   // top-right
                {5, -1, 4.33}, // bottom-right
                {1, -1, 4.33}, // bottom-left

                // White stripe (middle)
                {1, -1, 4.33}, // top-left
                {5, -1, 4.33}, // top-right
                {5, -1, 3.66}, // bottom-right
                {1, -1, 3.66}, // bottom-left

                // Blue stripe (bottom)
                {1, -1, 3.66}, // top-left
                {5, -1, 3.66}, // top-right
                {5, -1, 3},    // bottom-right
                {1, -1, 3}     // bottom-left
            };

            std::vector<cv::Point2f> proj;
            cv::projectPoints(flagPoints, rvec, tvec, camera_matrix, distortion_coefficients, proj);

            // Pole
            cv::line(frame, proj[0], proj[1], cv::Scalar(100, 100, 100), 3);

            // Red stripe
            std::vector<std::vector<cv::Point>> stripe;
            stripe = {{proj[2], proj[3], proj[4], proj[5]}};
            cv::fillPoly(frame, stripe, cv::Scalar(0, 0, 200));

            // White stripe
            stripe = {{proj[6], proj[7], proj[8], proj[9]}};
            cv::fillPoly(frame, stripe, cv::Scalar(255, 255, 255));

            // Blue stripe
            stripe = {{proj[10], proj[11], proj[12], proj[13]}};
            cv::fillPoly(frame, stripe, cv::Scalar(200, 0, 0));

            // Outline
            cv::line(frame, proj[2], proj[3], cv::Scalar(0, 0, 0), 1);
            cv::line(frame, proj[3], proj[12], cv::Scalar(0, 0, 0), 1);
            cv::line(frame, proj[12], proj[13], cv::Scalar(0, 0, 0), 1);
            cv::line(frame, proj[13], proj[2], cv::Scalar(0, 0, 0), 1);
        }

        cv::imshow("AR", frame);
        char key = cv::waitKey(10);
        if (key == 'q') break;
        if (key == 'p' && haspose){
            std::cout << "Rotation: " << rvec.t() << std::endl;
            std::cout << "Translation: " << tvec.t() << std::endl;
        }
        if (key == 's') {
            cv::imwrite("captured.jpg", frame);
            std::cout << "Frame saved to captured.jpg" << std::endl;
        }
    }
    return 0;
}