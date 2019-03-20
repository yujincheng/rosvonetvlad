/**
 * 该例程将发布chatter话题，消息类型String
 */
 
#include <sstream>
#include <queue>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <thread>
#include <unistd.h>


class dpu_VO{
public:
    dpu_VO(){
        sub_frame = n.subscribe("frame", 1, &dpu_VO::push2queue,this);
    }
    void push2queue(const std_msgs::String::ConstPtr& msg);
    static void callbackThread(void* __this);
    void startdputhread(){
        dpu_thread = std::thread(callbackThread, this);
    }
    std::thread dpu_thread;
    std::queue <std_msgs::String::ConstPtr> frame_queue;

private: 
    ros::NodeHandle n;
    ros::Subscriber sub_frame;
};

void dpu_VO::push2queue(const std_msgs::String::ConstPtr& msg){
    ROS_INFO("I heard: [%s]", msg->data.c_str());
    frame_queue.push(msg);
}

void dpu_VO::callbackThread(void* __this)
{
  dpu_VO * _this =(dpu_VO *)__this;
  ROS_INFO_STREAM("Callback thread id=" << std::this_thread::get_id());

  ros::NodeHandle n;
    int tmp;
  ros::Rate loop_rate2(50);
  ros::Publisher dpu_vo_pub = n.advertise<std_msgs::String>("dpu_VO", 3);
  int running_dpu_NetVLAD;
  int running_cpu_NetVLAD;
  int depth_cpu_NetVLAD;
    // n.param<int>("running_dpu_VO",tmp,0);
    // n.param<int>("depth_dpu_VO",tmp,0);

  while (n.ok())
  {
    n.param<int>("running_dpu_NetVLAD", running_dpu_NetVLAD, 0);
    n.param<int>("running_cpu_NetVLAD", running_cpu_NetVLAD, 0);
    n.param<int>("depth_cpu_NetVLAD",depth_cpu_NetVLAD,0);
    if ( (! _this->frame_queue.empty())){
        ROS_INFO_STREAM(" ;running_dpu_NetVLAD: " << running_dpu_NetVLAD << " ;depth_cpu_NetVLAD: " << depth_cpu_NetVLAD << " ;running_cpu_NetVLAD: " << running_cpu_NetVLAD);
        if ( (! running_dpu_NetVLAD) && ( (depth_cpu_NetVLAD <= 1 && running_cpu_NetVLAD) || (depth_cpu_NetVLAD < 1 && ! running_cpu_NetVLAD) ) ){
            n.setParam("running_dpu_VO", 1);
            n.setParam("depth_dpu_VO", int(_this->frame_queue.size() ) );
            ROS_INFO_STREAM("Queue deepth: " << _this->frame_queue.size() << " ;WORD: " << _this->frame_queue.front()->data.c_str() );
            
            // doing computation
            usleep(3*1000);
            
            std::stringstream ss;
            std_msgs::String msg;
            ss << "DPU VO result: [%s]" << _this->frame_queue.front()->data.c_str();
            msg.data = ss.str();
            dpu_vo_pub.publish(msg);
            ros::spinOnce();
            _this->frame_queue.pop();
            n.setParam("running_dpu_VO", 0);
            n.setParam("depth_dpu_VO", int(_this->frame_queue.size() ) );
            // loop_rate2.sleep();
        }
        else{
            ROS_INFO_STREAM("Queue not empty, but not start");
            n.setParam("depth_dpu_VO", int(_this->frame_queue.size() ) );
            loop_rate2.sleep();
        }
    }
    else {
        n.setParam("running_dpu_VO", 0);
        n.setParam("depth_dpu_VO", 0 );
        ROS_INFO_STREAM("Queue empty");
        loop_rate2.sleep();
    }
  }
}

int main(int argc, char **argv)
{
    // ROS节点初始化
    ros::init(argc, argv, "dpu_VO");
    dpu_VO dpu_VO_inst;
    ros::Rate loop_rate(50);
    dpu_VO_inst.startdputhread();
    
    while (ros::ok()){
        ros::spinOnce();
        loop_rate.sleep();
    }
    dpu_VO_inst.dpu_thread.join();
    return 0;
}
