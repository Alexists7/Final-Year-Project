#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>

using namespace std;

// Function to open camera, and take photos of individuals, which will be used later for training. Only really works in a Linux environment.

int main() {
    string name;

    cout << "Enter your name: ";
    getline(cin, name);

    string datasetPath = "dataset/" + name;
    std::filesystem::create_directories(datasetPath);

    string pipeline = "qtiqmmfsrc camera=0 ! video/x-raw,format=NV12,width=1280,height=720,framerate=30/1 ! "
                      "videoconvert ! video/x-raw,format=BGR ! appsink";

    cv::VideoCapture cam(pipeline, cv::CAP_GSTREAMER);

    if (!cam.isOpened()) {
        cerr << "Error: Unable to open the camera" << endl;
        return -1;
    }

    cv::namedWindow("Press SPACE to take a photo", cv::WINDOW_NORMAL);
    cv::resizeWindow("Press SPACE to take a photo", 500, 300);

    int img_counter = 0;
    while (true) {
        cv::Mat frame;
        cam >> frame;

        if (frame.empty()) {
            cerr << "Error: Unable to grab frame" << endl;
            break;
        }

        cv::imshow("Press SPACE to take a photo", frame);

        int k = cv::waitKey(1);
        if (k == 27) {
            cout << "Escape hit, closing..." << endl;
            break;
        } else if (k == 32) {
            string img_name = datasetPath + "/" + name + "_" + to_string(img_counter) + ".jpg";
            cv::imwrite(img_name, frame);
            cout << img_name << " written!" << endl;
            img_counter++;
        }
    }

    cam.release();
    cv::destroyAllWindows();
    return 0;
}
