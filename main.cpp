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
    }

    return 0;
}