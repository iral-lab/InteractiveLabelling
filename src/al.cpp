 // ROS core
 #include <ros/ros.h>
 #include "vision.h"
 #include "lang.h"
 #include "almodel.h"
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

string tNoB = "25";
int waitNoB = 0;
const string waitS[5] = {
 "Kindly wait. The system is processing",
 "Please wait for some time, the system is processing",
 "I am learning the object. It would take some time",
 "Processing always takes little time. Kindly wait",
 "System is trying to learn, so please wait"
}; 
// Mutex: //
boost::mutex cloud_mutex;

int qnType = 0;
int objNo = 1;
bool paramFlag = false;
using namespace pcl;
using namespace std;
stringstream ss;

int trainOrtest = 0; // 0 for training , 1 for testing
string annotation = "";

bool execFlag = false;
int pcdNo = 1;

clock_t startTime;

// common function that subscribes to rostopic and generates cloud 
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
		if(qnType == 0) {
			if(trainOrtest == 0) {
                           int waitMod = waitNoB % 5;
                           waitNoB++;
                           string str = waitS[waitMod];
                           string cmd = "echo \""+str+"\"  |festival --tts";
		//	   int ret = system(cmd.c_str());
       
			}
		}
                execFlag = false;
                vector<int> rgb = processDepth(input);
                string annt = annotation;
                for(int i = 0;annt[i];i++)
                   annt[i] = toupper(annt[i]);
                vector<string> tokens;
                if (annt.find("YES") != string::npos) {
                   tokens.push_back("yes");
		   for(int i = 0;annotation[i];i++)
                      annotation[i] = toupper(annotation[i]);

                   vector<string> tk = processLanguage(annotation);
                   for(size_t i = 0; i != tk.size(); i++) 
                      tokens.push_back(tk[i]);
                } else if(annt.find("NO") != string::npos) {
                   tokens.push_back("no");
                } else
                   tokens= processLanguage(annotation);


		if(trainOrtest == 0) {
			qnType = altrain(rgb,tokens,qnType);
		} else {
                       qnType = 0;
			altest(rgb,tokens);
		}
        	paramFlag = true;
                cloud_mutex.unlock();

	}
	
};
// displaying test options , where
// 1 denotes Novel Scene and
// 2 denotes Known Example

void displayTestOption() {
   if(trainOrtest == 1) {
      cout << "\nTest Options ::\n";
      cout <<   "            1 : Novel Scene/What is this?(Enter '1' )\n";
      cout <<   "            2 : Known Example/Is this 'Green'?(Enter description)\n";
      
   }
}
// Function that listens to rostopic and 
// directs to respective function for execution.

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
	    string str = "Unable to subscribe to rostopic. So aborting the execution" ;
	    string cmd = "echo \""+str+"\"  |festival --tts";
//	    int ret = system(cmd.c_str());

            cout << str << endl;
            break;
        }
      }
   }
   displayTestOption();
}

void saveDB() {
    
}
// Function that deletes earlier trained models and related files

void deleteTrain() {
   string cmd = "rm -rf /home/kamala/catkin_ws/src/jm/src/Train" + tNoB;
   cout << "Clearing Existing trained data ..." << endl;
   int ret = system(cmd.c_str());
}

// Function that trains RGB , description pair from file 
void saveFromFile() {
    string fName = "/home/kamala/catkin_ws/Arff/newTrain.txt";
//    string fName = "/home/kamala/catkin_ws/TrainModel/trainArff.txt";
  
    altrainFromFile(fName);
}

void testAllConfAL() {
   string fName = "/home/kamala/catkin_ws/Arff/newTrain.txt";
   testConfAL(fName);

}
// Main function that initializes rostopic and 
// displays options available

int  main (int argc, char** argv) {
   ss << "newobject.pcd";
   ros::init (argc, argv, "al");

   PointCloudToPCD b;
   string str = "System is ready !!! ";
   cout << str << endl;
   string cmd = "echo \""+str+"\"  |festival --tts";
//   echo "This is a test." |festival --tts
//   int ret = system(cmd.c_str());

   for (;;) {
      cout << ">> ";
      if(qnType == 0) {
         if(trainOrtest == 0) {
            objNo++;
            string str = "Please place new object and press 'Enter'.. ";
	    string cmd = "echo \""+str+"\"  |festival --tts";
	 //   int ret = system(cmd.c_str());
	    cout << str << endl << ">>";
	 } else
	    cout << endl << ">>";
      }
      string input;
      getline(cin,input);
      if(input == "restore") {
         saveFromFile();
      }  else if (input == "totaltest") {
         testAllConfAL();
      } else if(input == "exit") {
        saveDB();
        exit(0);
      } else if(input == "clear") {
        deleteTrain();  
      } else if(input == "train") {
        trainOrtest = 0;
        execute(input);
      } else if(input == "test") {
	trainOrtest = 1;
        displayTestOption();
  //    } else if (input == "learn") {
  //       learnModel();
      } else {
        execute(input);
      }
    }

   return (0);
}

