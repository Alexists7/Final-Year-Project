#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <ctime>
#include <fstream>
#include <sys/stat.h>  // For mkdir
#include <unistd.h>    // For access

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

    // Buffer to store 2 seconds of frames (assuming 30fps)
    const int fps = 30;
    const int preMotionBufferSize = 150;  // 2 seconds of frames
    const int postMotionFrames = 200;     // 3 seconds of post-motion frames
    deque<cv::Mat> buffer;                    // Circular buffer for 2 seconds before motion
    deque<cv::Mat> recordingBuffer;           // Buffer to hold frames during recording
    int motionCounter = 0;                    // Counter for sustained motion
    int noMotionCounter = 0;                  // Counter for no motion detected

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
        cv::threshold(diff, diff, 30, 255, cv::THRESH_BINARY); // Set a reasonable threshold

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

        // Buffer the frame, ensuring we always have the last 2 seconds of frames
        buffer.push_back(frame.clone());
        if (buffer.size() > preMotionBufferSize) {
            buffer.pop_front(); // Remove oldest frame when buffer exceeds 2 seconds
        }

        // Detect motion and adjust counters
        if (motionCount > 0) {
            motionCounter++;
            noMotionCounter = 0; // Reset no-motion counter when motion is detected

            // Motion detected for 2 seconds, start recording
            if (!motionTriggered && motionCounter >= fps * 2) {
                motionTriggered = true;
                cout << "Motion sustained for 2 seconds. Start recording..." << endl;

                // Initialize video writer
                time_t now = time(0);
                tm *ltm = localtime(&now);

                // Create the "~/data" directory if it doesn't exist
                string dataDir = "/data";
                if (access(dataDir.c_str(), F_OK) == -1) {
                    mkdir(dataDir.c_str(), 0777);
                }

                // Create sub-directory with format YYYY-MM-DD
                char dateDir[20];
                sprintf(dateDir, "%s/%04d-%02d-%02d", dataDir.c_str(), ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
                if (access(dateDir, F_OK) == -1) {
                    mkdir(dateDir, 0777);
                }

                // Construct file paths
                char filename[100];
                sprintf(filename, "%s/%04d-%02d-%02d_%02d:%02d:%02d.mp4", dateDir,
                        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
                videoFileName = filename;

                video.open(videoFileName, cv::VideoWriter::fourcc('H', '2', '6', '4'), fps, frame.size());

                // Write all frames from the buffer (2 seconds before motion)
                for (const auto& bufFrame : buffer) {
                    recordingBuffer.push_back(bufFrame);
                }
                buffer.clear();  // Clear the buffer after transferring frames
                recording = true;
            }
        } else {
            noMotionCounter++;
        }

        // Stop recording if no motion detected for 3 seconds
        if (motionTriggered && noMotionCounter >= postMotionFrames) {
            cout << "No motion for 3 seconds. Stop recording..." << endl;

            int durationOfMotion = motionCounter / fps; // Motion duration in seconds
            video.release(); // Release the video writer

            // Create metadata file with the same name as the video
            string metadataFileName = videoFileName.substr(0, videoFileName.find_last_of('.')) + ".txt"; // Same name as video
            ofstream metadataFile(metadataFileName);
            if (metadataFile.is_open()) {
                size_t dateStart = videoFileName.find_last_of('/') + 1; // Find the start of the file name (after the last '/')
                metadataFile << "Date: " << videoFileName.substr(dateStart, 10) << endl; // Extract date (YYYY-MM-DD)
                metadataFile << "Time: " << videoFileName.substr(dateStart + 11, 8) << endl; // Extract time (HH:MM:SS)
                metadataFile << "Duration of Motion: " << durationOfMotion << " seconds" << endl;
                metadataFile.close();
                cout << "Metadata saved to " << metadataFileName << endl;
            } else {
                cout << "Failed to create metadata file." << endl;
            }

            recording = false;
            motionTriggered = false;
            motionCounter = 0;
            noMotionCounter = 0;
            recordingBuffer.clear();  // Clear the recording buffer
        }

        // Write frame to video if recording is in progress
        if (recording) {
            recordingBuffer.push_back(frame.clone());
            if (recordingBuffer.size() > 0) {
                video.write(recordingBuffer.front());
                recordingBuffer.pop_front();
            }
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
