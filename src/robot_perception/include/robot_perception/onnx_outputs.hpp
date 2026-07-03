#pragma once

#include <vector>

struct ONNXOutputs {

    std::vector<float> pred_logits;
    std::vector<float> pred_boxes;
    std::vector<int64_t> logits_shape;
    std::vector<int64_t> boxes_shape;
};
