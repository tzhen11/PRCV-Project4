/*
  Aafi Mansuri, Terry
  Mar 2026
  CS 5330 - Project 4: Calibration and Augmented Reality
  Main program - camera calibration using checkerboard pattern detection
*/
#include "settings.h"

int main(int argc, char* argv[]) {
    Settings s;
    const string inputSettingsFile = "sample.xml";
    // Read the settings
    FileStorage fs(inputSettingsFile, FileStorage::READ);

    if (!fs.isOpened()) {
        cout << "Couldn't open config file: \"" << inputSettingsFile << "\"" << endl;
        //parser.printMessage();
        return -1;
    }

    fs["Settings"] >> s;
    if (!s.goodInput) {
        cout << "Invalid input detected." << endl;
        return -1;
    }

    // Close settings file
    fs.release();

    std::vector<cv::Point2f> corner_set;
    vector<Vec3f> point_set;
    vector<vector<Vec3f>> point_list;
    vector<vector<Point2f>> corner_list;

    if (argc > 1) {
    s.inputCapture.open(argv[1]);
}

    // Build the 3D world points once
    for (int i = 0; i < s.boardSize.height; i++) {
        for (int j = 0; j < s.boardSize.width; j++) {
            // -i because (0,0,0) top left corner, moving down rows means -y direction
            point_set.push_back(Vec3f(j, -i, 0));
        }
    }

    // Track last successful detection
    Mat last_view;
    vector<Point2f> last_corners;
    bool last_found = false;

    for (;;) {
        Mat view = s.nextImage();
        if (view.empty()) break;

        // Retrieve image size
        Size image_size = view.size();

        bool found = findChessboardCorners(view, s.boardSize, corner_set,
            CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE | CALIB_CB_FAST_CHECK);

        if (found) {
            Mat viewGray;
            cvtColor(view, viewGray, COLOR_BGR2GRAY);
            cornerSubPix(viewGray, corner_set, Size(11, 11), Size(-1, -1),
                TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));

            //cout << "Corners found: " << corner_set.size() << endl;
            //cout << "First corner: " << corner_set[0].x << ", " << corner_set[0].y << endl;

            last_found = true;
            last_corners = corner_set;
            view.copyTo(last_view);
        }
        else {
            last_found = false;
        }

        drawChessboardCorners(view, s.boardSize, Mat(corner_set), found);
        imshow("Video", view);

        char key = cv::waitKey(5);
        if (key == 'q') break;
        //cout << "Last found: " << last_found << "\n";
        if (key == 's' && last_found) {
            corner_list.push_back(last_corners);
            point_list.push_back(point_set);
            cout << "Saved frame " << corner_list.size() << endl;
        }

        if (key == 'c') {
            if (corner_list.size() >= 5) {
                // Initialize camera matrix
                Mat camera_matrix = Mat::eye(3, 3, CV_64FC1);
                camera_matrix.at<double>(0, 2) = image_size.width / 2;
                camera_matrix.at<double>(1, 2) = image_size.height / 2;

                Mat distortion_coefficients = Mat::zeros(5, 1, CV_64FC1);
                vector<Mat> rotation_vecs, translation_vecs;

                cout << "Camera matrix before calibration:" << endl << camera_matrix << endl;
                cout << "Distortion coefficients before calibration:" << endl << distortion_coefficients << endl;

                // Run calibration
                double reprojection_error = calibrateCamera(
                    point_list,
                    corner_list,
                    image_size,
                    camera_matrix,
                    distortion_coefficients,
                    rotation_vecs,
                    translation_vecs,
                    CALIB_FIX_ASPECT_RATIO
                );

                cout << "Camera matrix after calibration:" << endl << camera_matrix << endl;
                cout << "Distortion coefficients after calibration:" << endl << distortion_coefficients << endl;
                cout << "Reprojection error: " << reprojection_error << endl;

                // save intrinsic parameters to file
                FileStorage out("intrinsics.xml", FileStorage::WRITE);
                out << "camera_matrix" << camera_matrix;
                out << "distortion_coefficients" << distortion_coefficients;
                out.release();
                cout << "Intrinsics saved to intrinsics.xml" << endl;
            }
            else {
                cout << "Not enough samples saved for calibration" << endl;
            }
        }
    }

    return 0;
}