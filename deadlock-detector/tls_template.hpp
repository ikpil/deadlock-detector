#pragma once

#include <thread>
#include <memory>
#include <unordered_map>

template <typename T>
class tls_template
{
private:
	tls_template(const tls_template&) = delete;
	tls_template& operator=(const tls_template&) = delete;

public:
	~tls_template()
	{
		delete my_tls_ptr_;
		my_tls_ptr_ = nullptr;
	}

	tls_template()
	{

	}

	static void all_tls_release()
	{
		std::lock_guard<std::mutex> guard(m_);
		for (auto& kv : tls_map_)
		{
			delete kv.second;
			kv.second = nullptr;
		}

		tls_map_.clear();
	}

	static void tls_release()
	{
		std::lock_guard<std::mutex> guard(m_);

		delete my_tls_ptr_;
		my_tls_ptr_ = nullptr;

		tls_map_.erase((size_t)&my_tls_ptr_);
	}

	static T& tls_instance()
	{
		if (nullptr == my_tls_ptr_)
		{
			std::lock_guard<std::mutex> guard(m_);

			if (nullptr == my_tls_ptr_)
			{
				my_tls_ptr_ = new my_tls(my_tls_ptr_);
				my_tls_ptr_->context_uptr = std::make_unique<T>();
				tls_map_.insert(std::make_pair((size_t)&my_tls_ptr_, my_tls_ptr_));
			}
		}

		return *(my_tls_ptr_->context_uptr);
	}

private:
	struct my_tls
	{
		my_tls(my_tls *& that_tls_ptr)
			: my_tls_ptr(that_tls_ptr)
		{

		}

		std::unique_ptr<T> context_uptr;
		my_tls *&my_tls_ptr;
	};

	typedef std::unordered_map<size_t, my_tls*> tls_map;

private:
	static std::mutex m_;
	static tls_map tls_map_;

	static thread_local my_tls* my_tls_ptr_;
};

template <typename T> std::mutex tls_template<T>::m_;
template <typename T> typename tls_template<T>::tls_map tls_template<T>::tls_map_;
template <typename T> thread_local typename tls_template<T>::my_tls* tls_template<T>::my_tls_ptr_ = nullptr;

