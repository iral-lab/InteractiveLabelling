 // ROS core
 #include <ros/ros.h>
 #include "vision.h"
 #include "lang.h"
 #include "model.h"
 #include <cstdio>
 #include <ctime>

// PCL includes
 #include <pcl/io/io.h>
 #include <pcl/io/pcd_io.h>
 #include <pcl/point_types.h>
 #include <sensor_msgs/PointCloud2.h>
 #include <pcl_conversions/pcl_conversions.h>
 #include <pcl/point_cloud.h>

 #include <image_transport/image_transport.h>
 #include <iostream>

 #include <string>
 #include <sstream>
#include <vector>

// Mutex: //
boost::mutex cloud_mutex;

bool paramFlag = false;
using namespace pcl;
using namespace std;
stringstream ss;

string tNoA = "25";

int trainOrtest = 0; // 0 for training , 1 for testing
string annotation = "";

bool execFlag = false;
int pcdNo = 1;

clock_t startTime;

// A common function that suscribes to kinect2 cloud points, 
// process depth,color information from the cloud points.

class  PointCloudToPCD {

	ros::NodeHandle nh_;
	image_transport::ImageTransport it_;
	image_transport::Subscriber image_sub_;
	ros::Subscriber pcl_sub_;

	public:
	PointCloudToPCD() : 
	it_(nh_){
	   pcl_sub_ = nh_.subscribe ("/kinect2/qhd/points", 1, &PointCloudToPCD::cloudCb, this);
	}

        vector<int> processDepth(const sensor_msgs::PointCloud2ConstPtr& input) {
          pcl::PointCloud<pcl::PointXYZRGBA> cloud;
           pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudPtr( new pcl::PointCloud<pcl::PointXYZRGBA>() );
           pcl::fromROSMsg (*input, *cloudPtr);
           cloud = *cloudPtr;
           pcl::io::savePCDFileASCII(ss.str(), cloud);
		
	  ostringstream saveNo;
     	  saveNo << pcdNo;
          saveNo << ".pcd";
          pcl::io::savePCDFileASCII(saveNo.str(), cloud);
          pcdNo++;

           return getDepthParameters(ss.str());
        }
	
	void cloudCb (const sensor_msgs::PointCloud2ConstPtr& input){
                cloud_mutex.lock();
                cout << endl;
                execFlag = false;
                vector<int> rgb = processDepth(input);
                vector<string> tokens= processLanguage(annotation);


		if(trainOrtest == 0) {
			train(rgb,tokens);
		} else {
			test(rgb,tokens);
		}
        	paramFlag = true;
                cloud_mutex.unlock();

	}
	
};
//A function to display test options
//1 means Novel Scene
//2 means Known Example

void displayTestOption() {
   if(trainOrtest == 1) {
      cout << "\nTest Options ::\n";
      cout <<   "            1 : Novel Scene/What is this?(Enter '1' )\n";
      cout <<   "            2 : Known Example/Is this 'Green'?(Enter description)\n";
   }
}
// A function that listen to rostopic and then execute
// training or testing function

void execute (string input) {
   startTime = clock();
   execFlag = true;
   annotation = input;
   paramFlag = false;
   cout << "Listening to rostopic..";
   while(! paramFlag)
   {
      ros::Duration( 0.01 ).sleep();
      ros::spinOnce();
      if(execFlag == true) {
         double duration ;
         duration = ( std::clock() - startTime ) / (double) CLOCKS_PER_SEC;
         double maxD = 0.05;
      
         if(duration > maxD) {
            cout << endl;
            cout << "Unable to subscribe to rostopic. So aborting the execution" << endl;
            break;
        }
      }
   }
   displayTestOption();
}

void saveDB() {
    
}

// Function to delete prevous trained models and data

void deleteTrain() {
   string cmd = "rm -rf /home/kamala/catkin_ws/src/jm/src/Train" + tNoA;
   cout << "Clearing Existing trained data ..." << endl;
   int ret = system(cmd.c_str());
}

// A function to save RGB values and it's description from a file

void saveFromFile() {
    string fName = "/home/kamala/catkin_ws/Arff/newTrain.txt";
//    string fName = "/home/kamala/catkin_ws/TrainModel/trainArff.txt";
    trainFromFile(fName);
}

void testAllConf() {
   string fName = "/home/kamala/catkin_ws/Arff/newTrain.txt";
   testConf(fName);
}

// A function to train,learn from a text file and test the classification 
void trainFromFileAndTestClassification() {
// testAllConf();
    string fName = "/home/kamala/catkin_ws/1.txt";
//    string fName = "/home/kamala/catkin_ws/TrainModel/trainArff.txt";
    trainFromFile(fName);
    testing();
}

// Main Function that registers rostopic and directs the execution

int  main (int argc, char** argv) {
   ss << "newobject.pcd";
   ros::init (argc, argv, "jmtest");

   PointCloudToPCD b;
   cout << "System is ready !!! " << endl;
   for (;;) {
      cout << ">> ";
      string input;
      getline(cin,input);
      if(input == "restore") {
         saveFromFile();
      } else if (input == "totaltest") {
         testAllConf();
      } else if(input == "exit") {
        saveDB();
        exit(0);
      } else if(input == "clear") {
        deleteTrain();  
      } else if(input == "train") {
        trainOrtest = 0;
      } else if(input == "test") {
	trainOrtest = 1;
        displayTestOption();
      } else if (input == "learn") {
         learnModel();
      }else {
        execute(input);
      }
    }

   return (0);
}

