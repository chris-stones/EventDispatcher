EventDispatcher
===============

C++ classes for routing events to event handlers.

EXAMPLE: ( see Example.cpp for more... )


	// This condition filter only raises events for EVEN integers.
	auto isEven = [] (const int &i) -> bool { return !(i & 1); };

	/*** Create an instance of our EventDispatcher ***/
	auto myEventDispathcer = std::make_shared<MyEventDispatcher>()

		/*** Register an event handler (c-function) for EVEN integers only ***/
		->RegisterConditionalEventHandler<int>  ( &FunctionThatHandlesIntegers, isEven )

		/*** Register an event handler (c++ member function) for a specific Event 'MyEvent(69)' ***/
		->RegisterEventHandler        		    ( MyEvent(69), &_class, &Class::MemberFunctionThatHandlesMyEvents  )

		/*** Register an event handler (lambda) for ALL floats ***/
		->RegisterEventTypeHandler<float> 		( lambdaFloatEventHandler );


	/*** Now, raise all our 'int' / 'float' / 'MyEvent' events ***/
	myEventDispathcer->RaiseSomeEvents();

	/***
	 * OUTPUT:
	 *		FunctionThatHandlesIntegers 2		     	// NOTE: only even ints.
	 *		FunctionThatHandlesIntegers 4
	 *		LambdaThatHandlesFloats 100.500000		// NOTE: all floats.
	 *		LambdaThatHandlesFloats 200.500000
	 *		LambdaThatHandlesFloats 300.500000
	 *		LambdaThatHandlesFloats 400.500000
	 *		MemberFunctionThatHandlesMyEvents 69 	// NOTE: only MyEvent '69'
	 *
	 *	Note that only even integer events were raised.
	 *	All float events were raised.
	 *	Of all the 'MyEvents' only 'MyEvent(69)' was raised.
	 */
