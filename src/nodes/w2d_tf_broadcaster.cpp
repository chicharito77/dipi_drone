#define QUEUE_SIZE                                                       100
#define INCOMING_TOPIC_NAME         "/ControlCenter/set_transform_parameters"

#include <iostream>

#include "ros/ros.h"
#include "tf/transform_broadcaster.h"
#include "DroneNavigationPackage/TransformParameters.h"

int counter;
double X_w, Y_w, Z_w, rotationInRad;

tf::Quaternion rotationQuaternion;
ros::Subscriber operationTopic;


void setTransformationCallback(const DroneNavigationPackage::TransformParameters::ConstPtr& msg)
{
    X_w = msg -> X_w;
    Y_w = msg -> Y_w;
    Z_w = msg -> Z_w; 
    rotationInRad = msg->theta_dx_rad;   
    rotationQuaternion.setEuler(0, 0, msg->theta_dx_rad);
    counter = 0;
}


void initializeNode(ros::NodeHandle *node)
{
    Z_w = 0.0;
    X_w = 0.0;
    Y_w = 0.0;
    rotationInRad = 0.0;
    rotationQuaternion.setRPY(0,0,0);
    counter = 0;
    operationTopic = node->subscribe(INCOMING_TOPIC_NAME, QUEUE_SIZE, setTransformationCallback);

    ROS_INFO("TF broadcaster node started...\n");
}



int main(int argc, char** argv)
{
    ros::init(argc, argv, "W2D_tf_broadcaster");
    ros::NodeHandle node;
    ros::Rate r(15);
    tf::TransformBroadcaster broadcaster;

    initializeNode(&node);  

    while(ros::ok())
    {
        if (counter == 0)
        {
            ROS_INFO("\n\t/W->/odom details\n\toffset: (%.2f, %.2f, %.2f)\n\trot:=%.4f rad\n", X_w, Y_w, Z_w, rotationInRad);
            counter++;
        }

        broadcaster.sendTransform(
            
            tf::StampedTransform(
                            tf::Transform(rotationQuaternion, tf::Vector3(X_w, Y_w, Z_w)),
                            ros::Time::now(),"W", "odom"));
                                        //parent, child
        ros::spinOnce();
        r.sleep();
    }
}