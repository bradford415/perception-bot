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

### Reading lidar points with the `read_lidar` Node

```bash
ros2 run test_bot_python read_lidar
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

## Using RViz

### Visualize the lidar points
RViz can show the lidar points for the objects the beams hit off (it does not show all the beams since it does not measure empty space).

With RViz open:
1. In the left panel click Add > By Topic > /scan Laser Scan > OK
2. Laser Scan should now be added on the left panel and if the check box is chcked, you should see red points where the lidar hits
3. You can place blocks in gazebo and move the robot around to change where the lidar points hit (you might also notice the lidar hitting the wheels of the robot)



### TODO:
1. 
