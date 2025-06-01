#include <iostream>
#include <string>
#include "CLI11.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"����һ��С���ߣ����԰���ӽ����ļ����У��ͼ������ļ����У���Hashֵ"};

	app.set_version_flag("--version", VERSION, "��ʾ�汾��");

	// Is open debug mode
	bool isDebug = false;
	app.add_flag("-d, --debug", isDebug, "���õ���ģʽ");

	// Encrypt Command
	auto* encryptCommand = app.add_subcommand("encrypt", "�����ļ�");
	// Required
	// Encrypt input file
	string encryptInputFile;
	encryptCommand->add_option("-i, --input", encryptInputFile, "Ҫ���ܵ��ļ�")->required();
	// Encrypt output file
	string encryptOutputFile;
	encryptCommand->add_option("-o, --output", encryptOutputFile, "����ļ�")->required();
	// Encrypt key file
	string encryptKeyFile;
	encryptCommand->add_option("-k, --key", encryptKeyFile, "��Կ�ļ�")->required();
	
	// Optional
	// Is force overwrite of the output file
	bool isForce = false;
	encryptCommand->add_flag("-f, --force", isForce, "�Ƿ�ǿ�Ƹ�������ļ�");

	// Decrypt Command
	auto* decryptCommand = app.add_subcommand("decrypt", "�����ļ�");
	// Required
	// Decrypt input file
	string decryptInputFile;
	decryptCommand->add_option("-i, --input", decryptInputFile, "Ҫ���ܵ��ļ�")->required();
	// Decrypt output file
	string decryptOutputFile;
	decryptCommand->add_option("-o, --output", decryptOutputFile, "����ļ�")->required();
	// Decrypt Key file
	string decryptKeyFile;
	decryptCommand->add_option("-k, --key", decryptKeyFile, "��Կ�ļ�")->required();

	// ֻ��Ҫһ��������
	app.require_subcommand(1);

	// ���������в���
	try {
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& e) {
		return app.exit(e);
	}

	// ��������
	// Encrypt
	if (encryptCommand->parsed())
	{
		if (isDebug)
		{
			cout << "������Debugģʽ" << endl;
		}
		cout << "�������ļ�(Input File): " << encryptInputFile << endl;
		cout << "����ļ�(Output File): " << encryptOutputFile << endl;
		cout << "��Կ�ļ�(Key File): " << encryptKeyFile << endl;
		if (isForce)
		{
			cout << "ǿ�Ƹ�������ļ�" << endl;
		}
		cout << "׼����ʼ����" << endl;

		// TODO: Encrypt
	}
	// Decrypt
	else if (decryptCommand->parsed())
	{
		if (isDebug)
		{
			cout << "������Debugģʽ" << endl;
		}
		cout << "�������ļ�(Input File): " << decryptInputFile << endl;
		cout << "����ļ�(Output File): " << decryptOutputFile << endl;
		cout << "��Կ�ļ�(Key File): " << decryptKeyFile << endl;
		if (isForce)
		{
			cout << "ǿ�Ƹ�������ļ�" << endl;
		}
		cout << "׼����ʼ����" << endl;

		// TODO: Decrypt
	}

	return 0;
}