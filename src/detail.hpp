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

/* TODO: Are stale references possible in ISubscritpion implementations??? */

#pragma once

#include <typeinfo>
#include <typeindex>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <stdio.h>

namespace ED {

	namespace detail {
	  
		/**************************************************************************
		 * Subscribing to an event returns an ISubscritpion.
		 * 	Deleteing the subscription un-subscribes the event.
		 **************************************************************************/
		class ISubscritpion
		{
		public:
		  virtual ~ISubscritpion(){}
		};
	  

		/**************************************************************************
		 * Internal std::shared_ptr<T> wrapper.
		 * 	Allows unrelated interfaces to be stored in the same keyed container.
		 **************************************************************************/
		class IInstWrapper {
		protected:
			virtual void * member__() = 0;
		public:
			virtual ~IInstWrapper() {}
			template<typename T> T & GetInst() {
				return *reinterpret_cast<T*>(member__());
			}
		};
		template<typename T>
		class InstWrapper
			: public IInstWrapper
		{
			T p;
		protected:
			void * member__() {
				return &p;
			}
		public:
			InstWrapper(const T & p) : p(p) {}
			InstWrapper(           )        {}
			virtual ~InstWrapper() {}
		};

		/**************************************************************************
		 * Internal std::function<R(Args...)> wrapper.
		 * 	Allows unrelated functions to be stored in the same keyed container.
		 **************************************************************************/
		class IFunctorWrapper {
		protected:
			virtual void * member__() = 0;
		public:
			virtual ~IFunctorWrapper() {}
			template<typename T> T &GetFunctor() {
				return *reinterpret_cast<T*>(member__());
			}
		};
		template<typename T>
		class FunctorWrapper
			:	public IFunctorWrapper
		{
			T i;
		protected:
			void * member__() {
				return &i;
			}
		public:
			virtual ~FunctorWrapper() {}
			FunctorWrapper(T & i) : i(i) {}
		};
		/**************************************************************************/


		template<typename _EventType>
		class SpecificEventHandlerLookup {

		public:
			typedef std::vector<IFunctorWrapper*> 		FunctorVector;
			typedef std::map<_EventType, FunctorVector>			FunctorLookup;

		private:
			FunctorLookup functorLookup;

		public:

			virtual ~SpecificEventHandlerLookup() {

				for( auto itor0 : functorLookup )
					for( auto itor1 : itor0.second )
						delete itor1;
			}

			void Raise( const _EventType & event ) {

				auto itor0 = functorLookup.find( event );
				if( itor0 != functorLookup.end())
					for( auto itor1 : itor0->second ) {

						IFunctorWrapper* fw = itor1;
						fw->GetFunctor< std::function<void(const _EventType & )>>()(event);
					}
			}

			template<typename _T>
			class SpecificSubscription : public ISubscritpion
			{
			  FunctorLookup &functorLookup;
			  const _EventType event;
			  FunctorWrapper<_T> * functionWrapper;
			public:
			  SpecificSubscription(FunctorLookup &functorLookup, const _EventType & event, FunctorWrapper<_T> * functionWrapper)
			    :	functorLookup(functorLookup),
				event(event),
				functionWrapper(functionWrapper)
			  {}
			  virtual ~SpecificSubscription() {
			    
			    FunctorVector & functorVector = functorLookup[event];
			    FunctorVector::iterator itor = std::find(functorVector.begin(), functorVector.end(), functionWrapper);
			    if( itor != functorVector.end() ) {
			      delete (*itor);
			      functorVector.erase( itor );
			    }
			  }
			};

			template<typename _Handler>
			std::unique_ptr<detail::ISubscritpion> Register( const _EventType &event, const _Handler & handler ) {

				typedef std::function<void(const _EventType &)> Function;

				Function handler_function = handler;
				
				FunctorVector & functorVector = functorLookup[event];
				
				auto functorWrapper = new FunctorWrapper<Function>(handler_function);

				functorVector.push_back(functorWrapper);
				
				return std::unique_ptr<detail::ISubscritpion>( new SpecificSubscription<Function>(functorLookup, event, functorWrapper) );
			}
		};

		template<typename _EventType>
		class ConditionalEventHandlerLookup {

		public:

			typedef std::function<bool(const _EventType&)> 		 ConditionChecker;
			typedef std::pair<ConditionChecker,IFunctorWrapper*> ConditionalFunctor;

			typedef std::vector<ConditionalFunctor> 			FunctorVector;
			typedef std::map<std::type_index, FunctorVector>	FunctorLookup;

		private:
			FunctorLookup functorLookup;

		public:

			virtual ~ConditionalEventHandlerLookup() {

				for( auto itor0 : functorLookup )
					for( auto itor1 : itor0.second )
						delete itor1.second;
			}

			void Raise( const _EventType & event ) {

				auto itor0 = functorLookup.find( typeid(_EventType) );
				if( itor0 != functorLookup.end())
					for( auto itor1 : itor0->second ) {

						if(itor1.first(event)) {

							IFunctorWrapper* fw = itor1.second;
							fw->GetFunctor< std::function<void(const _EventType & )>>()(event);
						}
					}
			}
			
