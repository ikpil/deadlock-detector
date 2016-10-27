#include <iostream>
#include <string>
#include "dldetector.hpp"

class scope_log_log final
{
public:
	scope_log_log(const scope_log_log&) = delete;
	scope_log_log& operator=(const scope_log_log&) = delete;

public:
	~scope_log_log()
	{
		std::cout << "unlock : " << log_ << std::endl;
	}
	scope_log_log(const std::string& log) : log_(log)
	{
		std::cout << "lock   : " << log_ << std::endl;
	}

private:
	std::string log_;
};

ordered_mutex a(1000);
ordered_mutex b(100);
ordered_mutex c(10);
ordered_mutex d(1);

void test_case_1()
{
	std::cout << "test case 1" << std::endl;
	std::cout << std::endl;

	ORDERED_LOCK_GUARDING(a)
	{
		scope_log_log scope_a("a");
	}
	std::cout << std::endl;
	ORDERED_LOCK_GUARDING(b)
	{
		scope_log_log scope_b("b");
	}
	std::cout << std::endl;
	ORDERED_LOCK_GUARDING(c)
	{
		scope_log_log scope_c("c");
	}
	std::cout << std::endl;
	ORDERED_LOCK_GUARDING(d)
	{
		scope_log_log scope_d("d");
	}
}

void test_case_2()
{
	std::cout << "test case 2" << std::endl;

	std::cout << std::endl;
	// a -> b
	ORDERED_LOCK_GUARDING(a)
	{
		scope_log_log scope_a("a");

		ORDERED_LOCK_GUARDING(b)
		{
			scope_log_log scope_b("b");
		}
	}

	std::cout << std::endl;
	// c -> d
	ORDERED_LOCK_GUARDING(c)
	{
		scope_log_log scope_c("c");

		ORDERED_LOCK_GUARDING(d)
		{
			scope_log_log scope_d("d");
		}
	}

	std::cout << std::endl;
	// a -> d
	ORDERED_LOCK_GUARDING(a)
	{
		scope_log_log scope_a("a");

		ORDERED_LOCK_GUARDING(d)
		{
			scope_log_log scope_d("d");
		}
	}
}

void test_case_3_error()
{
	std::cout << "test case 3" << std::endl;

	// a -> b -> d -> c(error)
	ORDERED_LOCK_GUARDING(a)
	{
		scope_log_log scope_a("a");

		ORDERED_LOCK_GUARDING(b)
		{
			scope_log_log scope_b("b");

			ORDERED_LOCK_GUARDING(d)
			{
				scope_log_log scope_d("d");

				// deadlock - circular wait error
				ORDERED_LOCK_GUARDING(c)
				{
					scope_log_log scope_c("c");
				}
			}
		}
	}

	// b -> a(error)
	ORDERED_LOCK_GUARDING(b)
	{
		scope_log_log scope_b("b");

		// deadlock - circular wait error
		ORDERED_LOCK_GUARDING(a)
		{
			scope_log_log scope_a("a");
		}
	}

	std::cout << std::endl;
}

int main(char argc, char *argv[])
{
	test_case_1();

	std::cout << std::endl;

	test_case_2();

	std::cout << std::endl;

	test_case_3_error();


	// 이 부분 코드 정리해놔야 하는데, 귀찮아서 대충 이렇게 씁니다.
	// 알아서 잘 정리해 주세요.
	// 여기서 잘 정리한다는 것은 all_tls_release()가 호출하지 않아도 메모리 제거를 잘 한다는 뜻입니다.
	tls_template<deadlock_detector<ordered_mutex>>::all_tls_release();

	return 0;
}