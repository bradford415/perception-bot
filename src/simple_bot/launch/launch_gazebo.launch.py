import os


from launch import LaunchDescription
from launch_ros.descriptions import ParameterValue
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from ament_index_python.packages import get_package_share_directory



def generate_launch_description():
    # Get default paths
    default_world_file = os.path.join(get_package_share_directory("simple_bot"), "description", "empty_world.sdf")
    ros_gz_sim = get_package_share_directory("ros_gz_sim")

    # Assertion of default parameters existing
    assert os.path.exists(default_world_file)
    assert os.path.exists(os.path.join(ros_gz_sim, "launch", "gz_sim.launch.py"))

    # Define launch configuration variable
    world_file = LaunchConfiguration("world_file")

    # Declare launch argument w/ default values
    world_args = DeclareLaunchArgument(
        "world_file",
        default_value= 'empty.sdf', #default_world_file,
        #default_value=default_world_file,
        description="File to launch gazebo world"
    )

    # Specify Actions
    gzserver_params = {'gz_args': ["-r -s -v2 ", 
                                   ParameterValue(world_file, value_type=str)], 
                                   'on_exit_shutdown': 'true'} # TODO: Use for organization later
    gzserver_cmd = IncludeLaunchDescription(
        launch_description_source=PythonLaunchDescriptionSource(
            os.path.join(ros_gz_sim, "launch", "gz_sim.launch.py")),
        launch_arguments={'gz_args': ["-r -s -v2 ", world_file], 'on_exit_shutdown': 'true'}.items()
    )

    gzclient_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ros_gz_sim, "launch", "gz_sim.launch.py")
        ),
        launch_arguments={'gz_args': '-g -v2 '}.items()
    )

    # Add Actions
    
    # Include robot state publisher launch file. Forces sim time to be enabled
    ld = LaunchDescription()
    ld.add_action(world_args)
    ld.add_action(gzserver_cmd)
    ld.add_action(gzclient_cmd)
    return ld