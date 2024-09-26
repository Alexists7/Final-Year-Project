#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <ctime>
#include <fstream> // Include for file handling

using namespace std;

int main() {
    cv::VideoCapture cap(0); // Open the default camera

    if (!cap.isOpened()) {
        cout << "Failed to open the camera." << endl;
        return -1;
    }

    // Buffer to store 5 seconds of frames (assuming 30fps)
    const int fps = 30;
    const int bufferSize = fps * 5; // 5 seconds * 30 fps
    deque<cv::Mat> buffer;
    int motionCounter = 0;     // Counter for sustained motion
    int noMotionCounter = 0;   // Counter for no motion detected

    bool recording = false;
    bool motionTriggered = false;
    cv::VideoWriter video;
    string videoFileName;

    cv::Mat frame, prevFrame;

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

        // Convert frames to grayscale
        cv::Mat grayFrame, grayPrevFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::cvtColor(prevFrame, grayPrevFrame, cv::COLOR_BGR2GRAY);
        
        // Calculate difference and apply threshold
        cv::Mat diff;
        cv::absdiff(grayFrame, grayPrevFrame, diff);
        cv::threshold(diff, diff, 20, 255, cv::THRESH_BINARY); // Set a reasonable threshold

        // Morphological opening to remove noise
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(diff, diff, cv::MORPH_OPEN, kernel);
        
        // Find contours to detect motion
        vector<vector<cv::Point>> contours;
        cv::findContours(diff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        int motionCount = 0;
        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if (area > 500) { // Adjust this based on your environment
                motionCount++;
            }
        }

        // Debug output for motion detection
        cout << "Motion Count: " << motionCount << endl;

        // Buffer the frame
        buffer.push_back(frame.clone());
        if (buffer.size() > bufferSize) {
            buffer.pop_front(); // Remove oldest frame when buffer exceeds 5 seconds
        }

        // Detect motion and adjust counters
        if (motionCount > 0) {
            motionCounter++;
            noMotionCounter = 0; // Reset no-motion counter when motion is detected

            if (!motionTriggered && motionCounter >= fps * 2) { // Motion detected for 2 seconds
                motionTriggered = true;
                cout << "Motion sustained for 2 seconds. Start recording..." << endl;

                // Initialize video writer with the size of the frames in the buffer
                time_t now = time(0);
                tm *ltm = localtime(&now);
                char filename[100];
                sprintf(filename, "%04d-%02d-%02d_%02d:%02d:%02d.mp4", 
                        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday, 
                        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
                videoFileName = filename;

                // Open video writer
                video.open(videoFileName, cv::VideoWriter::fourcc('H', '2', '6', '4'), fps, frame.size());
                
                // Write all frames from the buffer to the video
                for (const auto& bufFrame : buffer) {
                    video.write(bufFrame);
                }

                recording = true; // Set recording to true after starting video writing
            }
        } else {
            // Increase the no-motion counter
            noMotionCounter++;
        }

        // Stop recording if no motion detected for 3 seconds
        if (motionTriggered && noMotionCounter >= fps * 3) { // No motion for 3 seconds
            cout << "No motion for 3 seconds. Stop recording..." << endl;

            // Calculate duration of motion in seconds
            int durationOfMotion = motionCounter / fps; // Motion duration in seconds
            video.release(); // Release the video writer

            // Create metadata file with the same name as the video
            string metadataFileName = videoFileName.substr(0, videoFileName.find_last_of('.')) + ".txt"; // Same name as video
            ofstream metadataFile(metadataFileName);
            if (metadataFile.is_open()) {
                metadataFile << "Date: " << videoFileName.substr(0, 10) << endl; // Extract date from filename
                metadataFile << "Time: " << videoFileName.substr(11, 8) << endl; // Extract time from filename
                metadataFile << "Duration of Motion: " << durationOfMotion << " seconds" << endl;
                metadataFile.close();
                cout << "Metadata saved to " << metadataFileName << endl;
            } else {
                cout << "Failed to create metadata file." << endl;
            }

            recording = false; // Reset recording flag
            motionTriggered = false; // Reset trigger for next motion detection
            buffer.clear(); // Clear buffer after stopping the recording
            motionCounter = 0; // Reset motion counter after stopping recording
        }

        // Write frame to video if recording is in progress
        if (recording) {
            video.write(frame);
        }

        prevFrame = frame.clone(); // Save current frame for next iteration

        // Display current frame for debugging
        cv::imshow("Current Frame", frame);

        if (cv::waitKey(1) == 27) { // Exit loop if ESC key is pressed
            break;
        }
    }

    cap.release(); // Release the camera
    cv::destroyAllWindows(); // Close all windows

    return 0;
}
