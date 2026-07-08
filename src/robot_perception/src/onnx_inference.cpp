#include <onnxruntime_cxx_api.h>
#include <vector>
#include <iostream>

#include "robot_perception/onnx_inference.hpp"
#include "robot_perception/onnx_outputs.hpp"
#include "robot_perception/detection.hpp"

ONNXInference::ONNXInference(const std::string& model_path)
    : model_path_(model_path),
        env_(ORT_LOGGING_LEVEL_WARNING, "onnx_inference"),
        session_options_(),
        session_(nullptr) {
        session_options_.SetIntraOpNumThreads(1);
        session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED); // enables basic and extended graph optimizations: https://onnxruntime.ai/docs/performance/model-optimizations/graph-optimizations.html#extended-graph-optimizations 
        session_ = Ort::Session(env_, model_path_.c_str(), session_options_);

        // Allocator used for querying the model input/output info; outputs logits and bboxes so we'll need two outputs
        Ort::AllocatorWithDefaultOptions allocator;
        auto input_name_ptr = session_.GetInputNameAllocated(0, allocator);
        auto output_logits_ptr = session_.GetOutputNameAllocated(0, allocator);
        auto output_boxes_ptr = session_.GetOutputNameAllocated(1, allocator);

        input_name_ = input_name_ptr.get();
        output_logits_ = output_logits_ptr.get();
        output_boxes_ = output_boxes_ptr.get();
    }

ONNXOutputs ONNXInference::inference_image(const std::vector<float>& input_data) {

    // extract the wrapped input and output names
    const char* input_names[] = {input_name_.c_str()};
    const char* output_names[] = {output_logits_.c_str(), output_boxes_.c_str()};

    // input shape for rt-detr (1, 3, 640, 640)
    std::vector<int64_t> input_shape = {1, 3, 640, 640};

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_,
        const_cast<float*>(input_data.data()), // function expects a float* not a const float* so const_cast removes the const
        input_data.size(),
        input_shape.data(),
        input_shape.size());

    
    std::cout << "\nRunning ONNXinference on the preprocessed image" << std::endl;
    // run inference
    auto output_tensors = session_.Run(
        Ort::RunOptions{nullptr},
        input_names,
        &input_tensor,
        1, // input count - we're passing 1 input tensor
        output_names,
        2 // output count - we're expecting 2 output tensor (logits and boxes)
    );
    std::cout << "\nFinished running inference" << std::endl;

    // extract the predicted logits (b, num_queries, num_classes) and predicted bboxes (b, num_queries, 4)
    // this returns a pointer to the first float; e.g., pred_logits[1] = the 2nd logit
    // so in total there will be  1*80
    float* pred_logits = output_tensors[0].GetTensorMutableData<float>();
    float* pred_boxes = output_tensors[1].GetTensorMutableData<float>();

    auto logits_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
    auto boxes_shape = output_tensors[1].GetTensorTypeAndShapeInfo().GetShape();
    std::cout << "\nLogits shape: ";
    for (const auto& dim : logits_shape) {
        std::cout << dim << " ";
    }
    std::cout << "\nBoxes shape: ";
    for (const auto& dim : boxes_shape) {
        std::cout << dim << " ";
    }
    std::cout << std::endl;

    // convert the float* to vectors
    //  1. get the number of elements
    //  2. use a vecotr range constructor to copy the elements
    std::size_t logits_count = 1; // 1 is neutral starting value for computing the product
    for (auto dim : logits_shape){
        logits_count *= static_cast<std::size_t>(dim);
    }
    std::size_t boxes_count = 1;
    for (auto dim : boxes_shape) {
        boxes_count *= static_cast<std::size_t>(dim);
    }
    std::vector<float> logits_vec(pred_logits, pred_logits + logits_count);
    std::vector<float> boxes_vec(pred_boxes, pred_boxes + boxes_count);

    // wrap the raw onnx outputs to return them to the object_detection node
    ONNXOutputs onnx_outputs{
        logits_vec,
        boxes_vec,
        logits_shape,
        boxes_shape
    };

    return onnx_outputs;
}