#include <swarm_control/SwarmPerception.h>


SwarmPerception::SwarmPerception(ros::NodeHandle & n): n_(n),n_priv_("~")
{
    // Initialize laser topics
    XmlRpc::XmlRpcValue swarm;
    n_priv_.getParam("swarm", swarm);
    ROS_INFO_STREAM("Loading swarm of " << swarm.size() << " robots.");
    std::map<std::string, XmlRpc::XmlRpcValue>::iterator i;
    for (i = swarm.begin(); i != swarm.end(); i++)
    {
        std::string laser_topic="/"+std::string(i->first)+"/"+std::string(i->second["laser_topic"]);
        std::string laser_frame="/"+std::string(i->first)+"/"+std::string(i->second["laser_frame"]);
        laser_topic_.insert(std::pair<std::string,std::string>(i->first, laser_topic));
        laser_frame_.insert(std::pair<std::string,std::string>(i->first, laser_frame));
        laser_readings_.insert(std::pair<std::string,sensor_msgs::PointCloud>(laser_frame, sensor_msgs::PointCloud()));


        laser_sub_.insert(std::pair<std::string,ros::Subscriber>(i->first,n_.subscribe<sensor_msgs::LaserScan>(laser_topic, 4, boost::bind(&SwarmPerception::laserCallback, this,_1))));
        std::cout << laser_topic <<std::endl;

    }

    point_cloud_pub_=n_priv_.advertise<sensor_msgs::PointCloud>("swarm_perception",10);

}

/**
 * This tutorial demonstrates simple receipt of messages over the ROS system.
 */
void SwarmPerception::laserCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    sensor_msgs::PointCloud cloud;
    std::map<std::string,sensor_msgs::PointCloud>::iterator it;

    try
    {
        ros::Time now = ros::Time::now();
        listener_.waitForTransform("swarm_centroid", msg->header.frame_id, now, ros::Duration(3.0));
        projector.transformLaserScanToPointCloud("swarm_centroid",*msg, cloud,listener_);

        it=laser_readings_.find(msg->header.frame_id);
        if(it!=laser_readings_.end())
        {
            it->second=cloud;
        }
    }
    catch (tf::TransformException ex)
    {
        ROS_INFO("%s",ex.what());
        return;
    }
    sensor_msgs::PointCloud total_cloud;
    //for(it=laser_readings_.begin();it!=laser_readings_.end();++it)
    //{
    //    total_cloud+=it->second;
    //}

    point_cloud_pub_.publish(cloud);
}

int main(int argc, char **argv)
{

  ros::init(argc, argv, "listener");
  ros::NodeHandle n_priv("~");
  ros::NodeHandle n;

  SwarmPerception swarm_perception(n);

  ros::AsyncSpinner spinner(4);
  spinner.start();
  ros::Rate loop_rate(50);
  while (ros::ok())
  {
    loop_rate.sleep();
  }
  spinner.stop();
  return 0;
}
