#include <iostream>
#include <cstdlib>

extern "C" {
	double Exp(double);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		exit(-1);

	for (int i = 0; i < atoi(argv[1]); ++i)
		std::cout << "exp of "<< i << ": " << Exp(i) << std::endl;
}
