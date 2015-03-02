
#pragma once

#include "DirectDispatcher.hpp"
#include "BlockableDispatcher.hpp"
#include "PipedDispatcher.hpp"
#include "ScopedBlock.hpp"

namespace EventDispatcher {

	/***
	 * When you subscribe to an event, you are issued an std::unique_ptr<ISubscription>
	 * The event handler is un-subscribed when the unique_ptr is reset, or goes out of scope.
	 */
	using detail::ISubscription;

	/***
	 * The direct dispatcher calls raised events handlers immediately.
	 * If multiple handlers are subscribed to an event, they are called in the order they were subscribed.
	 * This dispatcher is NOT 'Blockable'.
	 */
	using detail::DirectDispatcher;

	/***
	 * The piped dispatcher queues all events.
	 * Handlers are called when 'Flush' is called.
	 * Handlers are called in the order the corresponding events are raised.
	 * If multiple handlers are subscribed to an event, they are called in the order they were described.
	 * This Dispatcher is 'Blockable'.
	 */
	using detail::PipedDispatcher;

	/***
	 * The blockable dispatcher may call handlers immediately, or queue them for later.
	 * Handlers are called in the order the corresponding events are raised.
	 * If multiple handlers are subscribed to an event, they are called in the order they were described.
	 * If this dispatcher is not in a blocked state, and no previously events are pending when an event is raised, then the handler is called immediately.
	 * If the dispatcher is blocked when an event is raised, it is queued for delivery when the state becomes unblocked.
	 * If the dispatcher is not in a blocked state, but previously raised events are pending, then the event is pushed to the back of the pending queue.
	 * 	This is possible if an event handler itself raises an event.
	 * 	IF none of the pending events before this event cause the dispatcher to become blocked, then queued event handler will run immediately.
	 * 	IF a previously raised pending event handler blocks the dispatcher, then this events handler will be called when the dispatcher becomes unblocked.
	 */
	using detail::BlockableDispatcher;

	/***
	 * PipedDispatcher and BlockableDispatcher is 'Blockable'.
	 * The dispatcher is blocked with a call to Block().
	 * Each block must be matched by a matching Unblock().
	 * Blocks are nested. if Block is called 3 times, 3 Unblocks are required to put the dispatcher into an unblocked state.
	 * For Safety, ScopedBlock is provided.
	 * ScopedBlock blocks a blockable dispatcher, and unblocks it when it is destroyed, or goes out of scope.
	 * Scoped block can handle dispatcher objects, pointers to dispatcher objects, or std::shared_ptr's to dispatcher objects.
	 */
	using detail::ScopedBlock;
}

