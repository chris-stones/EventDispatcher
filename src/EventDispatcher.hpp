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

#pragma once

#include "detail.hpp"

namespace ED
{
class Manager;

typedef detail::ISubscritpion ISubscritpion;
typedef std::unique_ptr<ISubscritpion> subscription_t;


class EventSubscriptionManager
{
    friend class Manager;
    detail::EventDispatcher &eventDispatcher;

    EventSubscriptionManager(detail::EventDispatcher &eventDispatcher)
        :	eventDispatcher(eventDispatcher)
    {}

public:

    ////////////////////////////////////// SUBSCRIBE TO EVENTS MEETING A GIVEN CONDITION //////////////////////////////////
    // Foe example, subscribe to all integer events with an even value
    template<typename _EventType, typename _Handler, typename _Condition>
    std::unique_ptr<ISubscritpion>
    SubscribeConditionalEventHandler( const _Handler & handler, const _Condition & condition ) {

        return eventDispatcher.Register<_EventType>( handler, condition );
    }

    template<typename _EventType, typename _Thiz, typename _MFunc, typename _Condition>
    std::unique_ptr<ISubscritpion>
    SubscribeConditionalEventHandler( const _Thiz & thiz, const _MFunc & memb, const _Condition & condition ) {

        return SubscribeConditionalEventHandler<_EventType>(
            std::bind( memb, thiz, std::placeholders::_1 ),
            condition);
    }

    template<typename _EventType, typename _Thiz, typename _MFunc, typename _CThiz, typename _CMFunc >
    std::unique_ptr<ISubscritpion>
    SubscribeConditionalEventHandler( const _Thiz & thiz, const _MFunc & memb, const _CThiz & cthiz, const _CMFunc & cmemb ) {

        return SubscribeConditionalEventHandler<_EventType>(
            std::bind(  memb,  thiz, std::placeholders::_1 ),
            std::bind( cmemb, cthiz, std::placeholders::_1 ));
    }
    
    ////////////////////////////////////////// SUBSCRIBE TO EVENTS BY TYPE ////////////////////////////////////////////////
    // For example, Subscribe to integer events.
    template<typename _EventType, typename _Handler>
    std::unique_ptr<ISubscritpion>
    SubscribeEventTypeHandler( const _Handler & handler ) {

        return eventDispatcher.Register<_EventType>( handler );
    }

    template<typename _EventType, typename _Thiz, typename _MFunc>
    std::unique_ptr<ISubscritpion>
    SubscribeEventTypeHandler( const _Thiz & thiz, const _MFunc & memb ) {

        return SubscribeEventTypeHandler<_EventType>(
            std::bind( memb, thiz, std::placeholders::_1 ) );
    }


    ////////////////////////////////////////// SUBSCRIBE TO EVENTS BY TYPE VALUE ////////////////////////////////////////////
    // For example, Subscribe to integer events with value 101;
    template<typename _EventType, typename _Handler>
    std::unique_ptr<ISubscritpion>
    SubscribeEventHandler( const _EventType & event, const _Handler & handler ) {

        return eventDispatcher.Register( event, handler );
    }

    template<typename _EventType, typename _Thiz, typename _MFunc>
    std::unique_ptr<ISubscritpion>
    SubscribeEventHandler( const _EventType & event, const _Thiz & thiz, const _MFunc & memb ) {

        return SubscribeEventHandler(
            event,
            std::bind( memb, thiz, std::placeholders::_1 ) );
    }
};

class EventPublicationManager  {

    friend class Manager;

    detail::EventDispatcher &eventDispatcher;

    EventPublicationManager(detail::EventDispatcher &eventDispatcher)
        :	eventDispatcher(eventDispatcher)
    {}

public:

    template<typename _EventType>
    void PublishEvent( const _EventType & event ) {

        eventDispatcher.Raise( event );
    }
};

class Manager
{
    detail::EventDispatcher eventDispatcher;

public:
    EventSubscriptionManager SubscriptionInterface()
    {
        return EventSubscriptionManager(eventDispatcher);
    }

    EventPublicationManager PublicationInterface()
    {
        return EventPublicationManager(eventDispatcher);
    }
};
}

