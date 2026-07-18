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


// TODO: find a better place for this
// rt-detr class map
std::vector<std::string> class_names_{
"person",
"bicycle",
"car",
"motorcycle",
"airplane",
"bus",
"train",
"truck",
"boat",
"traffic light",
"fire hydrant",
"stop sign",
"parking meter",
"bench",
"bird",
"cat",
"dog",
"horse",
"sheep",
"cow",
"elephant",
"bear",
"zebra",
"giraffe",
"backpack",
"umbrella",
"handbag",
"tie",
"suitcase",
"frisbee",
"skis",
"snowboard",
"sports bat",
"kite",
"baseball bat",
"baseball glove",
"skateboard",
"surfboard",
"tennis rack",
"bottle",
"wine glass",
"cup",
"fork",
"knife",
"spoon",
"bowl",
"banana",
"apple",
"sandwich",
"orange",
"broccoli",
"carrot",
"hot dog",
"pizza",
"donut",
"cake",
"chair",
"couch",
"potted plant",
"bed",
"dining table",
"toilet",
"tv",
"laptop",
"mouse",
"remote",
"keyboard",
"cell phone",
"microwave",
"oven",
"toaster",
"sink",
"refrigerator",
"book",
"clock",
"vase",
"scissors",
"teddy bear",
"hair drier",
"toothbrush",
};

class ObjectDetectionNode : public rclcpp::Node {
    public:
        ObjectDetectionNode()
        : Node("object_detection_node")
        {
            image_subscriber_ = create_subscription<sensor_msgs::msg::Image>(
                "/camera/image", // the ROS2 topic name to subscribe to
                1, // Queue size - keeps the 10 most recent images (queue will back up image imageCallback is slow)
                   // NOTE: changeing from 10 to 1 so detection node does not have to process all frames

                // Callback function to be called when a new image is received
                std::bind(&ObjectDetectionNode::imageCallback, this, std::placeholders::_1)
            );

            // mostly for visualization and debugging purposes
            annotated_image_publisher_ = create_publisher<sensor_msgs::msg::Image>(
                "/annotated_image", 
                rclcpp::SensorDataQoS().keep_last(1));

            // initialize the inferencer as a pointer because we don't know the model_path
            // until after the node is initialized; model_path comes from the config yaml file
            model_path_ = this->declare_parameter<std::string>("model_path", "");
            RCLCPP_INFO(this->get_logger(), "\nLoading ONNX model from the path: %s", model_path_.c_str());

            confidence_threhsold_ = this->declare_parameter<double>("confidence_threhsold", 0.5);
            RCLCPP_INFO(this->get_logger(), "\nUsing a confidence threshold of %f", confidence_threhsold_);

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
            RCLCPP_INFO(this->get_logger(), "\nFinished inferencing");

            // postprocess the predictions
            std::vector<Detection> detections;
            detections = postprocessor_.process_outputs(onnx_outputs, image.cols, image.rows, confidence_threhsold_);
            RCLCPP_INFO(this->get_logger(), "\n\nFinished postprocessing the images with %zu detections:", detections.size());
            for (const auto& detection : detections) {
                RCLCPP_INFO(
                    this->get_logger(),
                    "Class: %s, Confidence: %.2f, Box (XYWH): [%.2f, %.2f, %.2f, %.2f]",
                    class_names_[detection.class_id].c_str(),
                    detection.confidence,
                    detection.box.x,
                    detection.box.y,
                    detection.box.width,
                    detection.box.height
                );
            }

            // TODO: probably should make this its own method
            // create an annotated image w/ the predicted bounding boxes and class labels for visualization
            RCLCPP_INFO(this->get_logger(), "Annotating the image with the detections");
            cv::Mat annotated_image = image.clone();
            for (const auto& detection: detections) {
        
                // convert boxes float->ints and clip the box to the image dimensions
                cv::Rect box = detection.box;
                box &= cv::Rect(0, 0, annotated_image.cols, annotated_image.rows);

                if (box.empty()) {
                    continue;
                }

                // opencv rectangle docs:
                //  https://www.opencv.org.cn/opencvdoc/2.3.2/html/modules/core/doc/drawing_functions.html?highlight=ellipse#void%20rectangle(Mat&%20img,%20Rect%20r,%20const%20Scalar&%20color,%20int%20thickness,%20int%20lineType,%20int%20shift)
                cv::rectangle(
                    annotated_image, // img to draw on
                    box, // the box to draw; 
                    cv::Scalar(0, 255, 0), // box color
                    2 // thickness of the rectangle in pixels
                );

                // create a string of the class pred + conf
                const std::string label = 
                    class_names_[detection.class_id] + " " + 
                    cv::format("%.2f", detection.confidence);

                // baseline is the additional space needed below the spaceline;
                // baseline is the line most letters sit on (e.g., g, p & y could go below the baseline)
                int baseline = 0; 
                const cv::Size text_size = cv::getTextSize(
                    label,
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    1,
                    &baseline);                

                // bottom left point of the baseline to start the text
                const cv::Point text_origin(
                    box.x,
                    std::max(box.y - 5, text_size.height));

                // draw the text on the image
                cv::putText(
                    annotated_image,
                    label,
                    text_origin,
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    cv::Scalar(0, 255, 0),
                    1,
                    cv::LINE_AA);
            }
            //RCLCPP_INFO(this->get_logger(), "\nDetected %zu objects", detections.size());

            auto output_msg = cv_bridge::CvImage(
                msg->header, // copies original timestamp & frame id to keep the annotated img synchronized w/ the camera img
                sensor_msgs::image_encodings::BGR8, 
                annotated_image)
                .toImageMsg();

            // publish the annotated image
            RCLCPP_INFO(this->get_logger(), "Publishing the annotated image");
            annotated_image_publisher_->publish(*output_msg);
        }
 
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_subscriber_;
        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr annotated_image_publisher_;
        std::unique_ptr<ONNXInference> inferencer_;
        std::string model_path_;
        double confidence_threhsold_;
        RTDETRPostprocessor postprocessor_;
};


int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObjectDetectionNode>());
    rclcpp::shutdown();
    return 0;
}
