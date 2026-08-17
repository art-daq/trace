#ifndef SNB_COROUTINE_TEST_HXX
#define SNB_COROUTINE_TEST_HXX

#include <iostream>
#include <exception>
#include <optional>
#include <atomic>
#include "TRACE/trace.h"

#if __cplusplus >= 202002L
#	include <coroutine>

// ---------------------------------------------------------------------------
// Generator<T> — a simple coroutine return type
// ---------------------------------------------------------------------------
template<typename T>
struct Generator {
	struct promise_type {
		T current_value;
		std::exception_ptr exception;

		Generator get_return_object() { return Generator{handle_type::from_promise(*this)}; }

		std::suspend_always yield_value(T value)
		{
			current_value= value;
			return {};
		}
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }

		void return_void() {}
		void unhandled_exception() { exception= std::current_exception(); }
	};

	using handle_type= std::coroutine_handle<promise_type>;

	handle_type coro;

	explicit Generator(handle_type h) : coro(h) {}
	~Generator()
	{
		if (coro) coro.destroy();
	}

	// Prevent copying to avoid double-freeing the coroutine frame
	Generator(const Generator&)= delete;
	Generator& operator=(const Generator&)= delete;

	// Allow moving
	Generator(Generator&& other) noexcept : coro(other.coro) { other.coro= nullptr; }
	Generator& operator=(Generator&& other) noexcept
	{
		if (this != &other) {
			if (coro) coro.destroy();
			coro= other.coro;
			other.coro= nullptr;
		}
		return *this;
	}

	// Fetches the next value from the coroutine.
	bool move_next()
	{
		if (!coro || coro.done()) return false;
		coro.resume();
		return !coro.done();
	}

	T current_value() { return coro.promise().current_value; }
};

// ---------------------------------------------------------------------------
// Template class with a coroutine member function.
// ---------------------------------------------------------------------------
template<typename T>
class EvenNumberProducer {
	T m_start;
	T m_end;

public:
	EvenNumberProducer(T start, T end) : m_start(start), m_end(end) {}

	Generator<T> generate();
};

template<typename T>
Generator<T> EvenNumberProducer<T>::generate()
{
	int buffer[10000];  // Example buffer to demonstrate local variables in coroutine
	for (int i= 0; i < 10000; ++i) { buffer[i]= i; }
	TLOG() << "Buffer address: " << static_cast<void*>(buffer) << " buffer[0]: " << buffer[0] << " buffer[9999]: " << buffer[9999];
	TLOG() << "generate() coroutine starting";
	T current= (m_start % 2 == 0) ? m_start : m_start + 1;
	for (; current <= m_end; current+= 2) {
		TLOG() << "about to yield: " << current;
		co_yield current;
		TLOG() << "resumed after yield of: " << current;
	}
	TLOG() << "generate() coroutine ending";
}

#else
#	warning "coroutine support only in C++ 20 and beyond. Building without coroutine functionality."
#endif

#endif  // SNB_COROUTINE_TEST_HXX
