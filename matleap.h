/// @file matleap.h
/// @brief leap motion controller interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-12

#ifndef MATLEAP_H
#define MATLEAP_H

#define MAJOR_REVISION 0
#define MINOR_REVISION 7

#include "Leap.h"
#include "mex.h"
#include <queue>

#ifndef PTBGetSecs
    #define PTBGetSecs false
#endif
#if PTBGetSecs
    #include "GetSecs.h"

    #if PSYCH_SYSTEM != PSYCH_WINDOWS
        #include <sys/time.h>
    #endif
#else
    #include <chrono>
#endif


namespace matleap
{

/// @brief a leap frame
struct frame
{
    int64_t id;
    int64_t timestamp;
    double getsecs;
    Leap::PointableList pointables;
};

/// @brief leap frame grabber interface
class frame_grabber:  public Leap::Listener
{
    private:
    bool debug;
    bool listening;
    std::queue<frame> frames;
    std::mutex frames_mutex;
    frame latest_frame;
    std::mutex latest_frame_mutex;
    int maxNframes;
    int64_t lastsampleId=-1;
    #if !PTBGetSecs
        std::chrono::high_resolution_clock timer;
    #endif
        
    private:
	/// @brief internal function to get high precision timestamps
	///
	/// @return a current timestamp
    double getsecs(){
    #if PTBGetSecs
        double t;
        PsychGetAdjustedPrecisionTimerSeconds(&t);
        return t;
    #else
        auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
        return (std::chrono::duration_cast<std::chrono::microseconds>(time)).count() *1000000;
    #endif
    }    
        
    public:
    Leap::Controller pcontroller;
        
	public:
    /// @brief constructor
    frame_grabber ()
        : debug (false),
          maxNframes(1000000),
          listening(false)
    {
        latest_frame.id = -1;
        latest_frame.timestamp = -1;
        #if PTBGetSecs
            PsychInitTimeGlue();
        #endif
        // receive frames even when you don't have focus
        pcontroller.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);
    }
        
	/// @brief start listening to frames sent from the Leap controller
    void start_listening(){
        if(!listening){
            pcontroller.addListener(*this);
        }
        listening=true;
    }
    
    /// @brief stop listening to frames sent from the Leap controller
    void stop_listening(){
        if(listening){
            pcontroller.removeListener(*this);
        }
        while(!frames.empty()) {
            frames.pop();
        }
        listening=false;
        latest_frame.id = -1;
        latest_frame.timestamp = -1;
    }
    
    /// @brief check if listening to frames sent from the Leap controller
	///
	/// @return listening status
    bool isListening(){
        return listening;
    }
    
    /// @brief destructor
    ~frame_grabber ()
    {
        if (debug)
            mexPrintf ("Closing matleap frame grabber\n");
        
        stop_listening();
    }
    
    /// @brief debug member access
    ///
    /// @param flag turn it on/off
    void set_debug (bool flag)
    {
        if (flag == debug)
            return;
        if (flag)
            mexPrintf ("Setting debug on\n");
        debug = flag;
    }

    /// @brief callback function for the Leap:listener class
    ///
    /// @param a Leap controller
    virtual void onFrame(const Leap::Controller& controller) { 
        if (debug){
            mexPrintf("received a new frame\n");
        }
        matleap::frame current_frame;
        current_frame.getsecs = getsecs();       
        Leap::Frame f = controller.frame();
               
        current_frame.id = f.id ();
        current_frame.timestamp = f.timestamp ();
        current_frame.pointables = f.pointables ();
        
        frames_mutex.lock();
        
//             //check if we're missing a sample?
//             if(lastsampleId!=-1 && lastsampleId<current_frame.id+1){
//                int missing_samples=current_frame.id-lastsampleId+1;
//                for(int iSample=missing_samples; iSample>0;iSample--){
//                     f = controller.frame(iSample);
//                     int64_t newid=f.id();
//                     if(newid<current_frame.id && newid>lastsampleId){
//                         matleap::frame extraf;
//                         extraf.id=newid;
//                         extraf.timestamp = f.timestamp ();
//                         extraf.getsecs=gs;
//                         extraf.pointables = f.pointables ();
//                         
//                         frames.push(extraf);
//                     }
//                }
//             }                

            //if we have to many samples, we will remove the oldest ones
            while(frames.size()>=maxNframes){
                frames.pop();
            } 
            frames.push(current_frame);
        frames_mutex.unlock();
        
        latest_frame_mutex.lock();
            latest_frame=current_frame;
        latest_frame_mutex.unlock();
    }
    
    /// @brief get a queue of all frames since last call
	///
	/// @return queue of frames (std::queue<frame>)
    std::queue<frame> get_frames() {
        std::queue<frame> ret_frames;
        
        frames_mutex.lock();
            while(!frames.empty()){
                ret_frames.push(frames.front());
                frames.pop();
            }
        frames_mutex.unlock();
        
        return ret_frames;
    }
    
    /// @brief get the latest frame from the controller
	///
	/// @return the frame
    matleap::frame get_latest_frame () {
        frame rf;
        latest_frame_mutex.lock();
            if(!listening || latest_frame.id==-1) {//no sample collected yet
                Leap::Frame f = pcontroller.frame();
                rf.getsecs = getsecs();
                rf.id = f.id ();
                rf.timestamp = f.timestamp ();
                rf.pointables = f.pointables ();
                latest_frame=rf;
            }
            rf=latest_frame;
        latest_frame_mutex.unlock();
        return rf;
    }
};

} // namespace matleap

#endif
