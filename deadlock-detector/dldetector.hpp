#pragma once

#pragma once

#include <mutex>
#include "tls_template.hpp"

#define DEFINE_WSTR_H(x) L##x
#define DEFINE_WSTR_HELPER(x) DEFINE_WSTR_H(x)

#define DEFINE_STR_H(x) #x
#define DEFINE_STR_HELPER(x) DEFINE_STR_H(x)

#define DEFINE_WFILE DEFINE_WSTR_HELPER(__FILE__)
#define DEFINE_WLINE DEFINE_WSTR_HELPER(DEFINE_STR_HELPER(__LINE__))
#define DEFINE_WFUNCTION DEFINE_WSTR_HELPER(__FUNCTION__)

#define DEFINE_LOCATION_WSTR DEFINE_WFILE L"(" DEFINE_WLINE L") : " DEFINE_WFUNCTION

//#define ORDERED_LOCK_GUARD(x, m) ordered_lock_guard<decltype(m)> x{ m, DEFINE_LOCATION_WSTR }

#define ORDERED_LOCK_GUARDING(m) \
	for (lock_guard_in_for<decltype(m)> for_inner_guard{ m, DEFINE_LOCATION_WSTR }; true == for_inner_guard.is_once; for_inner_guard.is_once = false)


template <typename T>
class ordered_lock
{
private:
	ordered_lock(const ordered_lock&) = delete;
	ordered_lock& operator=(const ordered_lock&) = delete;

public:
	ordered_lock(size_t n) : n_(n)
	{

	}

	void lock()
	{
		l_.lock();
	}

	bool try_lock()
	{
		return l_.try_lock();
	}

	void unlock()
	{
		l_.unlock();
	}

	size_t order() const
	{
		return n_;
	}


private:
	T l_;
	const size_t n_;
};

template <typename T>
class ordered_lock_guard
{
public:
	ordered_lock_guard(T &m, const wchar_t *entry)
		: m_(m), entry_(entry)
	{
		if (false == tls_template<deadlock_detector<T>>::tls_instance().check_lock_order(this))
		{
			tls_template<deadlock_detector<T>>::tls_instance().print_history();
		}
		m_.lock();
	}
	~ordered_lock_guard() noexcept
	{
		tls_template<deadlock_detector<T>>::tls_instance().check_unlock_order(this);
		m_.unlock();
	}

	const wchar_t* entry() const
	{
		return entry_;
	}

	const size_t order() const
	{
		return m_.order();
	}

private:
	T &m_;
	const wchar_t *entry_{ nullptr };
};


template <typename T>
class deadlock_detector
{
public:
	bool check_lock_order(ordered_lock_guard<T> *olg_ptr)
	{
		bool is_order = true;
		if (false == history_.empty())
		{
			// 데드락 위험성 검출
			if (history_.back()->order() <= olg_ptr->order())
			{
				is_order = false;
			}
		}

		history_.push_back(olg_ptr);
		return is_order;
	}

	bool check_unlock_order(ordered_lock_guard<T> *olg_ptr)
	{
		bool is_order = true;
		if (olg_ptr != history_.back())
		{
			is_order = false;
		}

		history_.pop_back();
		return is_order;
	}

	void print_history() const
	{
		std::cout << "---------------------------------------------------------------------------------" << std::endl;
		std::cout << "deadlock detector history" << std::endl;
		std::cout << "---------------------------------------------------------------------------------" << std::endl;

		for (auto *olg_ptr : history_)
		{
			std::wcout << olg_ptr->entry() << std::endl;
		}
		std::cout << "---------------------------------------------------------------------------------" << std::endl;
	}

private:
	std::vector<ordered_lock_guard<T> *> history_;
};

template <typename T>
struct lock_guard_in_for
{
	lock_guard_in_for(T &m, const wchar_t *entry) : guard(m, entry)
	{

	}
	ordered_lock_guard<T> guard;
	bool is_once{ true };
};


typedef ordered_lock<std::mutex> ordered_mutex;

