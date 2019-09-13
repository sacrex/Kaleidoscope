//
// "Library" functions that cna be "extern'd from user code"
//

// 在Ch4中我们调用这种库函数时，会出现找不到的函数地址的情况，这就需要修改
// KaleidoscopeJTI.h中的符号查找函数，解决方法如下:
// https://stackoverflow.com/questions/54048268/llvm-kaleidoscope-tutorial-failing-on-local-extern/56862433#56862433

// putchard - putchar thar takes a doubel and return 0.
extern "C" double putchard(double x)
{
	fputc((char)x, stderr);
	return 0;
}

// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" double printd(double x)
{
	fprintf(stderr, "%f\n", x);
	return 0;
}
