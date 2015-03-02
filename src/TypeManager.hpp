#pragma once

#include "ISubscription.hpp"
#include "TypeManagerBase.hpp"

#include<memory>
#include<list>
#include<functional>
#include<algorithm>

namespace EventDispatcher {
namespace detail {

template<typename _EventType>
class TypeManager: public TypeManagerBase {

	typedef std::function<void(const _EventType &)> CallbackType;
	typedef std::function<bool(const _EventType &)> TestType;

	class ConditionalCallback {
		CallbackType callback;
		TestType test;
	public:
		ConditionalCallback(const CallbackType &callback, const TestType &test) :
				callback(callback), test(test) {
		}
		bool operator()(const _EventType & event) {
			bool r;
			if ((r = test(event)))
				callback(event);
			return r;
		}
	};

	class ConditionalCallbackList {
		typedef std::shared_ptr<ConditionalCallback> Pointer;
		typedef std::list<Pointer> List;
		List list;
		Pointer current;
		bool dirty { false };
	public:

		void Add(Pointer &p) {
			list.push_back(p);
		}

		void Remove(Pointer &p) {

			auto it = std::find(std::begin(list), std::end(list), p);
			if (it != std::end(list)) {
				if (p == current) {
					(*it).reset();
					current.reset();
					dirty = true;
				} else
					list.erase(it);
			}
		}

		void ConditionalRaiseAll(const _EventType &event) {
			for (typename List::iterator itor = std::begin(list);
					itor != std::end(list); itor++) {
				current = *itor;
				if (current)
					(*current)(event);
			}
			current.reset();
			if (dirty) {
				list.erase(
						std::remove(std::begin(list), std::end(list),
								Pointer()), std::end(list));
				dirty = false;
			}
		}
	};

	std::shared_ptr<ConditionalCallbackList> conditionalCallbackList;

	class Subscription: public ISubscription {

		typedef std::shared_ptr<ConditionalCallbackList> List;
		typedef std::shared_ptr<ConditionalCallback> Callback;
		List list;
		Callback callback;
	public:

		Subscription(List &list, Callback &callback) :
				list(list), callback(callback) {
			list->Add(callback);
		}

		virtual ~Subscription() {
			list->Remove(callback);
		}
	};

	std::unique_ptr<ISubscription> Subscribe(
			std::shared_ptr<ConditionalCallback> conditionalCallback) {

		return std::unique_ptr<Subscription>(
				new Subscription(conditionalCallbackList, conditionalCallback));
	}

public:

	TypeManager() {
		conditionalCallbackList = std::make_shared<ConditionalCallbackList>();
	}

	virtual ~TypeManager() {

	}

	std::unique_ptr<ISubscription> Subscribe(CallbackType & callback,
			TestType test) {

		return Subscribe(std::make_shared<ConditionalCallback>(callback, test));
	}

	void Raise(const _EventType &event) {

		conditionalCallbackList->ConditionalRaiseAll(event);
	}
};
}
}

