# First Bot Workspace
Creating my first bot after learning the basics with Andrew; using a new repo to keep things cleaner

## Setup
For simplicity, some commands are in the `Makefile`

Any time you want to run ROS commands you must source the environment with following command; it's a good idea to put this in your `.bashrc`:

```bash
source /opt/ros/jazzy/setup.bash
```

Build the workspace with `colcon`
```bash
# remove the install, build, log dir (not always required)
make clean

# build the workspace
make build
```

Source the setup file
```bash
source install/setup.bash
```

## Launching the Bot

Whenever opening a new terminal, be sure to source the setup file as shown above.

### Launch Gazebo with the default world
```bash
ros2 launch simple_bot launch_gazebo.launch.py
```

### Spawn the robot and launch RVIZ

```bash
ros2 launch simple_bot start.launch.py viz_mode:=true
```

`viz_mode:=true` tells the launch script to also launch the `visualize_robot.launch.py` launch script, which is responsible for
1. Reading the robot model (`robot_description`)
2. Compute link transforms
3. Publishes the robot TF Tree (transformation tree)
4. Launches the RViz tool



### TODO:
1. 
