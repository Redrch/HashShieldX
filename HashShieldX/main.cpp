#include <iostream>
#include "CLI11.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"����һ��С���ߣ����԰���ӽ����ļ����У��ͼ������ļ����У���Hashֵ\n"
		"This is a small tool that can help you encrypt and decrypt files (or folders) and calculate the hash values of multiple files (or folders)"};

	app.set_version_flag("--version", VERSION, "��ʾ�汾��");

	CLI11_PARSE(app, argc, argv);
	return 0;
}