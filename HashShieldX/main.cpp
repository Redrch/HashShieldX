#include <iostream>
#include "CLI11.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"这是一个小工具，可以帮你加解密文件（夹）和计算多个文件（夹）的Hash值\n"
		"This is a small tool that can help you encrypt and decrypt files (or folders) and calculate the hash values of multiple files (or folders)"};

	app.set_version_flag("--version", VERSION, "显示版本号");

	CLI11_PARSE(app, argc, argv);
	return 0;
}