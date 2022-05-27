#include <torch/script.h> // One-stop header.

#include <iostream>
#include <memory>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <torch/torch.h>
#include <bits/stdc++.h>
#include "inference.hpp" 
#include "uicom.hpp"
#include <sstream>

// Returns Total Area  of two overlap
// rectangles
double Inference::iou(Point l1, Point r1,
                      Point l2, Point r2)
{
    // Area of 1st Rectangle
    double area1 = abs(l1.x - r1.x)
            * abs(l1.y - r1.y);

    // Area of 2nd Rectangle
    double area2 = abs(l2.x - r2.x)
            * abs(l2.y - r2.y);

    // Length of intersecting part i.e
    // start from max(l1.x, l2.x) of
    // x-coordinate and end at min(r1.x,
    // r2.x) x-coordinate by subtracting
    // start from end we get required
    // lengths
    double x_dist = std::min(r1.x, r2.x)
            - std::max(l1.x, l2.x);
    double y_dist = (std::min(r1.y, r2.y)
                     - std::max(l1.y, l2.y));
    double areaI = 0;
    if( x_dist > 0 && y_dist > 0 )
    {
        areaI = x_dist * y_dist;
    }

    return (areaI/(area1 + area2 - areaI));
}





std::vector<Detection> Inference::infere(int len, const char* bytes) {

    std::vector<Detection> detections;
    std::vector<uchar> data = std::vector<unsigned char>(bytes, bytes + len);;
    cv::Mat img = cv::imdecode(data,cv::IMREAD_UNCHANGED);
    if(img.data==NULL){
        std::cout<<"no valid image data"<<std::endl;
        wasValidImageData = false;
        return detections;
    }
    std::cout<<"decoded image: "<<img.cols<<" x " <<img.rows<<std::endl;

    // cv::imshow("imageres", img);
    // char c=(char)cv::waitKey(25);



    std::vector<torch::jit::IValue> inputs;
    torch::Tensor in = read_image(img);
    inputs.push_back(in);

    //    //  torch::Tensor output = torch::softmax(module.forward(inputs).toTensor(), 1);
    torch::Tensor output = module.forward(inputs).toTensor();
    //    std::vector<Detection> detections;
    //    std::cout<<output.toString()<<std::endl;

    //    std::cout<<" output.sizes.size: "<<output.sizes().size()<<std::endl;
    //    for (int i = 0; i < output.sizes().size(); ++i) {
    //        std::cout<<"  "<<i<<" "<<output.sizes().at(i)<<std::endl;

    //    }
    torch::Tensor det = output[0];



    for (int i = 0; i < output.sizes().at(1); ++i) { //tresholding by confidence

        torch::Tensor det0 = det[i];
        if( !std::isnan( det0[0].item<double>()) && det0[5].item<double>() > 0.3 ){

            std::cout<<det0[0].item<double>()<<" "<<det0[1].item<double>()<<" "<<det0[2].item<double>()<<" "<<det0[3].item<double>()<<" "<<det0[4].item<double>()<<" "<<det0[5].item<double>()<<" "<<std::endl;

            detections.push_back(Detection{det0[0].item<double>(),det0[1].item<double>(),det0[2].item<double>(),det0[3].item<double>(),det0[4].item<double>(),det0[5].item<double>()});


        }

    }

    //sort by confidence
    std::sort (detections.begin(), detections.end());
    std::cout<<"sorted "<<std::endl;
    //    for (int i = 0; i < detections.size(); ++i) {
    //        std::cout<<"conf: "<<detections.at(i).c <<std::endl;

    //    }
    // erase one of two overlaping boxes with less confidence
    std::reverse(detections.begin(),detections.end());  //reverse vector so that highetst confidences is at begining
    for (int i = 0; i < detections.size(); ++i) {
        for (int j = i+1; j < detections.size(); ++j) {
            Detection a = detections.at(i);
            Detection b = detections.at(j);

            double ioua = iou(Point{a.x,a.y},Point{a.x+a.w,a.y+a.h},Point{b.x,b.y},Point{b.x+b.w,b.y+b.h});
            if(ioua>=0.45){// overlapping

                detections.erase(detections.begin()+j);
                j--;//go back one index, because we shifted elements in list
            }
        }
    }
    std::cout<<"nms done, det size: "<<detections.size() <<std::endl;
    for (int i = 0; i < detections.size(); ++i) {
        std::cout<<detections.at(i).x <<" "<<detections.at(i).y<<" a: "<<detections.at(i).a << " conf: "<<detections.at(i).c <<std::endl;

    }
    //drawBoxes("/home/arturs/Projects/RAPiD/images/exhibition.jpg",detections);

    std::cout<<"done, found : "<<detections.size()<<std::endl;

    // UiCom::sendCount(detections.size());
    UiCom::sendLocations(makeLocationsString(detections));
    return detections;
}

