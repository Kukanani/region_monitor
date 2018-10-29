This ROS package monitors a cubic region of space and exposes a service that will tell you whether or not there are point clouds detected in that space.

It can be used to monitor a region for occupancy.

## Installation

This package should go straight into your ROS workspace  `src` as any other package
would.

## Usage

`rosrun region_monitor region_monitor` should start the node.

The region to monitor can be set by using the `region_monitor/set_region` topic, which is of type `region_monitor/Region`.

Once the node is running and a region is set, use the service `region_monitor/monitor` to check occupancy status of the region.

## About

This package was quickly made as a way to check whether or not a door was open
in a navigation map. While I'm fully aware that it is not the most elegant or
robust solution to this particular problem, it worked for the purposes of a
robotics research study and so I made it public in hopes that it may be helpful
to someone in the future.