#include <serial/serial.h>
#include <ros/ros.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Char.h>
#include <motor_msgs/State.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// 假设电机速度为3200，位置为1254。
// 单片机输出数据的流程：首先输出字符a,然后输出速度数据3200的3，2，0，0；最后输出速度数据1254的1，2，5，4
// 都是一个字节一个字节的发到电脑的usb的接收缓冲区。


//声明串口对象
serial::Serial my_serial;

int main(int argc, char** argv) {
    // 初始化ROS节点
    ros::init(argc, argv, "motor_speed_publisher_node");
    ros::NodeHandle nh;

    // 创建一个发布者，发布到"motor_speed_topic"题目，消息类型为int32
    ros::Publisher motor_speed_pub = nh.advertise<motor_msgs::State>("motor_speed_topic", 10);

    //设置串口打开方式，尝试打开。如果打开串口过程出现异常，则抛出异常
    try
    {
        my_serial.setPort("/dev/ttyUSB0"); 
        my_serial.setBaudrate(9600);
        serial::Timeout to = serial::Timeout::simpleTimeout(1000);
        my_serial.setTimeout(to);
        my_serial.open();
    }
    catch (serial::IOException& e)
    {
        ROS_ERROR_STREAM("Unable to open port" << e.what());
        return -1;
    }

    //对串口是否打开进行处理，如果打开了给出成功提示，如果没打开直接结束程序。
    if (my_serial.isOpen())
    {
        ROS_INFO_STREAM("Serial Port initialized");
    }
    else
    {
        return -1;
    }

    //循环读取串口并处理数据。转换为整数后发送到指定话题。
    ros::Rate loop_rate(5);
    while (ros::ok())
    {
        motor_msgs::State msg;
        string temp_data;
        int32_t my_speed = 0;
        int32_t my_pos = 0;
        
     
        if(my_serial.available() >= 9)
        {
            uint8_t buffer[11] = {0};
            my_serial.read(buffer,9);
            stringstream ss;
            stringstream ss1;
            
            //能读到9个字节的数据
            for(int i=0;i<9;i++)
            {
                cout<<buffer[i];
            }
            cout << endl;

            if(buffer[0]=='a')
            {
               
                //恢复速度数据
                for(int i = 0; i < 4; i++)
                {
                    ss << buffer[i+1];
                }
                ss >> my_speed;
                msg.speed = my_speed;
                ss.str();

                //恢复位置数据
                for(int i = 0; i < 4; i++)
                {
                    cout<<"位置循环"<<endl;
                    ss1 << buffer[i+5];
                }
                ss1 >> my_pos;
                msg.pos = my_pos;
                cout<<"pos="<<msg.pos<<endl;
                ss.str();
            }
            //发布电机数据到话题
            motor_speed_pub.publish(msg);

            cout << "速度为：" << msg.speed <<"  位置为："<< msg.pos << endl;
            
        }
   
        loop_rate.sleep();
    }

    return 0;
 }