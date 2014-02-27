/****************************************************************************************
Copyright (c) 2014, Chris Stones ( chris.stones_AT_gmail.com / chris.stones_AT_zoho.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/************
 *
 * Tested with GCC 4.7.3
 * compile: g++ -std=c++11 Example.cpp -o Example
 *
 * Simple c++ event mapping library.
 * Objects can throw arbitrary 'less than' comparable event types.
 * Events can be mapped to handlers by EventType, by EventType value,
 * or by an optional condition filter function / lambda.
 *
 * See main() below for a quick example.
 */

#include "EventDispatcher.hpp"

using namespace ED;

/********************************************************
 * Anything 'less than' comparable can be an event type.
 */
class MyEvent
{
public:
	const int i;
	MyEvent(int i) : i(i) {}
	bool operator < ( const MyEvent & that ) const { return this->i < that.i; };
};

/******************************************************************************
 * Class that will raise events should inherit from ED::EventDispatcher<SELF>
 */
class MyEventDispatcher
	:	public EventDispatcher<MyEventDispatcher>
{

public:

	void RaiseSomeEvents() {

		/*************************************************************
		 * Anything 'less than' comparable can be raised as an event
		 */
		// some 'int' events.
		RaiseEvent( 1 );
		RaiseEvent( 2 );
		RaiseEvent( 3 );
		RaiseEvent( 4 );

		// some 'float' events.
		RaiseEvent( 100.5f );
		RaiseEvent( 200.5f );
		RaiseEvent( 300.5f );
		RaiseEvent( 400.5f );

		// our custom event type.
		RaiseEvent( MyEvent(69) );
		RaiseEvent( MyEvent(70) );
		RaiseEvent( MyEvent(71) );
		RaiseEvent( MyEvent(72) );
	}
};

// A function can be an event handler.
void FunctionThatHandlesIntegers( const int & i ) {

	printf("%s %d\n",__FUNCTION__, i);
}

// A member function in a class can also be an event handler.
class Class {
public:
	void MemberFunctionThatHandlesMyEvents(const MyEvent & myEv ) {

		printf("%s %d\n",__FUNCTION__, myEv.i);
	}
}  _class ;

// A lambda event handler
auto lambdaFloatEventHandler =
		[] (const float &f) { printf("LambdaThatHandlesFloats %f\n", f); };


int main() {

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


	/*** Now, raise all out events ***/
	myEventDispathcer->RaiseSomeEvents();

	/***
	 * OUTPUT:
	 *		FunctionThatHandlesIntegers 2
	 *		FunctionThatHandlesIntegers 4
	 *		LambdaThatHandlesFloats 100.500000
	 *		LambdaThatHandlesFloats 200.500000
	 *		LambdaThatHandlesFloats 300.500000
	 *		LambdaThatHandlesFloats 400.500000
	 *		MemberFunctionThatHandlesMyEvents 69
	 *
	 *	Note that only even integer events were raised.
	 *	All float events were raised.
	 *	Of all the 'MyEvents' only 'MyEvent(69)' was raised.
	 */

	return 0;
}

