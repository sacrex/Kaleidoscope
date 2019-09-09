//
// "Library" functions that cna be "extern'd from user code"
//

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
