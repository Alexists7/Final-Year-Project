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

using namespace std;

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
    // GStreamer pipeline string with NV12 format
    string pipeline = "qtiqmmfsrc camera=0"
                      " ! video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! "
                      "videoconvert ! video/x-raw,format=BGR ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER); // Open GStreamer pipeline

    if (!cap.isOpened()) {
        cout << "Failed to open the camera!" << endl;
        return -1;
    }

    // Buffer to store frames
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
    string mp4Name;
    string tempMp4Name;
    string videoFileName;

    cv::Mat frame, prevFrame;

    while (true) {
        cap >> frame; // Capture frame from camera

        if (frame.empty()) {
            cout << "Failed to capture frame." << endl;
            break;
        }

        // Draw datetime in the top right corner of the frame immediately after capturing
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char datetimeStr[30];
        sprintf(datetimeStr, "%04d-%02d-%02d %02d:%02d:%02d", 
                ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday, 
                ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        cv::putText(frame, datetimeStr, cv::Point(frame.cols - 200, 30), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

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

                // Create "data" directory if it doesn't exist
                if (access("data", F_OK) == -1) {
                    mkdir("data", 0777);
                }

                // Create sub-directory with format YYYY-MM-DD
                char dateDir[20];
                sprintf(dateDir, "data/%04d-%02d-%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
                if (access(dateDir, F_OK) == -1) {
                    mkdir(dateDir, 0777);
                }

                // Construct file paths
                char videoname[100];
                sprintf(videoname, "%s/%04d-%02d-%02d_%02d.mp4", dateDir,
                        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                        ltm->tm_hour);
                mp4Name = videoname;
                tempMp4Name = "temp_output.mp4";

                // Construct file paths
                char filename[100];
                sprintf(filename, "%s/%04d-%02d-%02d_%02d:%02d:%02d.mp4", dateDir,
                        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
                        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
                videoFileName = filename;

                video.open(tempMp4Name, cv::VideoWriter::fourcc('H', '2', '6', '4'), fps, frame.size());

                // Write all frames from the buffer (2 seconds before motion)
                for (const auto& bufFrame : buffer) {
                    recordingBuffer.push_back(bufFrame);
                }
                buffer.clear();  // Clear the buffer after transferring frames

                // Take a picture of the current frame and call the recognition script
                cv::Mat detectedFaceImage = frame.clone(); // Capture the current frame
                std::thread recognizeThread(recognizeFace, detectedFaceImage); // Start recognition in a separate thread
                recognizeThread.detach(); // Detach the thread to allow it to run independently

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

            // Check if the main video file exists
            if (ifstream(mp4Name)) {
                // If the main video file exists, concatenate using FFmpeg's concat demuxer
                string mergedFileName = "merged_output.mp4"; // Temporary file for merged content

                // Create a temporary list file for the concat demuxer
                ofstream listFile("temp_list.txt");
                listFile << "file '" << mp4Name << "'\n";
                listFile << "file '" << tempMp4Name << "'\n";
                listFile.close();

                // Run FFmpeg command to concatenate using the concat demuxer
                string command = "ffmpeg -f concat -safe 0 -i temp_list.txt -c copy -y " + mergedFileName;
                system(command.c_str());

                // Remove the old video file and rename the merged file to the original name
                remove(mp4Name.c_str());
                rename(mergedFileName.c_str(), mp4Name.c_str());

                cout << "Merged videos" << std::endl;

                // Clean up the temporary list file
                remove("temp_list.txt");

            } else {
                // If the main video file does not exist, rename the temp file to main file
                rename(tempMp4Name.c_str(), mp4Name.c_str());

                cout << "First instance of video, temp renamed" << std::endl;
            }

            // Remove the temporary video file if it still exists
            remove(tempMp4Name.c_str());

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

        // Display current frame for debugging
        cv::imshow("Current Frame", frame);
        prevFrame = frame.clone(); // Save current frame for next iteration

        if (cv::waitKey(1) == 27) { // Exit loop if ESC key is pressed
            break;
        }
    }

    cap.release(); // Release the camera
    cv::destroyAllWindows(); // Close all windows

    return 0;
}