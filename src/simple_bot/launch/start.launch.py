import os
from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration, Command
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.conditions import IfCondition

from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Get Launch Directories
    launch_dir = os.path.join(get_package_share_directory("simple_bot"), "launch")

    assert os.path.exists(launch_dir)

    # Get default values
    default_use_sim_time = "True"
    default_pose = {"x": "0",
                    "y": "0",
                    "z": "0.5",
                    "R": "0",
                    "P": "0",
                    "Y": "0"}
    default_robot_name="robot"
    default_xacro_file = os.path.join(get_package_share_directory("simple_bot"), 
                                     "description", "robot.xacro")
    default_rviz_config_file = os.path.join(get_package_share_directory("simple_bot"), "description", "rviz.rviz")
    default_viz_mode = "False"

    # Define Launch Configuration Variables
    robot_name = LaunchConfiguration("robot_name")
    xacro_file = LaunchConfiguration("xacro_file")
    viz_mode = LaunchConfiguration("viz_mode")
    rviz_config_file = LaunchConfiguration("rviz_config_file")
    use_sim_time = LaunchConfiguration("use_sim_time")
    pose = {"x": LaunchConfiguration("x_pose"),
            "y": LaunchConfiguration("y_pose"),
            "z": LaunchConfiguration("z_pose"),
            "R": LaunchConfiguration("roll"),
            "P": LaunchConfiguration("pitch"),
            "Y": LaunchConfiguration("yaw")}
    

    # Declare Launch Argument w/ Default Values
    robot_name_args = DeclareLaunchArgument(
        "robot_name",
        default_value=default_robot_name,
        description="Name of robot"
    )
    xacro_file_args = DeclareLaunchArgument(
        "xacro_file",
        default_value=default_xacro_file,
        description="XML file of robot"
    )
    viz_mode_args = DeclareLaunchArgument(
        name="viz_mode",
        default_value=default_viz_mode,
        description="Enable visualization application"
    )
    rviz_config_args = DeclareLaunchArgument(
        name='rviz_config_file', 
        default_value=default_rviz_config_file,
        description='Full path to the RVIZ config file'
    )
    use_sim_time_args = DeclareLaunchArgument(
        "use_sim_time",
        default_value=default_use_sim_time,
        description="Enable simulation time"
    )
    x_pose_args = DeclareLaunchArgument(
        "x_pose",
        default_value=default_pose["x"],
        description="X Pose Spawn Position"
    )
    y_pose_args = DeclareLaunchArgument(
        "y_pose",
        default_value=default_pose["y"],
        description="Y Pose Spawn Position"
    )
    z_pose_args = DeclareLaunchArgument(
        "z_pose",
        default_value=default_pose["z"],
        description="Z Pose Spawn Position"
    )
    roll_args = DeclareLaunchArgument(
        "roll",
        default_value=default_pose["R"],
        description="Roll Spawn Position"
    )
    pitch_args = DeclareLaunchArgument(
        "pitch",
        default_value=default_pose["P"],
        description="Pitch Spawn Position"
    )
    yaw_args = DeclareLaunchArgument(
        "yaw",
        default_value=default_pose["Y"],
        description="Yaw Spawn Position"
    )


    

    # Specify Actions
    spawn_robot_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(launch_dir, "spawn_robot.launch.py")),
        launch_arguments={
            'robot_name': robot_name, "xacro_file": xacro_file,
            'x_pose': pose['x'], 'y_pose': pose['y'], 'z_pose': pose['z'],
            'roll': pose['R'], 'pitch': pose['P'], 'yaw': pose['Y']
        }.items()
    )

    visualize_robot_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(launch_dir, "visualize_robot.launch.py")),
        condition=IfCondition(viz_mode),
        launch_arguments={
            "use_sim_time": use_sim_time, "xacro_file": xacro_file,
            "joint_gui_mode": "False", "rviz_config_file": rviz_config_file
        }.items()
    ) # TODO: Remove visualization launch and just use rviz node

    # Add action
    ld = LaunchDescription()
    ld.add_action(xacro_file_args)
    ld.add_action(viz_mode_args)
    ld.add_action(rviz_config_args)
    ld.add_action(robot_name_args)
    ld.add_action(x_pose_args)
    ld.add_action(y_pose_args)
    ld.add_action(z_pose_args)
    ld.add_action(roll_args)
    ld.add_action(pitch_args)
    ld.add_action(yaw_args)
    ld.add_action(use_sim_time_args)
    ld.add_action(spawn_robot_launch)
    ld.add_action(visualize_robot_launch)

    """
    TODO:
    - Create start launch file
        - Contains launch gazebo - Done
        - Contains spawn robot - Done -> Rename this file to spawn gazebo
        - Visualizer - Done
        - Incorporate all into a start launch file
    - Look into ros2 differential drive robot ros2 control - Done for now, utilized gazebo diff drive
        - Ensure there is nothing weird with ros2 control
    """
    return ld