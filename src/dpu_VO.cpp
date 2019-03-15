/**
 * 该例程将发布chatter话题，消息类型String
 */
 
#include <sstream>
#include <queue>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <thread>


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
  ros::Rate loop_rate(20);
  while (n.ok())
  {
    if ( ! _this->frame_queue.empty() ){
        ROS_INFO_STREAM("Queue deepth: " << _this->frame_queue.size() << " ;WORD: " << _this->frame_queue.front()->data.c_str() );
        _this->frame_queue.pop();
    }
    else {
        ROS_INFO_STREAM("Queue empty");
    }
    loop_rate.sleep();
  }
}

int main(int argc, char **argv)
{
    // ROS节点初始化
    ros::init(argc, argv, "dpu_VO");
    dpu_VO dpu_VO_inst;
    ros::Rate loop_rate(10);
    dpu_VO_inst.startdputhread();
    
    while (ros::ok()){
        ros::spinOnce();
        loop_rate.sleep();
    }
    dpu_VO_inst.dpu_thread.join();
    return 0;
}