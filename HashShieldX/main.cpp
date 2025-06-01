#include <iostream>
#include <string>
#include "CLI11.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"这是一个小工具，可以帮你加解密文件（夹）和计算多个文件（夹）的Hash值"};

	app.set_version_flag("--version", VERSION, "显示版本号");

	// Encrypt Command
	auto* encryptCommand = app.add_subcommand("encrypt", "加密文件");
	// Required
	// Input file name
	string inputFile;
	encryptCommand->add_option("-i, --input", inputFile, "要加密的文件")->required();
	// Output file name
	string outputFile;
	encryptCommand->add_option("-o, --output", outputFile, "输出文件")->required();
	// Key file
	string keyFile;
	encryptCommand->add_option("-k, --key", keyFile, "密钥文件")->required();
	
	// Optional
	// Is force overwrite of the output file
	bool isForce = false;
	encryptCommand->add_flag("-f, --force", isForce, "是否强制覆盖输出文件");

	CLI11_PARSE(app, argc, argv);
	return 0;
}