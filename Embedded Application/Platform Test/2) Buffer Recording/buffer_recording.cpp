#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

// Function to use RB5 camera, detect motion and record video
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

    string pipeline = "qtiqmmfsrc camera=" + to_string(camera_id) +
                      " ! video/x-raw,format=NV12,width=1280,height=720,framerate=30/1 ! "
                      "videoconvert ! video/x-raw,format=BGR ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened()) {
        cout << "Failed to open the camera." << endl;
        return -1;
    }

    cout << "Camera opened successfully." << endl;

    const int fps = 30;
    const int preMotionBufferSize = 150;
    const int postMotionFrames = 200;
    deque<cv::Mat> buffer;
    deque<cv::Mat> recordingBuffer;
    int motionCounter = 0;
    int noMotionCounter = 0;

    bool recording = false;
    bool motionTriggered = false;
    cv::VideoWriter video;
    string videoFileName;

    cv::Mat frame, prevFrame;

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

        vector<vector<cv::Point>> contours;
        cv::findContours(diff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        int motionCount = 0;
        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if (area > 500) {
                motionCount++;
            }
        }

        buffer.push_back(frame.clone());
        if (buffer.size() > preMotionBufferSize) {
            buffer.pop_front();
        }

        if (motionCount > 0) {
            motionCounter++;
            noMotionCounter = 0;

            if (!motionTriggered && motionCounter >= fps * 2) {
                motionTriggered = true;
                cout << "Motion sustained for 2 seconds. Start recording..." << endl;

                time_t now = time(0);
                tm *ltm = localtime(&now);

                string dataDir = "/data";
                if (access(dataDir.c_str(), F_OK) == -1) {
                    mkdir(dataDir.c_str(), 0777);
                }

                char dateDir[20];
                sprintf(dateDir, "%s/%04d-%02d-%02d", dataDir.c_str(), ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
                if (access(dateDir, F_OK) == -1) {
                    mkdir(dateDir, 0777);
                }

                char filename[100];
                sprintf(filename, "%s/%04d-%02d-%02d_%02d:%02d:%02d.mp4", dateDir,
                        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
                videoFileName = filename;

                video.open(videoFileName, cv::VideoWriter::fourcc('H', '2', '6', '4'), fps, frame.size());

                for (const auto& bufFrame : buffer) {
                    recordingBuffer.push_back(bufFrame);
                }
                buffer.clear();
                recording = true;
            }
        } else {
            noMotionCounter++;
        }

        if (motionTriggered && noMotionCounter >= postMotionFrames) {
            cout << "No motion for 3 seconds. Stop recording..." << endl;

            int durationOfMotion = motionCounter / fps;
            video.release();

            string metadataFileName = videoFileName.substr(0, videoFileName.find_last_of('.')) + ".txt";
            ofstream metadataFile(metadataFileName);
            if (metadataFile.is_open()) {
                size_t dateStart = videoFileName.find_last_of('/') + 1;
                metadataFile << "Date: " << videoFileName.substr(dateStart, 10) << endl;
                metadataFile << "Time: " << videoFileName.substr(dateStart + 11, 8) << endl;
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
            recordingBuffer.clear();
        }

        if (recording) {
            recordingBuffer.push_back(frame.clone());
            if (recordingBuffer.size() > 0) {
                video.write(recordingBuffer.front());
                recordingBuffer.pop_front();
            }
        }

        prevFrame = frame.clone();

        cv::imshow("Current Frame", frame);

        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
