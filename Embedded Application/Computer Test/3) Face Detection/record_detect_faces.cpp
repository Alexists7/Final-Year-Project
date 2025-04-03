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

using namespace std;

// Function to call the Python script for face recognition
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
}

// Function to record video when motion is detected and call facial_recognition function which further calls script
int main() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        cout << "Failed to open the camera." << endl;
        return -1;
    }

    const int fps = 30;
    const int preMotionBufferSize = fps * 4;
    const int postMotionFrames = fps * 6;
    deque<cv::Mat> buffer;
    deque<cv::Mat> recordingBuffer;
    int motionCounter = 0;
    int noMotionCounter = 0;

    bool recording = false;
    bool motionTriggered = false;
    cv::VideoWriter video;
    string mp4Name;
    string tempMp4Name;
    string videoFileName;

    cv::Mat frame, prevFrame;

    while (true) {
        cap >> frame;

        if (frame.empty()) {
            cout << "Failed to capture frame." << endl;
            break;
        }

        time_t now = time(0);
        tm *ltm = localtime(&now);
        char datetimeStr[30];
        sprintf(datetimeStr, "%04d-%02d-%02d %02d:%02d:%02d",
                ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        cv::putText(frame, datetimeStr, cv::Point(frame.cols - 200, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

        if (prevFrame.empty()) {
            prevFrame = frame.clone();
            continue;
        }

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

                if (access("data", F_OK) == -1) {
                    mkdir("data", 0777);
                }

                char dateDir[20];
                sprintf(dateDir, "data/%04d-%02d-%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
                if (access(dateDir, F_OK) == -1) {
                    mkdir(dateDir, 0777);
                }

                char videoname[100];
                sprintf(videoname, "%s/%04d-%02d-%02d_%02d.mp4", dateDir,
                        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                        ltm->tm_hour);
                mp4Name = videoname;
                tempMp4Name = "temp_output.mp4";

                char filename[100];
                sprintf(filename, "%s/%04d-%02d-%02d_%02d:%02d:%02d.mp4", dateDir,
                        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
                videoFileName = filename;

                video.open(tempMp4Name, cv::VideoWriter::fourcc('H', '2', '6', '4'), fps, frame.size());

                for (const auto& bufFrame : buffer) {
                    recordingBuffer.push_back(bufFrame);
                }
                buffer.clear();

                cv::Mat detectedFaceImage = frame.clone();
                std::thread recognizeThread(recognizeFace, detectedFaceImage);
                recognizeThread.detach();

                recording = true;
            }
        } else {
            noMotionCounter++;
        }

        if (motionTriggered && noMotionCounter >= postMotionFrames) {
            cout << "No motion for 3 seconds. Stop recording..." << endl;

            int durationOfMotion = motionCounter / fps;
            video.release();

            if (ifstream(mp4Name)) {
                string mergedFileName = "merged_output.mp4";
                string command = "ffmpeg -i " + mp4Name + " -i " + tempMp4Name +
                                    " -filter_complex concat=n=2:v=1:a=0 -y " + mergedFileName;
                system(command.c_str());

                remove(mp4Name.c_str());
                rename(mergedFileName.c_str(), mp4Name.c_str());

                cout << "Merged videos" << std::endl;
            } else {
                rename(tempMp4Name.c_str(), mp4Name.c_str());

                cout << "First instance of video, temp renamed" << std::endl;
            }

            remove(tempMp4Name.c_str());

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

        cv::imshow("Current Frame", frame);
        prevFrame = frame.clone();

        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
