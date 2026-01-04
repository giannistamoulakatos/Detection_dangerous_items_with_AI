#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <thread>
#include <curl/curl.h>
#include "httplib.h"
//#include <Windows.h> //for windows

using namespace std;
using namespace cv;
using namespace dnn;
using namespace chrono;
using namespace this_thread;
using namespace httplib;



void webhook_function(const string &id,float confThreshold,int left,int top,int width,int height){
    CURL *data_curl_send = curl_easy_init();

    Client cli("https://localhost:8443");
    cli.enable_server_certificate_verification(false);

   
    string json_message = string("{\"message\":\"In this room one dangerous item is detected with top ") +to_string(top) + ", left " + to_string(left) + 
    ", width " + to_string(width) + ", and height " + to_string(height) + "\"}";

    cli.Post("/Data_receiving",json_message,"application/json");
    

}


int main(){

    VideoCapture cap(0);
    if(!cap.isOpened()){
        cout <<"Camera its not opening please try again ";
        return -1;
    }


    //for windows
    /*while(detection_dangerous_item=true)
    {
        Beep(1000,300);
        sleep(100);
        Beep(1000,300);
    }*/

    string modelconf="yolov4-tiny-custom.cfg";
    string modelweight="yolov4-tiny-custom_last.weights";
    string namesfile="obj.names";

    //load yolo files

    vector<string> yoloclassfiles;
    ifstream ifs ("obj.names");
    string line;

    while (getline(ifs, line)) {
        if (!line.empty()){
            yoloclassfiles.push_back(line);
            }
        }

    Net model=readNetFromDarknet(modelconf,modelweight);
    model.setPreferableBackend(DNN_BACKEND_OPENCV);
    model.setPreferableTarget(DNN_TARGET_CPU); 
    bool detection_dangerous_item = true;

    
    while (true)
    {   
        Mat frame_cam;
        cap >> frame_cam;
        vector<Rect> boxes;
        vector<int> classIds;
        vector<float> confidences;
        vector<int> indices;
        float confThreshold = 0.2; 
        float nmsThreshold = 0.5;

        Mat blob = blobFromImage(frame_cam, 1/255.0, Size(416,416), Scalar(0,0,0), true, false);
        model.setInput(blob);

        vector<Mat> outs;
        model.forward(outs, model.getUnconnectedOutLayersNames());
        
        

        for (auto &out : outs) {
            float *data = (float*)out.data;
            for (int i = 0; i < out.rows; ++i, data += out.cols) {
                Mat scores = out.row(i).colRange(5, out.cols);
                Point classIdPoint;
                double confidence;
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);

                if (confidence > confThreshold) {
                    int centerX = (int)(data[0] * frame_cam.cols);
                    int centerY = (int)(data[1] * frame_cam.rows);
                    int width   = (int)(data[2] * frame_cam.cols);
                    int height  = (int)(data[3] * frame_cam.rows);
                    int left    = centerX - width / 2;
                    int top     = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(Rect(left, top, width, height));
                    int id = classIdPoint.x;
                }
                    
                }   
                
            }
        NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    
        for (int idx : indices) {
            Rect box = boxes[idx];
            int classId = classIds[idx];
            string label = yoloclassfiles[classId] + " " + to_string((int)(confidences[idx]*100)) + "%";
            
            rectangle(frame_cam, box, Scalar(0, 255, 0), 2);
            putText(frame_cam, label, Point(box.x, box.y - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,0), 2);

            if(detection_dangerous_item){
                cout << "dangerous object detected" << endl;
                cout << "\a" << flush;
                sleep_for(milliseconds(300));
                webhook_function(label, confidences[idx], box.x, box.y, box.width, box.height);
            }

    }
    imshow("Detecting System",frame_cam);
    if(waitKey(30)==27){
        return 0;
    }
}
}
      

