#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <raft>

using cvm= typename cv::Mat;


template < class T > class source : public raft::kernel
{
public:
   source()
   {
      output.addPort< cvm >( "0" );
      if( not stream1.isOpened() ) 
      { 
         std::cout << "cannot open camera\n";
         exit( EXIT_FAILURE );
      }
   }

   virtual ~source() = default;

   virtual raft::kstatus run()
   {
      auto &frame( output[ "0" ].template allocate< cvm >() );
      stream1.read( frame );
      output[ "0" ].send();
      return( raft::proceed );
   }

private:
   cv::VideoCapture stream1 = { 0 };
};

static std::uint64_t frames = 0;
static double        start  = 0.0;
static double        end    = 0.0;

template < class T > class display : public raft::kernel
{
public:
   display()          
   {
      input.addPort< cvm >( "0" );
   }
   
   virtual ~display() = default;
   
   virtual raft::kstatus run()
   {
      auto &frame( input[ "0" ].template peek< cvm >() );
      cv::imshow( "cam", frame );
      cv::waitKey(1);
      /** decrement count within frame so it'll deallocate before recycle **/
      frame.release();
      input[ "0" ].recycle();
      frames++;
      if( frames % 1000  == 0 )
      {
         end = system_clock->getTime();
         const auto fps( frames / (end - start) );
         std::cout << fps << "\n";
      }
      return( raft::proceed );
   }
  
};

extern Clock *system_clock;

int main() 
{
   cv::namedWindow( "cam", cv::WINDOW_NORMAL );
   raft::map.link( 
      raft::kernel::make< source< cvm > >(), 
      raft::kernel::make< display< cvm > >() 
   );
   /** global time check **/
   start =  system_clock->getTime();
   raft::map.exe();
   return( EXIT_SUCCESS );
}

