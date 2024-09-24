#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " [camera id: 0|1|2|3]" << endl;
        return -1;
    }

    int camera_id = atoi(argv[1]);
    if (camera_id < 0 || camera_id > 3) {
        cout << "Camera ID must be 0, 1, 2, or 3." << endl;
        return -1;
    }

    // GStreamer pipeline string with NV12 format
    string pipeline = "qtiqmmfsrc camera=" + to_string(camera_id) +
                      " ! video/x-raw,format=NV12,width=1280,height=720,framerate=30/1 ! "
                      "videoconvert ! video/x-raw,format=BGR ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER); // Open GStreamer pipeline

    if (!cap.isOpened()) {
        cout << "Failed to open the camera." << endl;
        return -1;
    }

    cout << "Camera opened successfully." << endl;

    cv::Mat frame, prevFrame;
    int motionCounter = 0;

    while (true) {
        cap >> frame; // Capture frame from camera

        if (frame.empty()) {
            cout << "Failed to capture frame." << endl;
            break;
        }

        if (prevFrame.empty()) {
            prevFrame = frame.clone();
            continue;
        }

        cv::Mat grayFrame, grayPrevFrame;
        
        // Convert current and previous frames to grayscale
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::cvtColor(prevFrame, grayPrevFrame, cv::COLOR_BGR2GRAY);
        
        cv::Mat diff;
        cv::absdiff(grayFrame, grayPrevFrame, diff); // Calculate frame difference
        
        cv::threshold(diff, diff, 30, 255, cv::THRESH_BINARY); // Apply threshold to highlight motion
        
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(diff, diff, cv::MORPH_OPEN, kernel); // Perform morphological opening to remove noise
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(diff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find contours
        
        int motionCount = 0;
        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if (area > 500) { // Adjust the threshold based on your needs
                motionCount++;
            }
        }
        
        if (motionCount > 0) {
            motionCounter++;
            cout << "Motion detected " << motionCounter << endl;
        }

        prevFrame = frame.clone();

        // Display the original frame
        // cv::imshow("Motion Detection", frame);

        if (cv::waitKey(1) == 27) { // Exit loop if ESC key is pressed
            break;
        }
    }

    cap.release(); // Release the camera
    cv::destroyAllWindows(); // Close all windows

    return 0;
}
