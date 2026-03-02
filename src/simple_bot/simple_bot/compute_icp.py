import rclpy
from rclpy.node import Node

from sensor_msgs.msg import LaserScan

from simple_bot.icp import compute_iterative_icp

class ICPNode(Node):

    def __init__(self):
        super().__init__('icp_node')
        
        # subscribe to LIDAR scan topic
        self.subscription = self.create_subscription(
            LaserScan,
            '/scan',
            self.compute_icp_callback,
            10) # queue size; up to 10 point clouds will be buffered, if the queue fills up, the oldest ones will be dropped
        self.subscription  # prevent unused variable warning
        
        self.prev_scan = None

    def compute_icp_call_back(self, curr_scan: LaserScan):
        self.get_logger().info(f"Recieved LIDAR scan with {len(curr_scan.ranges)} points")
        self.get_logger().info("Computing ICP alignment...")

        if self.prev_scan is not None:
            aligned_points = compute_iterative_icp(curr_scan, self.prev_scan)

        # set the previous point cloud to the current one on the first iteration and then
        # used the aligned points on subsequent iterations
        if self.prev_scan is None:
            self.prev_scan = curr_scan
        else:
            self.prev_scan = aligned_points
    

def main(args=None):
    
    
    rclpy.init(args=args)

    icp_processor = ICPNode()

    rclpy.spin(icp_processor)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    icp_processor.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()