void Inference::init()
{
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load("./traced_rapid_model.pt");
    }
    catch (const c10::Error& e) {
        std::cerr << "error loading the model\n";
        return ;
    }

    std::cout << "model loaded"<<std::endl;

}

Inference::Inference()
{
    init();
}

void  Inference::drawBoxes(const std::string& imageName, std::vector<Detection> detections){
    cv::Mat img = cv::imread(imageName);
    img = crop_center(img);

    cv::resize(img, img, cv::Size(1024,1024));
    for (int i = 0; i < detections.size(); ++i) {
        Detection d = detections.at(i);
        cv::rectangle(img,cv::Point(d.x-d.w/2,d.y-d.h/2),cv::Point(d.x+d.w/2,d.y+d.h/2),cv::Scalar(0,255,0),2);

    }
    cv::imshow("imageres", img);
    cv::waitKey(0);
}


torch::Tensor  Inference::read_image(const std::string& imageName)
{
    cv::Mat img = cv::imread(imageName);
    img = crop_center(img);

    cv::resize(img, img, cv::Size(1024,1024));

    cv::namedWindow("image",cv::WINDOW_AUTOSIZE);
    cv::imshow("image", img);

    //    if (img.channels()==1)
    //        cv::cvtColor(img, img, cv::COLOR_GRAY2RGB);
    //    else
    //        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

    img.convertTo( img, CV_32FC3, 1/255.0 );
    //   cv::imshow("image2", img);
    //cv::waitKey(0);
    torch::Tensor img_tensor = torch::from_blob(img.data, {img.rows, img.cols, 3}, c10::kFloat);
    img_tensor = img_tensor.permute({2, 0, 1});
    img_tensor.unsqueeze_(0);


    // img_tensor = torch::data::transforms::Normalize<>(norm_mean, norm_std)(img_tensor);

    return img_tensor.clone();
}
torch::Tensor  Inference::read_image(const cv::Mat src)
{


    //    if (img.channels()==1)
    //        cv::cvtColor(img, img, cv::COLOR_GRAY2RGB);
    //    else
    //        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
    cv::Mat img;
    src.convertTo( img, CV_32FC3, 1/255.0 );
    //   cv::imshow("image2", img);
    //cv::waitKey(0);
    std::cout << "converted to 32fc3"<<std::endl;


    torch::Tensor img_tensor = torch::from_blob(img.data, {img.rows, img.cols, 3}, c10::kFloat);
    img_tensor = img_tensor.permute({2, 0, 1});
    img_tensor.unsqueeze_(0);

    std::cout << "converted to tensor"<<std::endl;

    // img_tensor = torch::data::transforms::Normalize<>(norm_mean, norm_std)(img_tensor);

    return img_tensor.clone();
}

cv::Mat  Inference::crop_center(const cv::Mat &img)
{
    const int rows = img.rows;
    const int cols = img.cols;

    const int cropSize = std::min(rows,cols);
    const int offsetW = (cols - cropSize) / 2;
    const int offsetH = (rows - cropSize) / 2;
    const cv::Rect roi(offsetW, offsetH, cropSize, cropSize);

    return img(roi);
}
std::string Inference::makeLocationsString(    std::vector<Detection> detections){
    std::string start = "{\"mapValue\": {\"fields\": {\"xCord\": {\"integerValue\": \"";
    std::string mid = "\"},\"yCord\": {\"integerValue\": \"";

    std::string end = "\"}}}}";

    std::stringstream ss;

    for (int i = 0; i < detections.size(); ++i) {
        ss<<start<<(int)detections.at(i).x<<mid<<(int)detections.at(i).y<<end;
        if(i<detections.size()-1) ss<<",";

    }

    return ss.str();
}
