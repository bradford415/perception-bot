#include <onnxruntime_cxx_api.h>
#include <vector>
#include <iostream>

#include "robot_perception/onnx_inference.hpp"

ONNXInference::ONNXInference(const std::string& model_path)
    : model_path_(model_path),
        env_(ORT_LOGGING_LEVEL_WARNING, "onnx_inference"),
        session_options_(),
        session_() {
        session_options_.SetIntraOpNumThreads(1);
        session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED); // enables basic and extended graph optimizations: https://onnxruntime.ai/docs/performance/model-optimizations/graph-optimizations.html#extended-graph-optimizations 
        session_ = Ort::Session(env_, model_path_, session_options_);

        // Allocator used for querying the model input/output info
        Ort::AllocatorWithDefaultOptions allocator;
        input_name_ = session_.GetInputNameAllocated(0, allocator);
        output_name_ = session_.GetOutputNameAllocated(0, allocator);
    }

std::vector<Detection> ONNXInference::inference_image(const cv::Mat& image) {

    // extract the wrapped input and output names
    const char* input_names[] = {input_name_.get()};
    const char* output_names[] = {output_name_.get()};

    // input shape for rt-detr (1, 3, 640, 640)
    std::vector<int64_t> input_shape = {1, 3, 640, 640};


    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        input_data.data(),
        input_data.size(),
        input_shape.data(),
        input_shape.size());

    /// START HERE, do inference and preprocess in ros
    // run inference
    auto output_tensors = session_.Run(
        Ort::RunOptions{nullptr},
        input_names,
        &input_tensor,
        1, // input count - we're passing 1 input tensor
        output_names,
        1 // output count - we're expecting 1 output tensor
    );

    // read the output 
    float* output_data = output_tensors[0].GetTensorMutableData<float>();

    // extract the output tensor shape
    auto output_shape_info  = output_tensors[0].GetTensorTypeAndShapeInfo();
    std::vector<int64_t> output_shape = output_shape_info.GetShape();


}