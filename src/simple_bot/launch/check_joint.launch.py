#!/usr/bin/env python3

import os

import xacro
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.descriptions import ParameterValue


def generate_launch_description():
    """
    Robot description launcher consists of:
    - Joint Publisher: Update the joints of the robot
    - Robot State Publisher: Gets the 3D pose of the robot links using a kinematic tree model of robot
    - Rviz: Visualizer to visualize joint + linkages, etc.
    """

    # Get default parameters
    default_xacro_file = os.path.join(
        get_package_share_directory("simple_bot"), "description", "robot.xacro"
    )
    default_rviz_config_file = os.path.join(
        get_package_share_directory("simple_bot"), "description", "rviz.rviz"
    )

    # Assertion of default parameters existing | TODO: See if we care if default path exists
    assert os.path.exists(default_xacro_file), "The xacro file does not exist: " + str(
        default_xacro_file
    )
    assert os.path.exists(
        default_rviz_config_file
    ), "The rviz config file does not exist: " + str(default_rviz_config_file)

    # Define Launch Configuration Variables
    xacro_file = LaunchConfiguration("xacro_file")
    rviz_config_file = LaunchConfiguration("rviz_config_file")
    joint_gui_mode = LaunchConfiguration("joint_gui_mode")
    use_sim_time = LaunchConfiguration("use_sim_time")

    # Set configuration to be evaluated at runtime
    robot_description_config = Command(
        ["xacro", " ", xacro_file]
    )  # TODO: Might potentiailly need to specify simulation mode

    # Declare launch arguments w/ default values
    xacro_file_args = DeclareLaunchArgument(
        "xacro_file",
        default_value=default_xacro_file,
        description="XML File to describe robot",
    )
    rviz_config_args = DeclareLaunchArgument(
        name="rviz_config_file",
        default_value=default_rviz_config_file,
        description="Full path to the RVIZ config file",
    )
    joint_gui_mode_args = DeclareLaunchArgument(
        name="joint_gui_mode",
        default_value="True",
        description="Enable joint state GUI",
    )
    use_sim_time_args = DeclareLaunchArgument(
        name="use_sim_time", default_value="False", description="Uses sim time or not"
    )

    # Specify Actions (e.g nodes, commands, etc.)
    """
    ROS1 joint state publisher reads from param server to find the non-fixed joint. 
    I am not sure if ros2 does the same, maybe just subscribes to topic robot_description instead?
    Come back on this when have movable joints
    """
    joint_state_publisher_node = Node(
        condition=UnlessCondition(joint_gui_mode),
        package="joint_state_publisher",
        executable="joint_state_publisher",
        name="joint_state_publisher_node",
        output="screen",
    )

    joint_state_publisher_gui_node = Node(
        condition=IfCondition(joint_gui_mode),
        package="joint_state_publisher_gui",
        executable="joint_state_publisher_gui",
        name="joint_state_publisher_gui_node",
        output="screen",
    )

    params = {
        "robot_description": ParameterValue(robot_description_config, value_type=str),
        "use_sim_time": ParameterValue(use_sim_time, value_type=bool),
    }  # Double check if use_sim_time should be string or bool
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher_node",
        output="screen",
        # parameters=[{"robot_description" : robot_description}]
        parameters=[params],  # Note difference in formatting for params and args
        # parameters=[{"robot_description": LaunchConfiguration("robot_description").perform()}]
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2_node",
        namespace="simple_bot_description",
        output="screen",
        arguments=["-d", rviz_config_file],  # Gets the launch argument rviz_config
    )

    ld = LaunchDescription()

    # Declare Launch options
    ld.add_action(rviz_config_args)
    ld.add_action(xacro_file_args)
    ld.add_action(joint_gui_mode_args)
    ld.add_action(use_sim_time_args)
    # ld.add_action(robot_description_args)

    # Add any actions
    ld.add_action(joint_state_publisher_node)
    ld.add_action(joint_state_publisher_gui_node)
    ld.add_action(robot_state_publisher_node)
    ld.add_action(rviz_node)

    return ld
