#include <memory>

#include "cv_bridge/cv_bridge.hpp"
#include "opencv2/opencv.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "robot_perception/onnx_inference.hpp"

class ObjectDetectionNode : public rclcpp::Node {
    public:
        ObjectDetectionNode()
        : Node("object_detection_node")
        {
            image_sub_ = create_subscription<sensor_msgs::msg::Image>(
                "/camera/image", // the ROS2 topic name to subscribe to
                10, // Queue size - keeps the 10 most recent images (queue will back up image imageCallback is slow)

                // Callback function to be called when a new image is received
                std::bind(&ObjectDetectionNode::imageCallback, this, std::placeholders::_1)
            );

            // initialize the inferencer as a pointer because we don't know the model_path
            // until after the node is initialized; model_path comes from the config yaml file
            model_path = this->get_parameter("model_path").as_string();
            inferencer_ = std::make_unique<OnnxInference>(model_path);
        }


    private:
        void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg) {
            // Process the received image


            cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg, "bgr8");

            // constant reference to the OpenCV image
            const cv::Mat& image = cv_ptr->image;

            RCLCPP_INFO(
            this->get_logger(),
                "Image: %d x %d (h, w) channels=%d",
                image.rows,
                image.width,
                image.channels());

            // preprocess the image; TODO, should make this a separate component
            cv::Mat resized_img;
            cv::resize(image, resized_img, cv::Size(640, 640));

            cv::Mat rgb_img;
            cv::cvtColor(resized_img, rgb_img, cv::COLOR_BGR2RGB);

            // normalize from 0-255 to 0-1; 32-bit floating point 3-channel
            cv::Mat normalized_img;
            rgb_img.convertTo(normalized_img, CV_32FC3, 1.0 / 255.0);

            inferencer_->inference_image(normalized_img);

            // TODO change model path and create detection output struct

            

        }
 
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
        std::unique_ptr<OnnxInference> inferencer_;
};


int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObjectDetectionNode>());
    rclcpp::shutdown();
    return 0;
}