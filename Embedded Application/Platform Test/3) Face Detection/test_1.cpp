#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <string>
#include <cstdlib>
#include <csignal>

using namespace std;

// First test script, used to compare against normal embedded program, continuous recording

atomic<bool> shutdown_requested(false);

void signalHandler(int signum) {
    cout << "Interrupt signal (" << signum << ") received." << endl;
    shutdown_requested = true;
}

void recognizeFace(const cv::Mat& faceImage) {
    string filename = "./test/detected_face.jpg";
    cv::imwrite(filename, faceImage);

    string command = "python3 recognise_face.py " + filename;
    cout << "[INFO] Calling Python script: " << command << endl;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        cerr << "[ERROR] Failed to run command." << endl;
        return;
    }

    char buffer[128];
    string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    cout << result << endl;

    if (result.find("Unknown") == 0) {
        cout << "[INFO] Playing alarm sound..." << endl;
        system("ffplay -nodisp -autoexit alarm.mp3");
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    string pipeline = "qtiqmmfsrc camera=0"
                      " ! video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! "
                      "videoconvert ! video/x-raw,format=BGR ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened()) {
        cout << "Failed to open the camera!" << endl;
        return -1;
    }

    if (access("data", F_OK) == -1) {
        mkdir("data", 0777);
    }

    time_t now = time(0);
    tm *ltm = localtime(&now);
    char dateDir[20];
    sprintf(dateDir, "data/%04d-%02d-%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
    if (access(dateDir, F_OK) == -1) {
        mkdir(dateDir, 0777);
    }

    char videoFileName[100];
    sprintf(videoFileName, "%s/%04d-%02d-%02d_%02d:%02d:%02d.mp4", dateDir,
            ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

    cv::VideoWriter video;
    video.open(videoFileName, cv::VideoWriter::fourcc('H', '2', '6', '4'), 30, cv::Size(640, 480));
    if (!video.isOpened()) {
        cout << "Failed to open video writer!" << endl;
        return -1;
    }

    cv::Mat frame, prevFrame;
    int motionCounter = 0;
    const int motionThreshold = 30;

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
        cap >> frame;

        if (frame.empty()) {
            cout << "Failed to capture frame." << endl;
            break;
        }

        now = time(0);
        ltm = localtime(&now);
        char datetimeStr[30];
        sprintf(datetimeStr, "%04d-%02d-%02d %02d:%02d:%02d",
                ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        cv::putText(frame, datetimeStr, cv::Point(frame.cols - 200, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

        video.write(frame);

        if (!prevFrame.empty()) {
            cv::Mat grayFrame, grayPrevFrame;
            cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
            cv::cvtColor(prevFrame, grayPrevFrame, cv::COLOR_BGR2GRAY);

            cv::Mat diff;
            cv::absdiff(grayFrame, grayPrevFrame, diff);
            cv::threshold(diff, diff, 20, 255, cv::THRESH_BINARY);

            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
            cv::morphologyEx(diff, diff, cv::MORPH_OPEN, kernel);

            vector<vector<cv::Point>> contours;
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

                if (motionCounter == motionThreshold) {
                    cout << "Motion detected! Triggering face recognition..." << endl;

                    cv::Mat detectedFaceImage = frame.clone();
                    std::thread recognizeThread(recognizeFace, detectedFaceImage);
                    recognizeThread.detach();
                }
            } else {
                motionCounter = 0;
            }
        }

        prevFrame = frame.clone();

        if (cv::waitKey(1) == 27) {
            shutdown_requested = true;
        }
    }

    cout << "Shutting down..." << endl;

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

    video.release();
    cap.release();
    cv::destroyAllWindows();

    cout << "Recording saved to " << videoFileName << endl;
    cout << "Program terminated successfully." << endl;

    return 0;
}
