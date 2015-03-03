
/***
* The direct dispatcher calls raised events handlers immediately.
* If multiple handlers are subscribed to an event, they are called in the order they were subscribed.
* This dispatcher is NOT 'Blockable'.
*/

#include "EventDispatcher.hpp"
#include<memory>
#include<string>
#include<stdio.h>


/******************************************************************************************
  Define some events... Anything that can be compared by the '==' operator can be an event.
*******************************************************************************************/
class MyEvent {
public:
	const std::string myData;
	MyEvent(const std::string myData)
		: myData(myData)
	{}
	bool operator == (const MyEvent &that) const {
		return myData == that.myData;
	}
};

/********************************************************************************************************************************
  Define some event handlers.
    Any callable with a void return type, taking a single const reference to an event type can be a handler for that event type.
*********************************************************************************************************************************/

void HandleAnIntegerFunc(const int & integer) {
	printf("Function \'HandleAnIntegerFunc\' called with %d\n", integer);
}

auto HandleAStringLambda = [](const std::string &string) { printf("function \'HandleAStringLambda\' called with %s\n", string.c_str()); };

class ClassWithMyEventMemberHandler {
public:
	void HandleMyEventMember(const MyEvent &myEvent) {
		printf("Member Function \'HandleMyEventMember\' called with data %s\n", myEvent.myData.c_str() );
	}
};
std::shared_ptr<ClassWithMyEventMemberHandler> classWithMyEventMemberHandler;

class Test 
{
	void LocalHandleFloatMember(const float &f) {

		printf("Local Member Function \'HandleMyEventMember\' called with %f\n", f);
	}

public:


	/********************************************************************************************************************************
	  Subscribe to some events.
	*********************************************************************************************************************************/
	void SetupSubscriptions() {

		// Subscribe function to EVEN integer events.
		functionSubscription = directDispatcher->Subscribe<int>(HandleAnIntegerFunc, [](const int & i){return (i & 1) == 0; });

		// Subscribe lambda to ALL events matching "Hello" string.
		lambdaSubscription = directDispatcher->Subscribe<std::string>(HandleAStringLambda, std::string("Hello") );

		// Subscribe local member function to ALL float events
		//	NOTE that it is okay to capture 'this' as when this object goes out of scope, the subscription destructor will un-subscribe, and delete the dangling pointer.
		localClassSubscription = directDispatcher->Subscribe<float>([this](const float &f){ LocalHandleFloatMember(f); });

		// IF it possible that the handler class could go out of scope WITHOUT its subscription... you need to be carefull that the dispatcher wont be left with a dangleing reference.
		{
			std::weak_ptr<ClassWithMyEventMemberHandler> weak = classWithMyEventMemberHandler;
			externalClassSubscription = directDispatcher->Subscribe<MyEvent>([=](const MyEvent &myEvent) {
				auto shared = weak.lock();
				if (shared) shared->HandleMyEventMember(myEvent);
			});
		}
	}

	/********************************************************************************************************************************
	  Raise some events.
	*********************************************************************************************************************************/
	void RaiseEvents() {

		// Raise some integer events.
		directDispatcher->Raise(67); // NOTE: that out integer handler is bound to EVEN integers only.
		directDispatcher->Raise(68);
		directDispatcher->Raise(69); // NOTE: that out integer handler is bound to EVEN integers only.
		directDispatcher->Raise(70);

		// Raise some float events;
		directDispatcher->Raise(4.20f);

		// Raise some string events.
		directDispatcher->Raise<std::string>("Hello");
		directDispatcher->Raise<std::string>("World!"); /* NOTE: that our string handler is bound to only events matching "Hello". */

		// Raise some of MyEvents.
		directDispatcher->Raise(MyEvent("OneTwoThree"));
		directDispatcher->Raise(MyEvent("FourFiveSix"));

		/*** PROGRAM OUTPUT IS:
		Function 'HandleAnIntegerFunc' called with 68
		Function 'HandleAnIntegerFunc' called with 70
		Local Member Function 'HandleMyEventMember' called with 4.200000
		function 'HandleAStringLambda' called with Hello
		Member Function 'HandleMyEventMember' called with data OneTwoThree
		Member Function 'HandleMyEventMember' called with data FourFiveSix
		*/
	}

	Test()
	{
		// Create a 'direct' event dispatcher.
		directDispatcher = std::make_shared<EventDispatcher::DirectDispatcher>();

		// Instantiate our member function handler.
		classWithMyEventMemberHandler = std::make_shared<ClassWithMyEventMemberHandler>();

		SetupSubscriptions();
	}


private:
	std::shared_ptr<EventDispatcher::DirectDispatcher> directDispatcher;
	std::unique_ptr<EventDispatcher::ISubscription> functionSubscription;
	std::unique_ptr<EventDispatcher::ISubscription> lambdaSubscription;
	std::unique_ptr<EventDispatcher::ISubscription> localClassSubscription;
	std::unique_ptr<EventDispatcher::ISubscription> externalClassSubscription;
};

int main() {

	Test test;
	test.RaiseEvents();

	getchar();
	return 0;

}

