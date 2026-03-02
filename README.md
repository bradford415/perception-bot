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


