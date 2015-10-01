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

template < typename T, 
           size_t n >
   class OneToMany : public raft::kernel 
{
   /** 
    * you might be able to do this with std::is_null_pointer
    * along with std::enable_if once c++14 is widely avail.
    */
	static_assert( n > 0, "n must be positive");

public:
	
   OneToMany()
   {
		input.addPort< T >("input");
		for (size_t i = 0; i < n; i++) 
      {
			output.addPort< T >( std::to_string(i) );
		}
	}

	virtual raft::kstatus run() 
   {
	   auto &img_in( input[ "input" ].template peek< T >() );
      for( auto i( 0 ); i < n; i++) 
      {
         auto &port( output[ std::to_string( i ) ] );
         auto &img( port.template allocate< T >() );
         std::memcpy( &img, &img_in, sizeof( T ) );
		   port.send();
      }
      input[ "input" ].recycle();
	   return( raft::proceed );
	}	
};


template <typename T>
class Source : public raft::kernel {
public:
	Source(cv::VideoCapture device) : _device(device) 
   {
		output.addPort< T >("output");
	}

	virtual raft::kstatus run() 
   {
      volatile T x;

		T &img( output[ "output" ]. template allocate< T >() );
		if (_device.grab() && _device.retrieve( img ) ) 
      {
			output[ "output" ].send();
			return raft::proceed;
		}
      else
      {
         output[ "output" ].deallocate();
      }
		return raft::stop;
	}

private:
	cv::VideoCapture _device;
};

template <typename T>
class Blur : public raft::kernel {
public:
	Blur( cv::Size kSize, 
         cv::Point anchor = cv::Point(-1,-1), 
         int borderType= cv::BORDER_DEFAULT ) :
                                                   kSize_(kSize), 
                                                   anchor_(anchor), 
                                                   borderType_(borderType) 
	{
		input.addPort < T >( "input"  );
		output.addPort< T >( "output" );
	}

	Blur( std::size_t sizeX = 5, 
         std::size_t sizeY = 5, 
         cv::Point anchor  = cv::Point(-1,-1), 
         int borderType    = cv::BORDER_DEFAULT ) : 
	                                                   kSize_(sizeX, sizeY), 
                                                      anchor_(anchor), 
                                                      borderType_(borderType)
	{
		input.addPort<  T >( "input"  );
		output.addPort< T >( "output" );
	}



	virtual raft::kstatus run() {

		T &img( input["input"].template peek< T >() );
      T &edge( output[ "output" ].template allocate< T >() );
		
		T gray;
		cv::cvtColor(  img,   
                     gray, 
                     CV_BGR2GRAY );
		
      cv::Canny(  gray,
                  edge, 
                  1, 
                  300, 
                  3 );

      output[ "output" ].send();
      input[ "input" ].recycle();
      
      return( raft::proceed );
	}

private:
	cv::Size       kSize_;
	cv::Point      anchor_;
	int            borderType_; 
};



class FindContoursOutput {
public:
	std::vector<std::vector<cv::Point> >contours;
	std::vector<cv::Vec4i> hierarchy;
};



class FindContours : public raft::kernel {
   using fco = FindContoursOutput;
   using cvm = cv::Mat;
public:
	FindContours(  int mode,          
		            int method,
		            cv::Point offset ) : FindContours() 
   {
      mode_    =  mode; 
      method_  =  method; 
      offset_  =  offset; 
	}

   FindContours()
   {
		input.addPort < cvm >( "input"  );
		output.addPort< fco >( "output" );
   }

	virtual raft::kstatus run() 
   {
      /** peek mem on inbound port **/
		auto &img( input["input"].template peek< cvm >() );
	   /** allocate mem on outbound port **/
      auto &out( output[ "output" ].template allocate< fco >() );
		
      cv::findContours( img, 
                        out.contours, 
                        out.hierarchy, 
                        mode_, 
                        method_, 
                        offset_);
      /** release mem to next kernel **/
	   output[ "output" ].send();
      /** free mem on inbound port **/
      input[ "input" ].recycle();
      /** 
       * only need to call raft::proceed, the runtime will stop when 
       * no more data is available.
       */
	   return( raft::proceed );
	}

private:
   /** this just makes ctor reading easier **/
	int         mode_       = CV_RETR_TREE;
	int         method_     = cv::CHAIN_APPROX_SIMPLE;
	cv::Point   offset_     = cv::Point();
};


class DrawContours : public raft::kernel
{
   using fco = FindContoursOutput;
   using cvm = cv::Mat;

public:
	DrawContours(  int contourIdx = -1, 
                  // meant to be red for RGB, but if BGR, will be blue
		            const cv::Scalar color = cv::Scalar(255,0,0), 
		            int thickness = 1,
		            int lineType = 8,
		            int maxLevel = INT_MAX,
		            cv::Point offset = cv::Point()) :
				                                 contourIdx_(contourIdx),
				                                 color_(color),
				                                 thickness_(thickness),
				                                 lineType_(lineType),
				                                 maxLevel_(maxLevel),
				                                 offset_(offset) 
   {
			input.addPort<  cvm >( "inputImg"       );
			input.addPort<  fco >( "inputContours"  );
			output.addPort< cvm >( "output"         );	
	}

	virtual raft::kstatus run() 
   {
		auto &in_( input[ "inputContours" ].template peek< fco >() );
      auto &img( input[ "inputImg" ].template peek< cvm >() );
		

		for( auto  i( 0 ); i < in_.contours.size(); i++ ) 
      {
			cv::drawContours( img, 
                           in_.contours, 
                           i, 
                           color_, 
                           thickness_, 
                           lineType_, 
                           in_.hierarchy, 
                           maxLevel_, 
                           offset_ );
		}
      /** don't need this piece anymore, release it **/
      input[ "inputContours" ].recycle();
      /** allocate mem **/
		auto &dst( output[ "output" ].template allocate< cvm >() );
      std::memcpy( &dst, &img, sizeof( cvm ) );
      /** release cvm **/
      input[ "inputImg" ].recycle();
      /** send mem to next kernel **/
		output["output"].send();
	   return( raft::proceed );
   }

private:
	int               contourIdx_;
	const cv::Scalar  color_;
	int               thickness_;
	int               lineType_;
	int               maxLevel_;
	cv::Point         offset_;
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

		cv::waitKey(1);
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

	cv::VideoCapture cap0( 0 );
	
	if( not cap0.isOpened() )
   {
		std::cout << "cannot open video camera 0!\n";
      return( EXIT_FAILURE );
	}


	auto link1 = map.link(  new Source<    cvm >( cap0 ), 
                           new OneToMany< cvm, 2>() );

	auto link2 = map.link( &(link1.getDst()), std::to_string( 0 ), 
                          new Blur< cvm >() );

	auto link3 = map.link( &(link2.getDst()), 
                          new FindContours() );

	auto link4 = map.link( &(link1.getDst()), std::to_string( 1 ), 
                          new DrawContours(), "inputImg" );

	auto link5 = map.link( &(link3.getDst()), 
                          &(link4.getDst()), "inputContours" );

	map.link( &(link5.getDst()), 
             new Display< cvm >( "result" ) );
	
   map.exe();

	return( EXIT_SUCCESS );
}
