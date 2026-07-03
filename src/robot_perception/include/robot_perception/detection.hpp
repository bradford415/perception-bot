#pragma once

#include <opencv2/core.hpp>

struct Detection {

    int class_id;
    float confidence;

    // data structure which stores XYWH (coco format)
    // box.x, box.y, box.width, box.height
    cv::Rect2f box;
};