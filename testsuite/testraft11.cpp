/**
 * testraft9.cpp
 *
 * toy example of sending data in multiple data types
 *
 * overlay
 *
 *
 * Cv::vector<vector<Point> > contours;
 * Cv::vector<Vec4i> hierarchy;
 * Cv::findContours( edgeImg, contours, hierarchy, CV_RETR_TREE , CHAIN_APPROX_SIMPLE);
 * Cv::drawContours( colorImg, contours, -1, RGB(255,0,0),1.5,8,hierarchy,2,Point());
 *
 * NOTE: cleaned up code, changed out smart ptrs since they weren't really
 * doing anything.  There's no way the "streams" can leak data, but if you
 * wanted to pass around ptrs you can but they can only be plain old data
 * pointers at the moment.  There's also no reason to use the signals unless
 * you're changing the app behavior based on them.  The runtime will detect
 * when no more data is coming in most cases (although in yours the camera
 * will run continuously).  For yours I'd suggest adding a signal for ctrl-d
 * or something to kill the app then propogate the signal through the application.
 * Since you weren't using them, I removed them.  
 */

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <iostream>
#include <cstdlib>
#include <raft>


template <typename T>
class Source : public raft::kernel {
public:
	Source()
   {
		output.addPort< T >("output");
	}

	virtual raft::kstatus run() 
   {
		T &img( output[ "output" ]. template allocate< T >() );
		device >> img;
		output[ "output" ].send();
		return( raft::proceed );
	}

private:
	cv::VideoCapture device = {0};
};


class FindContoursOutput {
public:
	std::vector<std::vector<cv::Point> >contours;
	std::vector<cv::Vec4i> hierarchy;
};



template <typename T> class Display : public raft::kernel
{
public:
	Display( cv::String winname ) : winname_( winname ) 
   {
		input.addPort< T >("input");
	}

	virtual raft::kstatus run()
   {
      /** peek input mem to avoid copy **/
		T &img( input[ "input" ].template peek< T >() );

		cv::imshow( winname_, 
                  img );

		cv::waitKey( 1 );
      /** free input mem, continue **/
      input[ "input" ].recycle();
		return( raft::proceed );
	}

private:
	cv::String winname_;
};


int 
main( int argc, char **argv )
{
   using namespace raft;
   using fco = FindContoursOutput;
   using cvm = cv::Mat;

	map.link(  new Source<    cvm >(), 
              new Display< cvm >( "result" ) );
   map.exe();

	return( EXIT_SUCCESS );
}
