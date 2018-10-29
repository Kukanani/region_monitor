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

#include "region_monitor/region_monitor.h"

int main(int argc, char **argv) {
  // Start the segmentation node and all ROS publishers
  ros::init(argc, argv, "region_monitor");
  pcl::console::setVerbosityLevel(pcl::console::L_ALWAYS);
  ROS_INFO("Starting Region Monitor");
  RegionMonitor r;
  r.run();
  return 1;
}

RegionMonitor::RegionMonitor()
    : node("region_monitor"), transformToFrame(""), listener(), spinner(4) {
  ros::NodeHandle privateNode("~");
  if (!privateNode.getParam("clippingFrame", transformToFrame)) {
    transformToFrame = "world";
  }

  inputCloud = PCPtr(new PC());

  boundedScenePublisher =
      privateNode.advertise<sensor_msgs::PointCloud2>("bounded_scene", 1);

  regionSub = privateNode.subscribe("set_region", 1000,
                                    &RegionMonitor::cb_params, this);
  pointCloudSub = node.subscribe("/rosarnl_node/S3Series_1_pointcloud", 1000,
                                 &RegionMonitor::cb_pointCloud, this);

  monitorServer =
      privateNode.advertiseService("monitor", &RegionMonitor::cb_monitor, this);
}

void RegionMonitor::run() {
  ROS_INFO("Region Monitor running...");
  spinner.start();
  ros::waitForShutdown();
}

void RegionMonitor::cb_params(const region_monitor::Region::ConstPtr &region) {
  minX = region->min_x;
  maxX = region->max_x;
  minY = region->min_y;
  maxY = region->max_y;
  minZ = region->min_z;
  maxZ = region->max_z;
  ROS_INFO("[region_monitor] Monitoring region set.");
}

bool RegionMonitor::cb_monitor(region_monitor::MonitorRequest &req,
                               region_monitor::MonitorResponse &res) {
  // ROS_INFO("clipping by distance.");
  PCPtr blah = clipByDistance(inputCloud);

  // ROS_INFO("checking occupation");
  res.occupied = !(blah->empty());

  // ROS_INFO("publishing point cloud");
  sensor_msgs::PointCloud2 outgoing;
  pcl::toROSMsg(*blah, outgoing);
  boundedScenePublisher.publish(outgoing);
  // ROS_INFO("done");
  return true;
}

void RegionMonitor::cb_pointCloud(
    const sensor_msgs::PointCloud::ConstPtr &cloud) {
  // ROS_INFO("received input cloud");
  sensor_msgs::PointCloud2 interimPC2, transformedPC2;

  // ROS_INFO("converting to point cloud 2");
  sensor_msgs::convertPointCloudToPointCloud2(*cloud, interimPC2);
  // ROS_INFO("transforming into other frame");
  pcl_ros::transformPointCloud(transformToFrame, interimPC2, transformedPC2,
                               listener);

  // ROS_INFO("converting from ros message to PCL point cloud");
  inputCloud->points.clear();
  pcl::fromROSMsg(transformedPC2, *inputCloud);
}

bool compareClusterSize(const sensor_msgs::PointCloud2 &a,
                        const sensor_msgs::PointCloud2 &b) {
  return a.width > b.width;
}

PCPtr RegionMonitor::clipByDistance(PCPtr &unclipped) {
  PCPtr processCloud = PCPtr(new PC());
  // processCloud->resize(0);

  // We must build a condition.
  // And "And" condition requires all tests to check true.
  // "Or" conditions also available.
  // Checks available: GT, GE, LT, LE, EQ.
  pcl::ConditionAnd<pcl::PointXYZ>::Ptr clip_condition(
      new pcl::ConditionAnd<pcl::PointXYZ>);
  clip_condition->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(
      new pcl::FieldComparison<pcl::PointXYZ>("x", pcl::ComparisonOps::GT, minX)));
  clip_condition->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(
      new pcl::FieldComparison<pcl::PointXYZ>("x", pcl::ComparisonOps::LT, maxX)));
  clip_condition->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(
      new pcl::FieldComparison<pcl::PointXYZ>("y", pcl::ComparisonOps::GT, minY)));
  clip_condition->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(
      new pcl::FieldComparison<pcl::PointXYZ>("y", pcl::ComparisonOps::LT, maxY)));
  clip_condition->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(
      new pcl::FieldComparison<pcl::PointXYZ>("z", pcl::ComparisonOps::GT, minZ)));
  clip_condition->addComparison(pcl::FieldComparison<pcl::PointXYZ>::ConstPtr(
      new pcl::FieldComparison<pcl::PointXYZ>("z", pcl::ComparisonOps::LT, maxZ)));

  // Filter object.
  pcl::ConditionalRemoval<pcl::PointXYZ> filter;
  filter.setCondition(clip_condition);
  filter.setInputCloud(unclipped);
  // If true, points that do not pass the filter will be set to a certain value
  // (default NaN). If false, they will be just removed, but that could break
  // the structure of the cloud (organized clouds are clouds taken from
  // camera-like sensors that return a matrix-like image).
  filter.setKeepOrganized(true);
  // If keep organized was set true, points that failed the test will have
  // their Z value set to this.
  filter.setUserFilterValue(0.0);

  filter.filter(*processCloud);
  return processCloud;
}