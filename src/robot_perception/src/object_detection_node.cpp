#include <memory>
#include <string>
#include <vector>

#include "cv_bridge/cv_bridge.hpp"
#include "opencv2/opencv.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "robot_perception/detection.hpp"
#include "robot_perception/onnx_inference.hpp"
#include "robot_perception/rtdetr_postprocessor.hpp"


class ObjectDetectionNode : public rclcpp::Node {
    public:
        ObjectDetectionNode()
        : Node("object_detection_node")
        {
            image_subscriber_ = create_subscription<sensor_msgs::msg::Image>(
                "/camera/image", // the ROS2 topic name to subscribe to
                10, // Queue size - keeps the 10 most recent images (queue will back up image imageCallback is slow)

                // Callback function to be called when a new image is received
                std::bind(&ObjectDetectionNode::imageCallback, this, std::placeholders::_1)
            );

            // initialize the inferencer as a pointer because we don't know the model_path
            // until after the node is initialized; model_path comes from the config yaml file
            model_path_ = this->declare_parameter<std::string>("model_path", "");
            std::cout << "\nLoading ONNX model from the path: " << model_path_ << std::endl;

            inferencer_ = std::make_unique<ONNXInference>(model_path_);
            std::cout << "\nConstructed ONNXInference instance" << std::endl;
        }

    private:
        void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg) {
            // Process the received image

            cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg, "bgr8");

            // constant reference to the OpenCV image
            const cv::Mat& image = cv_ptr->image;

            RCLCPP_INFO(
            this->get_logger(),
                "Original image dimensions: %d x %d (h, w) channels=%d",
                image.rows,
                image.cols,
                image.channels());

            // preprocess the image; TODO, should make this a separate component
            cv::Mat resized_img;
            cv::resize(image, resized_img, cv::Size(640, 640));

            cv::Mat rgb_img;
            cv::cvtColor(resized_img, rgb_img, cv::COLOR_BGR2RGB);

            // normalize from 0-255 to 0-1; 32-bit floating point 3-channel
            cv::Mat normalized_img;
            rgb_img.convertTo(normalized_img, CV_32FC3, 1.0 / 255.0);

            // flatten the image to a vector of floats; ORT requires this
            // reserve prealocates the vector instead of continuously reallocating the vector
            std::vector<float> input_data;
            input_data.reserve(normalized_img.rows * normalized_img.cols * normalized_img.channels());

            // assign the pixel values to the flattened vector and conceptually 
            // convert (h, w, c) to (c, h, w)
            for (int c = 0; c < normalized_img.channels(); ++c) {
                for (int y = 0; y < normalized_img.rows; ++y) {
                    for (int x = 0; x < normalized_img.cols; ++x) {
                        // opencv images are stored as (h, w, c)
                        // so this line is saying at coordinate (y, x) the pixel value is [r, g, b]
                        // so we grab this pixel but the at each channel [c]. 
                        input_data.push_back(normalized_img.at<cv::Vec3f>(y, x)[c]);
                    }
                }
            }

            RCLCPP_INFO(this->get_logger(), "\n Finished preprocessing image");

            // inference the image
            RCLCPP_INFO(this->get_logger(), "\nRunning inference on the preprocessed image");
            ONNXOutputs onnx_outputs = inferencer_->inference_image(input_data);

            // postprocess the predictions
            postprocessor_.process_outputs(onnx_outputs, image.cols, image.rows);

            //RCLCPP_INFO(this->get_logger(), "\nDetected %zu objects", detections.size());
        }
 
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_subscriber_;
        std::unique_ptr<ONNXInference> inferencer_;
        std::string model_path_;
        RTDETRPostprocessor postprocessor_;
};


int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObjectDetectionNode>());
    rclcpp::shutdown();
    return 0;
}
