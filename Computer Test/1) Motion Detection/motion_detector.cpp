#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

int main() {
    // For RB5, will need to start a tcp stream and open tcp stream here with opencv
    cv::VideoCapture cap(0); // Open the default camera

    if (!cap.isOpened()) {
        cout << "Failed to open the camera." << endl;
        return -1;
    }

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
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY); // Convert current frame to grayscale
        cv::cvtColor(prevFrame, grayPrevFrame, cv::COLOR_BGR2GRAY); // Convert previous frame to grayscale
        
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

        cv::imshow("Motion Detection", frame);

        if (cv::waitKey(1) == 27) { // Exit loop if ESC key is pressed
            break;
        }
    }

    cap.release(); // Release the camera
    cv::destroyAllWindows(); // Close all windows

    return 0;
}