			class ConditionalSubscription : public ISubscritpion
			{
			  FunctorLookup & functorLookup;
			  ConditionalFunctor 	functionWrapper;
			  std::type_index ti;
			public:
			  ConditionalSubscription(FunctorLookup & functorLookup, std::type_index ti, const ConditionalFunctor & functionWrapper)
			    :	functorLookup(functorLookup),
				ti(ti),
				functionWrapper(functionWrapper)
			  {}
			  virtual ~ConditionalSubscription() {
			    
			    FunctorVector & functorVector = functorLookup[ti];
			    typename FunctorVector::iterator itor = std::find(functorVector.begin(), functorVector.end(), functionWrapper);
			    if( itor != functorVector.end() ) {
			      delete (*itor).second;
			      functorVector.erase( itor );
			    }
			  }
			};

			template<typename _Handler, typename _Condition >
			std::unique_ptr<ISubscritpion> Register( const _Handler & handler, const _Condition & condition ) {

				typedef std::function<void(const _EventType &)> Function;

				Function handler_function = handler;

				ConditionalFunctor cf;

				cf.first  = condition;
				cf.second = new FunctorWrapper<Function>(handler_function);
				
				FunctorVector &functorVector = functorLookup[typeid(_EventType)];

				functorVector.push_back(cf);
				
				return std::unique_ptr<ISubscritpion>( new ConditionalSubscription( functorVector, typeid(_EventType), cf ) );
			}
		};

		class EventDispatcher {

			// General map...
			typedef std::vector<IFunctorWrapper*> 		FunctorVector;
			typedef std::map<std::type_index, FunctorVector >  	GeneralMap;
			GeneralMap generalMap;

			// Special maps... Maps requiring an extra level of indirection!
			typedef std::map< std::type_index, IInstWrapper *> SpecialMap;
			SpecialMap specificMap;
			SpecialMap conditionalMap;

		public:

			virtual ~EventDispatcher() {

				for( auto itor0 : specificMap )
					delete itor0.second;

				for( auto itor0 : conditionalMap )
					delete itor0.second;

				for( auto itor1 : generalMap)
					for( auto itor2 : itor1.second )
						delete itor2;
			}

			template<typename _EventType>
			void Raise( const _EventType & event ) {

				{
					auto itor0 = conditionalMap.find( typeid(event) );
					if(itor0 != conditionalMap.end())
						itor0->second->GetInst<ConditionalEventHandlerLookup<_EventType>>().Raise( event );
				}

				{
					auto itor0 = specificMap.find( typeid(event) );
					if(itor0 != specificMap.end())
						itor0->second->GetInst<SpecificEventHandlerLookup<_EventType>>().Raise( event );
				}

				{
					auto itor0 = generalMap.find( typeid(_EventType) );

					if(itor0 != generalMap.end()) {

						auto v = itor0->second;
						for( auto wrapper : v )
							wrapper->GetFunctor< std::function<void(const _EventType & )>>()(event);
					}
				}
			}

			template<typename _EventType, typename _Handler, typename _Condition>
			std::unique_ptr<detail::ISubscritpion> Register( const _Handler & handler, const _Condition & condition) {

				typedef std::function<void(const _EventType &)> Function;
				Function handler_function = handler;

				auto itor0 = conditionalMap.find( typeid(_EventType) );

				if( itor0 == conditionalMap.end() ) {
					conditionalMap[typeid(_EventType)] =
						new InstWrapper<ConditionalEventHandlerLookup<_EventType>>( );
					itor0 = conditionalMap.find( typeid(_EventType) );
				}
				return itor0->second->GetInst<ConditionalEventHandlerLookup<_EventType>>().Register(handler_function, condition);
			}

			template<typename _EventType, typename _Handler>
			std::unique_ptr<detail::ISubscritpion>  Register( const _EventType & event, const _Handler & handler ) {

				typedef std::function<void(const _EventType &)> Function;
				Function handler_function = handler;

				auto itor0 = specificMap.find( typeid(_EventType) );

				if( itor0 == specificMap.end() ) {
					specificMap[typeid(_EventType)] =
						new InstWrapper<SpecificEventHandlerLookup<_EventType>>( );
					itor0 = specificMap.find( typeid(_EventType) );
				}
				return  itor0->second->GetInst<SpecificEventHandlerLookup<_EventType>>().Register(event, handler_function);
			}
			
			template<typename _T>
			class GeneralSubscription : public ISubscritpion
			{
			  GeneralMap & generalMap;
			  FunctorWrapper<_T> * functionWrapper;
			  std::type_index ti;
			public:
			  GeneralSubscription(GeneralMap & generalMap, std::type_index ti, FunctorWrapper<_T> * functionWrapper)
			    :	generalMap(generalMap),
				ti(ti),
				functionWrapper(functionWrapper)
			  {}
			  virtual ~GeneralSubscription() {
			    
			    FunctorVector &functorVector = generalMap[ti];
			    FunctorVector::iterator itor = std::find(functorVector.begin(), functorVector.end(), functionWrapper);
			    if( itor != functorVector.end() ) {
			      delete (*itor);
			      functorVector.erase( itor );
			    }
			  }
			};

			template<typename _EventType, typename _Handler>
			std::unique_ptr<detail::ISubscritpion> Register( const _Handler & handler ) {

				typedef std::function<void(const _EventType &)> Function;

				Function handler_function = handler;
				
				auto functorWrapper = new FunctorWrapper<Function>(handler_function);
				
				FunctorVector &functorVector = generalMap[typeid(_EventType)];

				functorVector.push_back(functorWrapper);
				
				return std::unique_ptr<detail::ISubscritpion>( new GeneralSubscription<Function>(generalMap, typeid(_EventType), functorWrapper) );
			}
		};
	}
}

