#pragma once

#include <string>
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <vector>


class ONNXInference {
public:
    explicit ONNXInference(const std::string& model_path);
    std::vector<Detection> inference_image(const cv::Mat& image);
private:
    std::string model_path_;
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    Ort::Session session_;
    std::string input_name_;
    std::string output_name_;

    // tells the ONNX Runtime to use the CPU for memory allocation
    Ort::MemoryInfo memory_info_ = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
};