/**
 * 该例程将发布chatter话题，消息类型String
 */
 
#include <sstream>
#include <queue>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <thread>
#include <unistd.h>


class cpu_VO{
public:
    cpu_VO(){
        cpu_vo_sub = n.subscribe("dpu_VO", 5, &cpu_VO::cpuVOPub, this);
        cpu_vo_pub = n.advertise<std_msgs::String>("cpu_VO", 3);
    }
    void cpuVOPub(const std_msgs::String::ConstPtr& msg);

private: 
    ros::NodeHandle n;
    ros::Subscriber cpu_vo_sub;
    ros::Publisher cpu_vo_pub;
};


void cpu_VO::cpuVOPub(const std_msgs::String::ConstPtr& msg)
{
    ROS_INFO("CPU VO heard: [%s]", msg->data.c_str());
    int tmp;
    
    // n.param<int>("running_cpu_VO",tmp,0);

    // doing computation
    n.setParam("running_cpu_VO", 1);
    usleep(10*1000);
    std::stringstream ss;
    std_msgs::String msg_pub;
    ss << "  CPU VO result: " << msg->data.c_str();
    msg_pub.data = ss.str();
    cpu_vo_pub.publish(msg_pub);
    n.setParam("running_cpu_VO", 0);

}

int main(int argc, char **argv)
{
    // ROS节点初始化
    ros::init(argc, argv, "cpu_VO");
    cpu_VO cpu_VO_inst;
    ros::Rate loop_rate(50);

    while (ros::ok()){
        // while()
        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}
