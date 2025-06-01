#include <iostream>
#include <string>
#include "CLI11.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"这是一个小工具，可以帮你加解密文件（夹）和计算多个文件（夹）的Hash值"};

	app.set_version_flag("--version", VERSION, "显示版本号");

	// Is open debug mode
	bool isDebug = false;
	app.add_flag("-d, --debug", isDebug, "启用调试模式");

	// Encrypt Command
	auto* encryptCommand = app.add_subcommand("encrypt", "加密文件");
	// Required
	// Encrypt input file
	string encryptInputFile;
	encryptCommand->add_option("-i, --input", encryptInputFile, "要加密的文件")->required();
	// Encrypt output file
	string encryptOutputFile;
	encryptCommand->add_option("-o, --output", encryptOutputFile, "输出文件")->required();
	// Encrypt key file
	string encryptKeyFile;
	encryptCommand->add_option("-k, --key", encryptKeyFile, "密钥文件")->required();
	
	// Optional
	// Is force overwrite of the output file
	bool isForce = false;
	encryptCommand->add_flag("-f, --force", isForce, "是否强制覆盖输出文件");

	// Decrypt Command
	auto* decryptCommand = app.add_subcommand("decrypt", "解密文件");
	// Required
	// Decrypt input file
	string decryptInputFile;
	decryptCommand->add_option("-i, --input", decryptInputFile, "要加密的文件")->required();
	// Decrypt output file
	string decryptOutputFile;
	decryptCommand->add_option("-o, --output", decryptOutputFile, "输出文件")->required();
	// Decrypt Key file
	string decryptKeyFile;
	decryptCommand->add_option("-k, --key", decryptKeyFile, "密钥文件")->required();

	// 只需要一个子命令
	app.require_subcommand(1);

	// 解析命令行参数
	try {
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& e) {
		return app.exit(e);
	}

	// 处理命令
	// Encrypt
	if (encryptCommand->parsed())
	{
		if (isDebug)
		{
			cout << "已启用Debug模式" << endl;
		}
		cout << "被加密文件(Input File): " << encryptInputFile << endl;
		cout << "输出文件(Output File): " << encryptOutputFile << endl;
		cout << "密钥文件(Key File): " << encryptKeyFile << endl;
		if (isForce)
		{
			cout << "强制覆盖输出文件" << endl;
		}
		cout << "准备开始加密" << endl;

		// TODO: Encrypt
	}
	// Decrypt
	else if (decryptCommand->parsed())
	{
		if (isDebug)
		{
			cout << "已启用Debug模式" << endl;
		}
		cout << "被解密文件(Input File): " << decryptInputFile << endl;
		cout << "输出文件(Output File): " << decryptOutputFile << endl;
		cout << "密钥文件(Key File): " << decryptKeyFile << endl;
		if (isForce)
		{
			cout << "强制覆盖输出文件" << endl;
		}
		cout << "准备开始解密" << endl;

		// TODO: Decrypt
	}

	return 0;
}