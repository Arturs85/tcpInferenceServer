#ifndef INFERENCE_HPP
#define INFERENCE_HPP



#include <stdio.h>

#include <pthread.h>
#include <string>
#include "tcpserver.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <torch/torch.h>

struct Point {
    double x, y;
};

struct Detection {
    double x,y,w,h,a,c;
    bool operator < (const Detection& other) const
    {
        return (c < other.c);
    }
};
class Inference{
public:
  void sendImage();
  void formatImage();
  cv::Mat crop_center(const cv::Mat &img);
  torch::Tensor read_image(const std::string &imageName);
  void drawBoxes(const std::string &imageName, std::vector<Detection> detections);
  double iou(Point l1, Point r1, Point l2, Point r2);
  std::vector<Detection> infere(int len, const char *data);
  void init();
  Inference();
  torch::Tensor read_image(const cv::Mat src);
  torch::jit::script::Module module;
  bool wasValidImageData = true;
  std::string makeLocationsString(std::vector<Detection> detections);
};

#endif // INFERENCE_HPP
