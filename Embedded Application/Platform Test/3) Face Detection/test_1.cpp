#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <ctime>
#include <fstream>
#include <sys/stat.h>  // For mkdir
#include <unistd.h>    // For access
#include <thread>      // For threading
#include <atomic>      // For atomic variables
#include <string>
#include <cstdlib>
#include <csignal>     // For signal handling

using namespace std;

// Global flag for clean shutdown
atomic<bool> shutdown_requested(false);

// Function to handle termination signals
void signalHandler(int signum) {
    cout << "Interrupt signal (" << signum << ") received." << endl;
    shutdown_requested = true;
}

// Function to call the Python script for face recognition
void recognizeFace(const cv::Mat& faceImage) {
    // Save the face image as a temporary file
    string filename = "./test/detected_face.jpg";
    cv::imwrite(filename, faceImage);

    // Construct the command to call the Python script
    string command = "python3 recognise_face.py " + filename;
    cout << "[INFO] Calling Python script: " << command << endl;

    // Execute the command and read the output
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        cerr << "[ERROR] Failed to run command." << endl;
        return;
    }

    // Buffer to hold the output from the Python script
    char buffer[128];
    string result;

    // Read the output a line at a time - output it.
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer; // Append output to the result string
    }

    // Close the pipe
    pclose(pipe);

    // Print the result returned by the Python script
    cout << result << endl;

    // Check if the result starts with "Unknown"
    if (result.find("Unknown") == 0) {
        cout << "[INFO] Playing alarm sound..." << endl;
        system("ffplay -nodisp -autoexit alarm.mp3");
    }
}

int main() {
    // Register signal handlers for clean shutdown
    signal(SIGINT, signalHandler);  // Ctrl+C
    signal(SIGTERM, signalHandler); // Termination request

    // GStreamer pipeline string with NV12 format
    string pipeline = "qtiqmmfsrc camera=0"
                      " ! video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! "
                      "videoconvert ! video/x-raw,format=BGR ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER); // Open GStreamer pipeline

    if (!cap.isOpened()) {
        cout << "Failed to open the camera!" << endl;
        return -1;
    }

    // Create "data" directory if it doesn't exist
    if (access("data", F_OK) == -1) {
        mkdir("data", 0777);
    }

    // Create sub-directory with current date
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char dateDir[20];
    sprintf(dateDir, "data/%04d-%02d-%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
    if (access(dateDir, F_OK) == -1) {
        mkdir(dateDir, 0777);
    }

    // Create video file name with timestamp
    char videoFileName[100];
    sprintf(videoFileName, "%s/%04d-%02d-%02d_%02d:%02d:%02d.mp4", dateDir,
            ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

    // Open video writer immediately when program starts
    cv::VideoWriter video;
    video.open(videoFileName, cv::VideoWriter::fourcc('H', '2', '6', '4'), 30, cv::Size(640, 480));
    if (!video.isOpened()) {
        cout << "Failed to open video writer!" << endl;
        return -1;
    }

    cv::Mat frame, prevFrame;
    int motionCounter = 0;
    const int motionThreshold = 30; // Number of consecutive frames with motion needed to trigger recognition

    // Create metadata file with the same name as the video
    string metadataFileName = string(videoFileName).substr(0, string(videoFileName).find_last_of('.')) + ".txt";
    ofstream metadataFile(metadataFileName);
    if (metadataFile.is_open()) {
        size_t dateStart = string(videoFileName).find_last_of('/') + 1;
        metadataFile << "Date: " << string(videoFileName).substr(dateStart, 10) << endl;
        metadataFile << "Start Time: " << string(videoFileName).substr(dateStart + 11, 8) << endl;
        metadataFile << "Status: Recording started" << endl;
    }

    cout << "Recording started. Press ESC or send SIGINT/SIGTERM to stop recording." << endl;

    while (!shutdown_requested) {
        cap >> frame; // Capture frame from camera

        if (frame.empty()) {
            cout << "Failed to capture frame." << endl;
            break;
        }

        // Draw datetime in the top right corner of the frame
        now = time(0);
        ltm = localtime(&now);
        char datetimeStr[30];
        sprintf(datetimeStr, "%04d-%02d-%02d %02d:%02d:%02d",
                ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        cv::putText(frame, datetimeStr, cv::Point(frame.cols - 200, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

        // Write frame to video
        video.write(frame);

        // Motion detection (only process if we have a previous frame)
        if (!prevFrame.empty()) {
            // Convert frames to grayscale
            cv::Mat grayFrame, grayPrevFrame;
            cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
            cv::cvtColor(prevFrame, grayPrevFrame, cv::COLOR_BGR2GRAY);

            // Calculate difference and apply threshold
            cv::Mat diff;
            cv::absdiff(grayFrame, grayPrevFrame, diff);
            cv::threshold(diff, diff, 20, 255, cv::THRESH_BINARY);

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

            // If motion detected, increment counter
            if (motionCount > 0) {
                motionCounter++;

                // If motion sustained for enough frames, trigger face recognition
                if (motionCounter == motionThreshold) {
                    cout << "Motion detected! Triggering face recognition..." << endl;

                    // Take a picture of the current frame and call the recognition script
                    cv::Mat detectedFaceImage = frame.clone(); // Capture the current frame
                    std::thread recognizeThread(recognizeFace, detectedFaceImage);
                    recognizeThread.detach();
                }
            } else {
                // Reset counter if no motion
                motionCounter = 0;
            }
        }

        prevFrame = frame.clone(); // Save current frame for next iteration

        // Check for ESC key press
        if (cv::waitKey(1) == 27) {
            shutdown_requested = true;
        }
    }

    // Cleanup and shutdown
    cout << "Shutting down..." << endl;

    // Update metadata file with end time
    if (metadataFile.is_open()) {
        now = time(0);
        ltm = localtime(&now);
        char endTimeStr[20];
        sprintf(endTimeStr, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
        metadataFile << "End Time: " << endTimeStr << endl;
        metadataFile << "Status: Recording completed" << endl;
        metadataFile.close();
        cout << "Metadata saved to " << metadataFileName << endl;
    }

    // Release resources
    video.release();
    cap.release();
    cv::destroyAllWindows();

    cout << "Recording saved to " << videoFileName << endl;
    cout << "Program terminated successfully." << endl;

    return 0;
}
