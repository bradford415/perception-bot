#include <memory>

#include "cv_bridge/cv_bridge.hpp"
#include "opencv2/opencv.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"

class ObjectDetectionNode : public rclcpp::Node {
    public:
        ObjectDetectionNode()
        : Node("object_detection_node") {
            image_sub_ = create_subscription<sensor_msgs::msg::Image>(
                "/camera/image", // the ROS2 topic name to subscribe to
                10, // Queue size - keeps the 10 most recent images (queue will back up image imageCallback is slow)

                // Callback function to be called when a new image is received
                std::bind(&ObjectDetectionNode::imageCallback, this, std::placeholders::_1)
            );
        }

    /////////////////// start here!! understand whats going on here
    private:
        void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg) {
            // Process the received image
                RCLCPP_INFO(
            this->get_logger(),
            "Received image: %u x %u, encoding=%s, data=%zu bytes",
            msg->width,
            msg->height,
            msg->encoding.c_str(),
            msg->data.size());
        }
 
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
};


int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObjectDetectionNode>());
    rclcpp::shutdown();
    return 0;
}