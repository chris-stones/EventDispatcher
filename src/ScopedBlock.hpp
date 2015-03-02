
#pragma once

#include<memory>

namespace EventDispatcher { namespace detail {

/***
 * Block a BlockableManager or PipedManager for ScopedBlock's lifetime.
 */
template<typename _T> class ScopedBlock {
	_T &t;
public:
	ScopedBlock(_T &t) : t(t) {
		t.Block();
	}
	~ScopedBlock() {
		t.Unblock();
	}
};

template<typename _T> class ScopedBlock<_T*> {
	_T* t;
public:
	ScopedBlock(_T *t) : t(t) {
		t->Block();
	}
	~ScopedBlock() {
		t->Unblock();
	}
};

template<typename _T> class ScopedBlock<std::shared_ptr<_T> > {
	std::shared_ptr<_T> t;
public:
	ScopedBlock(std::shared_ptr<_T> t) : t(t) {
		t->Block();
	}
	~ScopedBlock() {
		t->Unblock();
	}
};

}}

