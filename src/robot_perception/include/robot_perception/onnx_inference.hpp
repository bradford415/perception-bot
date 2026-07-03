#pragma once

#include <string>
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <vector>

#include "robot_perception/detection.hpp"
#include "robot_perception/onnx_outputs.hpp"

class ONNXInference {
public:
    explicit ONNXInference(const std::string& model_path);
    ONNXOutputs inference_image(const std::vector<float>& input_data);
private:
    std::string model_path_;
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    Ort::Session session_;
    std::string input_name_;
    std::string output_logits_;
    std::string output_boxes_;

    // tells the ONNX Runtime to use the CPU for memory allocation
    Ort::MemoryInfo memory_info_ = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
};
