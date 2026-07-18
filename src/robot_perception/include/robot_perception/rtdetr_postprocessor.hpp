#pragma once

#include <vector>

#include "robot_perception/detection.hpp"
#include "robot_perception/onnx_outputs.hpp"

class RTDETRPostprocessor {
public:
    RTDETRPostprocessor() = default;
    std::vector<Detection> process_outputs(
        ONNXOutputs onnx_outputs,
        int orig_image_width,
        int orig_image_height,
        double confidence_threshold);
private:
    // static -> there is one shared value for the class, not one copy per object 
    // constexpr -> values is known at compile time and cannot change
    static constexpr int num_selected_queries = 300;

};
