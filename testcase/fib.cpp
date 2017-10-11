typedef long long int64_t;
extern "C" int printf(const char*, ...);

extern "C" int64_t Fibonacci(int64_t n)
{
	if (n < 2) return n;
	return (Fibonacci(n - 2) + Fibonacci(n - 1));
}

extern "C" int64_t g_Result = 0;

int main()
{
	int64_t num = 34;
	// Should return 433494437
	g_Result = Fibonacci(num);

	printf("fib: %lld\n", g_Result);
	return 0;
}
