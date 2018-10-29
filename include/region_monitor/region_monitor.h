// Copyright (c) 2016, Adam Allevato
// Copyright (c) 2017, The University of Texas at Austin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _REGION_MONITOR_H_
#define _REGION_MONITOR_H_

#define _USE_MATH_DEFINES
#include <cmath>

// TODO(Kukanani): clean up these includes and the whole header section.
#include <pcl/ModelCoefficients.h>
#include <pcl/common/transforms.h>
#include <pcl/features/normal_3d.h>
#include <pcl/filters/conditional_removal.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/segmentation/sac_segmentation.h>

#include <dynamic_reconfigure/server.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl_ros/transforms.h>
#include <ros/ros.h>
#include <sensor_msgs/point_cloud_conversion.h>
#include <tf/transform_listener.h>

#include <region_monitor/Monitor.h>
#include <region_monitor/Region.h>

typedef pcl::PointCloud<pcl::PointXYZ> PC;
typedef pcl::PointCloud<pcl::PointXYZ>::Ptr PCPtr;

/**
 * Monitor a region of space for any points. This was originally written for
 * the Guido guide-bot project, to see when a door region was clear and
 * initiate motion.
 */
class RegionMonitor {
private:
  ///////////////////////////////////////////////////////////////////////////////
  // CLASS VARS
  ///////////////////////////////////////////////////////////////////////////////
  /// Standard ROS node handle
  ros::NodeHandle node;
  ros::AsyncSpinner spinner;

  /// Accepts the segmentation requests
  ros::ServiceServer monitorServer;

  /// Set the region of interest
  ros::Subscriber regionSub;

  /// Publishes clipped cloud for visualization
  ros::Publisher boundedScenePublisher;

  /// Listens for point clouds
  ros::Subscriber pointCloudSub;

  /// Used to transform into the correct processing/recognition frames
  tf::TransformListener listener;
  std::string transformToFrame;

  ///////////////////////////////////////////////////////////////////////////////
  // SEGMENTATION PARAMS
  ///////////////////////////////////////////////////////////////////////////////
  // The minimum camera-space X for the working area bounding box
  float minX; // left in world space
  float maxX; // right in world space
  float minY; // up in world space
  float maxY; // down in world space
  float minZ; // near clipping in world space
  float maxZ; // far clipping in world space

  /// input is stored here
  PCPtr inputCloud;

  ///////////////////////////////////////////////////////////////////////////////
  // FILTERING STEPS (FUNCTIONS)
  ///////////////////////////////////////////////////////////////////////////////

  /**
   * Spatially filter a point cloud
   * @param  unclipped the point cloud to be filtered
   * @return           the filtered point cloud
   */
  PCPtr clipByDistance(PCPtr &unclipped);

public:
  /// Basic constructor
  RegionMonitor();

  /// Start!
  void run();

  /// Do the segmentation steps enabled by parameter flags and return the
  /// result.
  void cb_params(const region_monitor::Region::ConstPtr &region);

  /// Called when a point cloud comes in
  void cb_pointCloud(const sensor_msgs::PointCloud::ConstPtr &cloud);

  /// process a request to monitor.
  bool cb_monitor(region_monitor::MonitorRequest &req, region_monitor::MonitorResponse &res);
}; // RegionMonitor

#endif //_SEGMENTATION_H_
