#pragma once

#include "BlockableDispatcher.hpp"

#include<deque>
#include<memory>
#include<exception>

namespace EventDispatcher { namespace detail {

/*
 * Route events to handlers.
 * If the manager is not blocked, handler is called immediately.
 * Otherwise it is queued.
 * All handlers are called in the order their events were raised.
 * If an event has multiple handlers, handlers will be called in the order they were subscribed.
 */
class BlockableDispatcher {

	class IQueuedEvent {
	public:
		virtual void Raise(DirectDispatcher &manager) = 0;
		virtual ~IQueuedEvent() {}
	};

	template<typename _T>
	class QueuedEvent : public IQueuedEvent {
		_T t;
	public:
		QueuedEvent(const _T & t) : t(t) {}
		virtual void Raise(DirectDispatcher &manager) override {
			manager.Raise<_T>(t);
		}
	};

	int blockedCount {0};

	typedef std::unique_ptr<IQueuedEvent> Pointer;
	typedef std::deque<Pointer> Deque;

	DirectDispatcher manager;
	Deque deque;
	bool flushLock{false};

	class FlushScopeLock {
		bool &b;
	public:
		FlushScopeLock(bool &b) : b(b) {
			b = true;
		}
		virtual ~FlushScopeLock() {
			b = false;
		}
	};

	template<typename _T>
	void QueueEvent(const _T & t) {
		deque.push_back( std::make_shared<QueuedEvent<_T> >(t) );
	}

	bool RaiseOneQueuedEvent() {

		auto first = std::begin(deque);
		if(first != std::end(deque)) {
			(*first)->Raise(this->manager);
			deque.erase(first);
			return true;
		}
		return false;
	}

	int Flush() {

		int events = 0;
		if(!flushLock) {
			FlushScopeLock flushScopeLock(flushLock);
			while(!IsBlocked() && RaiseOneQueuedEvent())
				++events;
		}
		return events;
	}

	bool ShouldQueueNewEvent() const {

		return (IsBlocked() || deque.size());
	}

public:

	void Block() {
		blockedCount++;
	}
	void Unblock() {
		if(blockedCount<=0)
			throw std::runtime_error("can't unblock EventDispatcher::QueuedManager");
		blockedCount--;
		Flush();
	}
	bool IsBlocked() const {
		return blockedCount>0;
	}

	template<typename _EventType>
	std::unique_ptr<ISubscription> Subscribe(std::function<void(const _EventType &)> callback, std::function<bool(const _EventType &)> test) {

		return manager.Subscribe<_EventType>(callback, test);
	}

	template<typename _EventType>
	std::unique_ptr<ISubscription> Subscribe(std::function<void(const _EventType &)> callback) {

		return manager.Subscribe<_EventType>(callback);
	}

	template<typename _EventType>
	std::unique_ptr<ISubscription> Subscribe(std::function<void(const _EventType &)> callback, const _EventType &compareEvent) {

		return manager.Subscribe<_EventType>(callback, compareEvent );
	}

	template<typename _EventType>
	void Raise(const _EventType &event) {

		Flush();
		if(ShouldQueueNewEvent())
			QueueEvent(event);
		else
			manager.Raise<_EventType>(event);
	}
};

}}

