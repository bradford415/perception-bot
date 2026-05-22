import os

from ament_index_python.packages import get_package_share_directory


from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node



def generate_launch_description():
    """This file essentially runs the 3 commands from the command line
    that you would have to run to run the rsp, gz sim, and spawn the robot
    
    This file:
        1. Launches the robot state publisher to publish the robot description to a topic
        2. Launches the Gazebo simulator with an empty world
        3. Spawns the robot into the Gazebo simulator by reading the robot description topic
    
    Usage: ros2 launch robot_description launch_sim.launch.py
    """
    


    # Include the robot_state_publisher launch file, provided by our own package. Force sim time to be enabled
    # !!! MAKE SURE YOU SET THE PACKAGE NAME CORRECTLY !!!

    package_name='robot_description' # name of our package

    rsp = IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(
                    get_package_share_directory(package_name),'launch','rsp.launch.py'
                )]), launch_arguments={'use_sim_time': 'true'}.items()
    )

    world_file = os.path.join(get_package_share_directory(package_name), 'worlds', 'empty.world')

    # Include the Gazebo launch file, provided by the gazebo_ros package
    gazebo = IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(
                    get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')]),
                launch_arguments={"gz_args": world_file}.items()
             )

    # Run the spawner node from the gazebo_ros package. The entity name doesn't really matter if you only have a single robot.
    spawn_entity = Node(package='ros_gz_sim', executable='create',
                        arguments=['-topic', 'robot_description',
                                   '-name', 'my_bot',
                                   '-x', '0', '-y', '0', '-z', '0.1'],
                        output='screen')



    # Launch them all!
    return LaunchDescription([
        rsp,
        gazebo,
        # give time for the rsp to load before spawning the bot; without this the bot was not spawning
        TimerAction(
            period=3.0,
            actions=[spawn_entity]
        ),
    ])