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

    string json_message = string("{\"message\":\"In this room one dangerous item is detected with top ") +to_string(top) + ", left " + to_string(left) + 
    ", width " + to_string(width) + ", and height " + to_string(height) + "\"}";

    Client cli("https://127.0.0.1:5001");

    if(data_curl_send){
        CURLcode mc;
        struct curl_slist *json_header = NULL;
        curl_easy_setopt(data_curl_send,CURLOPT_URL,"https://127.0.0.1:5001");
        curl_easy_setopt(data_curl_send, CURLOPT_POST,1L);
        curl_easy_setopt(data_curl_send,CURLOPT_POSTFIELDS, json_message.c_str());
        json_header = curl_slist_append(json_header,"Type: Detection Program");
        curl_easy_setopt(data_curl_send,CURLOPT_HTTPHEADER,json_header);
        mc = curl_easy_perform(data_curl_send);
        curl_slist_free_all(json_header);
        //curl_easy_cleanup(data_curl_send);
        curl_easy_setopt(data_curl_send,CURLOPT_CAINFO,"server_program_code.ca.cert");
        
        
    }
    cli.Post("/client send data",json_message,"text/plain");
    
}


int main(){

    VideoCapture cap(0);
    if(!cap.isOpened()){
        cout <<"Camera its not opening please try again ";
        return -1;
    }

    string modelconf="yolov3.cfg";
    string modelweight="yolov3.weights";
    string namesfile="custom.names";
    vector<string> names;

    //load yolo files

    vector<string> yoloclassfiles;
    ifstream ifs (string("custom.names"));
    string line;
    
    while (true)
    {
        Mat frame_cam;
        cap.read(frame_cam);
        imshow("Detecting System",frame_cam);
        if(waitKey(30)==27){
            return 0;
        }
        
        
        
        cap >> frame_cam;
        Net model=readNetFromDarknet(modelconf,modelweight);
        Mat blob = blobFromImage(frame_cam, 1/255.0, Size(416, 416), Scalar(0,0,0), true, false);
        model.setInput(blob);

        vector<Mat> outs;
        model.forward(outs, model.getUnconnectedOutLayersNames());


        vector<int> classIds;
        vector<float> confidences;
        vector<Rect> boxes;
        float confThreshold = 0.5; 
        float nmsThreshold = 0.4;
        bool detection_dangerous_item = false;

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
                    detection_dangerous_item = true;
                    int id = classIdPoint.x;
                    string label = names[id];
                    webhook_function(label,confThreshold,left,top,width,height);
                }   
                if(detection_dangerous_item){
                    cout<<"dangerous object detected";
                    cout<<"/a";
                    sleep_for(milliseconds(300));
                }
                //for windows
                /*while(detection_dangerous_item=true)
                {
                    Beep(1000,300);
                    sleep(100);
                    Beep(1000,300);
                }*/
            }
            
        }
    
        vector<int> indices;
        NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

        
        for (int idx : indices) {
            Rect box = boxes[idx];
            int classId = classIds[idx];
            string label = yoloclassfiles[classId] + " " + to_string((int)(confidences[idx]*100)) + "%";

            rectangle(frame_cam, box, Scalar(0, 255, 0), 2);
            putText(frame_cam, label, Point(box.x, box.y - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,0), 2);
        }

    }
      

    
}