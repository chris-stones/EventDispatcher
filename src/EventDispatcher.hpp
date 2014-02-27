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
	template<typename _SequenceType>
	class EventDispatcher : public std::enable_shared_from_this<_SequenceType> {

	private:

		detail::EventDispatcher eventDispatcher;

	protected:

		template<typename _EventType>
		void RaiseEvent( const _EventType & event ) {

			eventDispatcher.Raise( event );
		}

	public:

		template<typename _EventType, typename _Handler, typename _Condition>
		std::shared_ptr<_SequenceType>
		RegisterConditionalEventHandler( const _Handler & handler, const _Condition & condition ) {

			eventDispatcher.Register<_EventType>( handler, condition );

			return this->shared_from_this();
		}

		template<typename _EventType, typename _Thiz, typename _MFunc, typename _Condition>
		std::shared_ptr<_SequenceType>
		RegisterConditionalEventHandler( const _Thiz & thiz, const _MFunc & memb, const _Condition & condition ) {

			return RegisterConditionalEventHandler<_EventType>(
					std::bind( memb, thiz, std::placeholders::_1 ),
					condition);
		}

		template<typename _EventType, typename _Thiz, typename _MFunc, typename _CThiz, typename _CMFunc >
		std::shared_ptr<_SequenceType>
		RegisterConditionalEventHandler( const _Thiz & thiz, const _MFunc & memb, const _CThiz & cthiz, const _CMFunc & cmemb ) {

			return RegisterConditionalEventHandler<_EventType>(
					std::bind(  memb,  thiz, std::placeholders::_1 ),
					std::bind( cmemb, cthiz, std::placeholders::_1 ));
		}

		template<typename _EventType, typename _Handler>
		std::shared_ptr<_SequenceType>
		RegisterEventTypeHandler( const _Handler & handler ) {

			eventDispatcher.Register<_EventType>( handler );

			return this->shared_from_this();
		}

		template<typename _EventType, typename _Thiz, typename _MFunc>
		std::shared_ptr<_SequenceType>
		RegisterEventTypeHandler( const _Thiz & thiz, const _MFunc & memb ) {

			return RegisterEventTypeHandler<_EventType>(
					std::bind( memb, thiz, std::placeholders::_1 ) );
		}

		template<typename _EventType, typename _Handler>
		std::shared_ptr<_SequenceType>
		RegisterEventHandler( const _EventType & event, const _Handler & handler ) {

			eventDispatcher.Register( event, handler );

			return this->shared_from_this();
		}

		template<typename _EventType, typename _Thiz, typename _MFunc>
		std::shared_ptr<_SequenceType>
		RegisterEventHandler( const _EventType & event, const _Thiz & thiz, const _MFunc & memb ) {

			return RegisterEventHandler(
					event,
					std::bind( memb, thiz, std::placeholders::_1 ) );
		}
	};
}

