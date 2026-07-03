#include <algorithm>
#include <cmath>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <iostream>

#include "robot_perception/detection.hpp"
#include "robot_perception/rtdetr_postprocessor.hpp"

// struct to store the sigmoided logits and their topk index in the flat
// num_queries*num_classes vector
struct ScoreIndex {
    float score;
    int index;
};

// static here means this function is only visibile inside this c++ file
static float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

// NOTE: do not need to write the constructor in this file because it's set as deafult
std::vector<Detection> RTDETRPostprocessor::process_outputs(
    ONNXOutputs onnx_outputs,
    int orig_image_width, 
    int orig_image_height) {

    const auto& logits = onnx_outputs.pred_logits;
    const auto& boxes = onnx_outputs.pred_boxes;
    const auto& logits_shape = onnx_outputs.logits_shape;
    const auto& boxes_shape = onnx_outputs.boxes_shape;
    
    // This function assumes a batch size of 1
    assert(logits_shape[0] == 1);
    assert(boxes_shape[0] == 1);

    assert(boxes_shape[2] == 4);
    assert(logits_shape[1] == boxes_shape[1]);

    int num_classes = logits_shape[2];
    int num_queries = logits_shape[1];

    std::vector<ScoreIndex> scored;
    scored.reserve(num_queries*num_classes);

    // apply sigmoid to all logits; keep track of value and flat_index
    for (int q = 0; q < num_queries; ++q) {
        for (int c = 0; c < num_classes; ++c) {
            const int flat_index = q * num_classes + c;

            // bc we defined scored to be a ScoreIndex type,brace initialization will automatically 
            // create a temporary object annd know we're initializing a ScoreIndex
            scored.push_back({
                sigmoid(logits[flat_index]), 
                flat_index
            });
        }
    }

    // Sort the logits by topk; partial sort is O(nlog(k)) where k = top_k;
    // this is very useful for sorting topk because it's more efficient than O(nlogn)
    // partial sort only sorts the first 0-k elements and the latter half is unspecified
    // inclusive (start) - exclusive (middle/k)
    std::partial_sort(
        scored.begin(), 
        scored.begin() + this->num_selected_queries, 
        scored.end(),
        // in-line comparator using a lambda function since we're using ScoreIndex as vector values;
        // this says the 1st element should be greater than the 2nd element
        // -> sort in descending; we're also telling it to use the score field of the ScoreIndex struct
        [](const ScoreIndex& a, const ScoreIndex& b) {
            return a.score > b.score;
        }
    );

    // for (auto& val : scored) {
    //     std::cout << val << std::endl;
    // }

    
}

