
#pragma once

#include "TypeManager.hpp"

#include<map>
#include<typeindex>
#include<typeinfo>
#include<memory>

namespace EventDispatcher { namespace detail {

  /*
   * Route events to handlers.
   * Handler is called immediately.
   */
  class DirectDispatcher {

    typedef std::unique_ptr<TypeManagerBase> 	BasePtr;
    typedef std::map<std::type_index, BasePtr> 	TypeBaseMap;
    
    TypeBaseMap typeBaseMap;
    
    // Get the type manager for the given type.
    //	If  one doesn't exist yet, create one.
    template<typename _EventType>
    TypeManager<_EventType> * GetTypeManager() {
     
      std::type_index typeIndex = typeid(_EventType);
      auto it = typeBaseMap.find(typeIndex);
      if(it != std::end(typeBaseMap))
    	  return dynamic_cast<TypeManager<_EventType>*>(it->second.get());
      
      TypeManager<_EventType> * typeManager;
      typeBaseMap[typeIndex] = BasePtr(
    		  typeManager = new TypeManager<_EventType>());

      return typeManager;
    }
    
  public:
    
    template<typename _EventType>
    std::unique_ptr<ISubscription> Subscribe(std::function<void(const _EventType &)> callback, std::function<bool(const _EventType &)> test) {
      
    	return GetTypeManager<_EventType>()->Subscribe(callback, test);
    }
    
    template<typename _EventType>
    std::unique_ptr<ISubscription> Subscribe(std::function<void(const _EventType &)> callback) {
      
    	return Subscribe<_EventType>(callback, [](const _EventType &event){return true;} );
    }
    
    template<typename _EventType>
    std::unique_ptr<ISubscription> Subscribe(std::function<void(const _EventType &)> callback, const _EventType &compareEvent) {
      
    	return Subscribe<_EventType>(callback, [=](const _EventType &event){ return event == compareEvent; } );
    }

    template<typename _EventType>
    void Raise(const _EventType &event) {

    	GetTypeManager<_EventType>()->Raise(event);
    }
  };
}}

