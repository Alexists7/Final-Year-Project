#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

// Function to detect motion in a video stream
int main() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        cout << "Failed to open the camera." << endl;
        return -1;
    }

    cv::Mat frame, prevFrame;
    int motionCounter = 0;

    while (true) {
        cap >> frame;

        if (frame.empty()) {
            cout << "Failed to capture frame." << endl;
            break;
        }

        if (prevFrame.empty()) {
            prevFrame = frame.clone();
            continue;
        }

        cv::Mat grayFrame, grayPrevFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::cvtColor(prevFrame, grayPrevFrame, cv::COLOR_BGR2GRAY);

        cv::Mat diff;
        cv::absdiff(grayFrame, grayPrevFrame, diff);

        cv::threshold(diff, diff, 30, 255, cv::THRESH_BINARY);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(diff, diff, cv::MORPH_OPEN, kernel);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(diff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        int motionCount = 0;
        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if (area > 500) {
                motionCount++;
            }
        }

        if (motionCount > 0) {
            motionCounter++;
            cout << "Motion detected " << motionCounter << endl;
        }

        prevFrame = frame.clone();

        cv::imshow("Motion Detection", frame);

        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
