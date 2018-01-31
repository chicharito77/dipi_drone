#define NUMBER_OF_BEACONS              1
#define QUEUE_SIZE_1				1000
#define STARTUP_TIMEOUT               10

#define MM_TOPICNAME                                    "/hedge_pos_a"
#define SERVICENAME         "/LocationProvider/provide_hedge_location"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "ros/ros.h"
#include "marvelmind_nav/hedge_pos_a.h"
#include "geometry_msgs/Point.h"
#include "ros/master.h"
#include "DroneNavigationPackage/HedgePositions.h"

using namespace std;

ros::Subscriber locationReceiver;
ros::ServiceServer serviceProvider;

std::map<int, geometry_msgs::Point> receivedLocations;


bool provide_location(DroneNavigationPackage::HedgePositions::Request &req,
                      DroneNavigationPackage::HedgePositions::Response &res  )
{
    int i=0;
    map<int, geometry_msgs::Point>::iterator it;

    for(; i<NUMBER_OF_BEACONS; i++)
    {
        it = receivedLocations.find(req.addresses[i]);
        if (it != receivedLocations.end())
        {
            geometry_msgs::Point pi = receivedLocations[req.addresses[i]];
            res.positions[i].address = req.addresses[i];
            res.positions[i].x_m = pi.x;
            res.positions[i].y_m = pi.y;       
            res.positions[i].z_m = pi.z;
            res.positions[i].flags = 1;
        }
        else
        {
            res.positions[i].address = req.addresses[i];
            res.positions[i].x_m = 0.0;
            res.positions[i].y_m = 0.0;
            res.positions[i].z_m = 0.0;
            res.positions[i].flags = 0;    
        }
    }

    return true;
}


void locationReceivedCallback(const marvelmind_nav::hedge_pos_a::ConstPtr& msg)
{
    if ( msg->flags == 2 )
    {
        map<int, geometry_msgs::Point>::iterator it = receivedLocations.find(msg->address);
        geometry_msgs::Point newPos;
        newPos.x = msg->x_m;
        newPos.y = msg->y_m;
        newPos.z = msg->z_m;
        
        if (it != receivedLocations.end())
        {
            //address found
            receivedLocations[msg->address] = newPos;
        }
        else
        {
            //address not found
            receivedLocations.insert(std::pair<int, geometry_msgs::Point>( msg->address, newPos ));
        }
    }
}


bool isTopicAvailable(const string& topic)
{
    ros::master::V_TopicInfo topic_infos;
    ros::master::getTopics(topic_infos);

    for (ros::master::V_TopicInfo::iterator it = topic_infos.begin() ; it != topic_infos.end(); it++) 
    {
        const ros::master::TopicInfo& info = *it;
        if(info.name.compare(topic)==0)
        {
            return true;
        }
    }
    return false;
}

void startNode(ros::NodeHandle* n)
{
    time_t start, current;
    start = time(NULL);
    
    do
    {
        if (isTopicAvailable(MM_TOPICNAME))
        {
            serviceProvider = n->advertiseService(SERVICENAME, provide_location);
            locationReceiver = n->subscribe(MM_TOPICNAME, QUEUE_SIZE_1, locationReceivedCallback);

            ROS_INFO("Service is ready to call ...");
            ros::spin();
            break;
        }
        current = time(NULL);
    }while(difftime(current, start) < STARTUP_TIMEOUT);

    ROS_ERROR("TIMEOUT: Node doesn't start because dependency topic does not exist!");
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "LocationProvider");
    ros::NodeHandle n;

    startNode(&n);

    /*boost::algorithm::split( lv_elems, topic_infos[0].name, boost::algorithm::is_any_of( lc_delim ) );

    int i=0;
    for(; i<lv_elems.size(); i++)
    {
        printf("%s\n", lv_elems[0].c_str());
    }

    bond::Bond _bond("/hedge_pos_a", "55420");
    _bond.start();

    if (! _bond.waitUntilFormed(ros::Duration(5.0)))
    {
        ROS_ERROR("ERROR!");
        return false;
    }
    else
    {
    }*/

    return 0;
}