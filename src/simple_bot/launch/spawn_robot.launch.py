import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    # Get default values
    default_xacro_file = os.path.join(
        get_package_share_directory("simple_bot"), "description", "robot.xacro"
    )
    default_robot_name = "robot"
    default_pose = {"x": "0", "y": "0", "z": "0.5", "R": "0", "P": "0", "Y": "0"}
    default_gz_params_file = os.path.join(
        get_package_share_directory("simple_bot"), "config", "gz_bridge.yaml"
    )
    print(f"Default GZ Bridge Params File Path: {default_gz_params_file}")

    # Define Launch Configuration Variables
    xacro_file = LaunchConfiguration("xacro_file")
    robot_name = LaunchConfiguration("robot_name")
    pose = {
        "x": LaunchConfiguration("x_pose"),
        "y": LaunchConfiguration("y_pose"),
        "z": LaunchConfiguration("z_pose"),
        "R": LaunchConfiguration("roll"),
        "P": LaunchConfiguration("pitch"),
        "Y": LaunchConfiguration("yaw"),
    }
    gz_params_file = LaunchConfiguration("gz_params_file")

    robot_description_config = Command(["xacro", " ", xacro_file])

    # Declare Launch Argument w/ Default Values
    xacro_file_args = DeclareLaunchArgument(
        "xacro_file",
        default_value=default_xacro_file,
        description="XML file description of robot",
    )
    robot_name_args = DeclareLaunchArgument(
        "robot_name",
        default_value=default_robot_name,
        description="Default name for robot",
    )
    x_pose_args = DeclareLaunchArgument(
        "x_pose", default_value=default_pose["x"], description="X Pose Spawn Position"
    )
    y_pose_args = DeclareLaunchArgument(
        "y_pose", default_value=default_pose["y"], description="Y Pose Spawn Position"
    )
    z_pose_args = DeclareLaunchArgument(
        "z_pose", default_value=default_pose["z"], description="Z Pose Spawn Position"
    )
    roll_args = DeclareLaunchArgument(
        "roll", default_value=default_pose["R"], description="Roll Spawn Position"
    )
    pitch_args = DeclareLaunchArgument(
        "pitch", default_value=default_pose["P"], description="Pitch Spawn Position"
    )
    yaw_args = DeclareLaunchArgument(
        "yaw", default_value=default_pose["Y"], description="Yaw Spawn Position"
    )
    gz_params_file_args = DeclareLaunchArgument(
        "gz_params_file",
        default_value=default_gz_params_file,
        description="GZ Bridge Parameter File",
    )

    # Specify Actions
    start_gazebo_ros_spawner_cmd = Node(
        package="ros_gz_sim",
        executable="create",
        name="gazebo_ros_spawner",
        arguments=[
            "-name",
            robot_name,
            "-string",
            robot_description_config,
            "-x",
            pose["x"],
            "-y",
            pose["y"],
            "-z",
            pose["z"],
            "-R",
            pose["R"],
            "-P",
            pose["P"],
            "-Y",
            pose["Y"],
        ],
        output="screen",
    )

    # ROS2 Bridge (New Gazebo - Communication w/ ros is done through topic bridges done by ros_gazebo pakg)
    # TODO: Check how gz bridge works w/o using robot state publisher / joint state publisher
    # Source: Search gazebo jazzy and look @ reddit
    gz_bridge_node = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        parameters=[{"config_file": gz_params_file}],
        # arguments=[
        #    '--ros-args -p config_file:=', gz_params_file
        # ]
    )

    # Add action
    ld = LaunchDescription()

    ld.add_action(xacro_file_args)
    ld.add_action(robot_name_args)
    ld.add_action(x_pose_args)
    ld.add_action(y_pose_args)
    ld.add_action(z_pose_args)
    ld.add_action(roll_args)
    ld.add_action(pitch_args)
    ld.add_action(yaw_args)
    ld.add_action(gz_params_file_args)
    ld.add_action(start_gazebo_ros_spawner_cmd)
    ld.add_action(gz_bridge_node)

    return ld
