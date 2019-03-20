/**
 * 该例程将发布chatter话题，消息类型String
 */
 
#include <sstream>
#include <queue>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <thread>
#include <unistd.h>


class cpu_NetVLAD{
public:
    cpu_NetVLAD(){
        cpu_netvlad_sub = n.subscribe("dpu_NetVLAD", 3, &cpu_NetVLAD::push2queue, this);
    }
    static void cpuNetVLADPub(void* __this);
    void push2queue(const std_msgs::String::ConstPtr& msg);
    std::thread cpu_thread;
    void startcputhread(){
        cpu_thread = std::thread(cpuNetVLADPub, this);
    }
    std::queue <std_msgs::String::ConstPtr> dpu_netvlad_queue;

private: 
    ros::NodeHandle n;
    ros::Subscriber cpu_netvlad_sub;
};

void cpu_NetVLAD::push2queue(const std_msgs::String::ConstPtr& msg){
    ROS_INFO("cpu_NetVLAD heard: [%s]", msg->data.c_str());
    dpu_netvlad_queue.push(msg);
}

void cpu_NetVLAD::cpuNetVLADPub(void* __this)
{
    cpu_NetVLAD* _this = (cpu_NetVLAD*)  __this;
    ros::NodeHandle n;
    ros::Publisher cpu_netvlad_pub = n.advertise<std_msgs::String>("cpu_NetVLAD", 3);;
    ros::Rate loop_rate2(50);
    int tmp;
    // n.param<int>("running_cpu_NetVLAD",tmp,0);
    // n.param<int>("depth_cpu_NetVLAD",tmp,0);
    while (n.ok()){
        if ( (! _this->dpu_netvlad_queue.empty() ) ){
            n.setParam("running_cpu_NetVLAD", 1);
            n.setParam("depth_cpu_NetVLAD", int(_this->dpu_netvlad_queue.size() ) );
            ROS_INFO("CPU NetVLAD heard: [%s]", _this->dpu_netvlad_queue.front()->data.c_str() );

            // doing computation

            usleep(360*1000);
            std::stringstream ss;
            std_msgs::String msg_pub;
            ss << "  CPU NetVLAD result: " << _this->dpu_netvlad_queue.front()->data.c_str();
            msg_pub.data = ss.str();
            cpu_netvlad_pub.publish(msg_pub);
            ros::spinOnce();
            _this->dpu_netvlad_queue.pop();
            
            n.setParam("running_cpu_NetVLAD", 0);
            n.setParam("depth_cpu_NetVLAD", int(_this->dpu_netvlad_queue.size() ) );
        }
        else{
            n.setParam("running_cpu_NetVLAD", 0);  
            n.setParam("depth_cpu_NetVLAD", 0);           
            ROS_INFO_STREAM("Queue empty");
            loop_rate2.sleep();
        }
    }


}

int main(int argc, char **argv)
{
    // ROS节点初始化
    ros::init(argc, argv, "cpu_NetVLAD");
    cpu_NetVLAD cpu_NetVLAD_inst;
    ros::Rate loop_rate(50);
    cpu_NetVLAD_inst.startcputhread();

    while (ros::ok()){
        ros::spinOnce();
        loop_rate.sleep();
    }
    cpu_NetVLAD_inst.cpu_thread.join();
    return 0;
}
