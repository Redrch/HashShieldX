#include <iostream>
#include <string>
#include "CLI11.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"����һ��С���ߣ����԰���ӽ����ļ����У��ͼ������ļ����У���Hashֵ"};

	app.set_version_flag("--version", VERSION, "��ʾ�汾��");

	// Encrypt Command
	auto* encryptCommand = app.add_subcommand("encrypt", "�����ļ�");
	// Required
	// Input file name
	string inputFile;
	encryptCommand->add_option("-i, --input", inputFile, "Ҫ���ܵ��ļ�")->required();
	// Output file name
	string outputFile;
	encryptCommand->add_option("-o, --output", outputFile, "����ļ�")->required();
	// Key file
	string keyFile;
	encryptCommand->add_option("-k, --key", keyFile, "��Կ�ļ�")->required();
	
	// Optional
	// Is force overwrite of the output file
	bool isForce = false;
	encryptCommand->add_flag("-f, --force", isForce, "�Ƿ�ǿ�Ƹ�������ļ�");

	CLI11_PARSE(app, argc, argv);
	return 0;
}