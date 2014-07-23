
#include "EventDispatcher.hpp"
#include <stdio.h>


// Define a base class for objects that will publish events.
class PublisherBase
{
protected:
  static ED::EventPublicationManager globalPublisher;
};

// Define a base class for objects that will subscribe to events.
class SubscriberBase
{
protected:
  static ED::EventSubscriptionManager globalSubscriber;
  static ED::EventScheduleManager globalScheduler;	// The delivery of events can be suspended / resumed.
};

// Instantiate the event manager, and distribute interfaces to static base members.
static ED::Manager globalEventManager;
ED::EventPublicationManager 	PublisherBase::globalPublisher 		= globalEventManager.PublicationInterface();
ED::EventSubscriptionManager	SubscriberBase::globalSubscriber 	= globalEventManager.SubscriptionInterface();
ED::EventScheduleManager	SubscriberBase::globalScheduler 	= globalEventManager.SchedulerInterface();

// Define some events.
// Anything comparable with the '<' operator can be an event.
enum EventType0 { EventA=1, EventB=2 };
enum EventType1 { EventC=3, EventD=4 };
enum EventType2 { EventE=5, EventF=6 };

// Define a class that will pulish some events.
class MyPublisher : public PublisherBase
{
public:
  
  void Publish()
  {
    globalPublisher.PublishEvent(EventA);
    globalPublisher.PublishEvent(EventB);
    globalPublisher.PublishEvent(EventC);
    globalPublisher.PublishEvent(EventD);
    globalPublisher.PublishEvent(EventE);
    globalPublisher.PublishEvent(EventF);
  }
};

// Define a class that will subscribe to some events.
class MySubscriber : public SubscriberBase
{
  ED::subscription_t subscriptionToEventType0;
  ED::subscription_t subscriptionToEventType1;
  ED::subscription_t subscriptionToEventType2;
  
public:
  MySubscriber()
  {
    
  }
  
  void SubscribeAll()
  {
    // Your object is un-subscribed from an event when the returned subscription goes out of scope, or is reset. 
    //	' subscriptionToEventType0.reset() '
    subscriptionToEventType0 = globalSubscriber.SubscribeEventTypeHandler<EventType0>( this, &MySubscriber::OnEventType0 );
    subscriptionToEventType1 = globalSubscriber.SubscribeEventTypeHandler<EventType1>( this, &MySubscriber::OnEventType1 );
    subscriptionToEventType2 = globalSubscriber.SubscribeEventTypeHandler<EventType2>( this, &MySubscriber::OnEventType2 );
  }
  
  void QueueSome()
  {
    // Any events raised when they have been marked as queued in the scheduler are stored for later delivery.
    //	The scheduler is a stack of states.
    //	The event will queued is any level of the stack marks it as queued.
    globalScheduler.Push();
    globalScheduler.Queue<EventType0,EventType1>();
  }
  
  void Unqueue()
  {
    // After 'possibly' allowing more events to be raised by calling globalScheduler.Pop() or globalScheduler.Publish<...>()
    //	You need to call Flush() to re-process the queued events.
    globalScheduler.Pop();
    globalScheduler.FlushQueue();
  }
  
  void OnEventType0( EventType0 event ) { printf("EventType0 ( %d )\n", event);  }
  void OnEventType1( EventType1 event ) { printf("EventType1 ( %d )\n", event);  }
  void OnEventType2( EventType2 event ) { printf("EventType2 ( %d )\n", event);  }
};

int main(int argc, char **argv) {
  
  MySubscriber subscriber;
  MyPublisher publisher;
  
  subscriber.SubscribeAll();
  subscriber.QueueSome();
  publisher.Publish();
  subscriber.Unqueue();
  
  // Events are raised in the order 1,2,3,4,5,6.
  // But due to the actions of the scheduler, the program outputs 5,6,1,2,3,4.
  
  return 0;
}


