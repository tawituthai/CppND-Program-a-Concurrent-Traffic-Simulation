#include <iostream>
#include <random>
#include "TrafficLight.h"

// Forward declaration for helper function
double getRandom(double min, double max);

/* Implementation of class "MessageQueue" */

TrafficLightPhase MessageQueue::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(this->_mutex);
    this->_cond.wait(uLock, [this](){ return !this->_TLPqueue.empty(); });

    // Once TLPqueue is not empthy
    TrafficLightPhase tlp = std::move(_TLPqueue.back());
    _TLPqueue.pop_back();

    return tlp;
}

void MessageQueue::send(TrafficLightPhase &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    // std::lock_guard<std::mutex> uLock(this->_mutex);

    // add vector to queue
    this->_TLPqueue.push_front(std::move(msg));
    this->_cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        TrafficLightPhase tlp = _TLqueue.receive();
        if (tlp == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    // init stop watch
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    double cycleDuration = getRandom(4.0, 6.0);
    lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // Lock before access to private veriables
            std::lock_guard<std::mutex> uLock(_mutex);
            // Toggle TrafficLight Phase
            if (this->getCurrentPhase() == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }

            // Send message to intersection::addVehicleToQueue()
            _TLqueue.send(std::move(_currentPhase));

            // Random for the next cycle duration
            cycleDuration = getRandom(4.0, 6.0);
            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
        }
    }
}

double getRandom(double min, double max){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distr(min, max);
    
    return double(distr(gen));
}