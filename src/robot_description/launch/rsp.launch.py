import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node

import xacro


def generate_launch_description():
    """Every ROS 2 launch file must define this function
    
    Returns:
        a LaunchDescription object which tells ROS "here are the nodes + arguments to launch"
    """

    # Check if we're told to use sim time
    use_sim_time = LaunchConfiguration('use_sim_time')

    # Locate where your robot package is (ws/install/robot_description/share/robot_description) and process the URDF file
    pkg_path = os.path.join(get_package_share_directory('robot_description'))
    xacro_file = os.path.join(pkg_path,'description','robot.urdf.xacro')
    robot_description_config = xacro.process_file(xacro_file)
    
    # toxml converts the urdf to a string structured like xml; ros expects the robot description as a string parameter
    params = {'robot_description': robot_description_config.toxml(), 'use_sim_time': use_sim_time} 
  
   # Create a robot_state_publisher node;
   # robot_state_publisher node does the following:
   #    - Reads your URDF
   #    - Computes transforms (TF tree)
   #    - Publishes /tf
    node_robot_state_publisher = Node(
        package='robot_state_publisher', # prebuilt ROS package that does the core logic of the rsp (not something we created)
        executable='robot_state_publisher',
        output='screen',
        parameters=[params]
    )


    # Must return a LaunchDescription object
    return LaunchDescription([
        # Declare a launch argument that lets us run: ros2 launch robot_description rsp.launch.py use_sim_time:=true
        # use_sim_time = True -> use Gazebo clock, False -> use System clock  
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use sim time if true'),
        node_robot_state_publisher
    ])