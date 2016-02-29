/// @file matleap.h
/// @brief leap motion controller interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-09-12

#ifndef MATLEAP_H
#define MATLEAP_H

#define MAJOR_REVISION 0
#define MINOR_REVISION 2

#include "Leap.h"
#include "mex.h"
#include <queue>

#include "GetSecs.h"

#if PSYCH_SYSTEM != PSYCH_WINDOWS
#include <sys/time.h>
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

class frame_listener:  public Leap::Listener
{
    private:
    std::queue<frame> frames;
    std::mutex frames_mutex;
    frame latest_frame;
    std::mutex latest_frame_mutex;
    int maxNframes;
    int64_t lastsampleId=-1;
//     bool listening;
    
    public:
    Leap::Controller controller;
    
    frame_listener()
        :maxNframes(1000000)
//          listening(false)
    {
        latest_frame.id = -1;
        latest_frame.timestamp = -1;
        PsychInitTimeGlue();
        // receive frames even when you don't have focus
        controller.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);

    }
        
    /// @brief callback function for the Leap:listener class
    ///
    /// @param a Leap controller
    virtual void onFrame(const Leap::Controller& controller) { 
        //if (debug){
        //    std::cout << "New frame available" << std::endl;
        //}
        matleap::frame current_frame;
//         double gs;
//         double* gsp=&gs;
        PsychGetAdjustedPrecisionTimerSeconds(&(current_frame.getsecs));
//         current_frame.getsecs=gs;
        
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
    
    matleap::frame get_frames() {
        matleap::frame f;
        frames_mutex.lock();
        if(!frames.empty()){
            f = frames.front();
            frames.pop();
        }else{
            f.id=-1;
            f.timestamp=-1;
        }
        frames_mutex.unlock();
        
        return f;
    }
    
    matleap::frame get_latest_frame () {
        frame rf;
        latest_frame_mutex.lock();
            if(latest_frame.id==-1) {//no sample collected yet
                Leap::Frame f = controller.frame();
                PsychGetAdjustedPrecisionTimerSeconds(&(rf.getsecs));
                rf.id = f.id ();
                rf.timestamp = f.timestamp ();
                rf.pointables = f.pointables ();
                latest_frame=rf;
            }
            rf=latest_frame;
        latest_frame_mutex.unlock();
        return rf;
    }
    
//      void start_listening(){
//         if(!listening){
//             controller.addListener(this);
//         }
//         listening=true;
//     }
//     
//     void stop_listening(){
//         if(listening){
//             controller.removeListener(this);
//             
//             //flush the queue
//             frames_mutex.lock();
//             while(!frames.empty()){
//             	frames.pop();
//             }
//             frames_mutex.unlock();
//         }
//         listening=false;
//     }
//     
//     bool isListening(){
//         return listener.isListening();
//     }
};

/// @brief leap frame grabber interface
class frame_grabber
{
    private:
    bool debug;
//     Leap::Controller controller;
    matleap::frame_listener listener;
    bool listening;
    
	public:
    /// @brief constructor
    frame_grabber ()
        : debug (false),
          listening(false)
    {
//         // receive frames even when you don't have focus
//         controller.setPolicyFlags (Leap::Controller::POLICY_BACKGROUND_FRAMES);
    }
        
    void start_listening(){
        if(!listening){
            listener.controller.addListener(listener);
        }
        listening=true;
//         listener.start_listening()
    }
    
    void stop_listening(){
        if(listening){
            listener.controller.removeListener(listener);
            
            //flush the queue
            frame f;
            do{
                f= get_frames();
            }while(f.id!=-1);
            
        }
        listening=false;
//         listener.stop_listening()
    }
    
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
        if (flag)
            mexPrintf ("Setting debug on\n");
        debug = flag;
    }
//     /// @brief get a frame from the controller
//     ///
//     /// @return the frame
//     const frame &get_frame ()
//     {
//         const Leap::Frame &f = controller.frame ();
//         current_frame.id = f.id ();
//         if (debug)
//             mexPrintf ("Got frame with id %d\n", current_frame.id);
//         current_frame.timestamp = f.timestamp ();
//         current_frame.pointables = f.pointables ();
//         return current_frame;
//     }
    
    frame get_frames ()
    {
        return listener.get_frames();
    }
    
    frame get_latest_frame ()
    {
       return listener.get_latest_frame();
    }
};

} // namespace matleap

#endif
