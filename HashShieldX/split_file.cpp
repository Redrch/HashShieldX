#include "split_file.h"
#include <iostream>
#include <algorithm>
#include <regex>

SplitFile::SplitFile() : progressCallback(nullptr)
{
}

SplitFile::~SplitFile()
{
}

bool SplitFile::splitFile(
	const string& sourceFilePath,
	const string& outputDir,
	size_t chunkSizeBytes,
	bool showProgress)
{
	// ���ô�����Ϣ
	lastError.clear();

	try {
		// ��֤Դ�ļ��Ƿ����
		if (!filesystem::exists(sourceFilePath)) {
			lastError = "Դ�ļ�������: " + sourceFilePath;
			return false;
		}

		// ��ȡ�ļ���С
		size_t fileSize = getFileSize(sourceFilePath);
		if (fileSize == 0) {
			lastError = "�ļ���СΪ0���޷���ȡ�ļ���С";
			return false;
		}

		// ��֤�ֿ��С
		if (chunkSizeBytes == 0) {
			lastError = "�ֿ��С����Ϊ0";
			return false;
		}

		// ��֤�򴴽����Ŀ¼
		if (!validateOrCreateDirectory(outputDir)) {
			return false;
		}

		// �����ܿ���
		size_t totalChunks = (fileSize + chunkSizeBytes - 1) / chunkSizeBytes;
		if (totalChunks == 0) totalChunks = 1;

		// ��ȡ�����ļ���������·����
		string baseFileName = filesystem::path(sourceFilePath).filename().string();

		// ��Դ�ļ�
		ifstream sourceFile(sourceFilePath, ios::binary);
		if (!sourceFile) {
			lastError = "�޷���Դ�ļ�: " + sourceFilePath;
			return false;
		}

		// ����������
		vector<char> buffer(chunkSizeBytes);

		// �ָ��
		for (size_t chunkIndex = 0; chunkIndex < totalChunks; ++chunkIndex) {
			// ��������ļ���
			string outputFilePath = outputDir + "/" +
				generateChunkFileName(baseFileName, chunkIndex, totalChunks);

			// ������ļ�
			ofstream outFile(outputFilePath, ios::binary);
			if (!outFile) {
				lastError = "�޷������ֿ��ļ�: " + outputFilePath;
				return false;
			}

			// ���㵱ǰ��Ĵ�С�����һ����ܽ�С��
			size_t currentChunkSize = (chunkIndex == totalChunks - 1) ?
				(fileSize - chunkIndex * chunkSizeBytes) : chunkSizeBytes;

			// ��ȡ��д������
			sourceFile.read(buffer.data(), currentChunkSize);
			outFile.write(buffer.data(), sourceFile.gcount());

			// ����Ƿ�ɹ�
			if (outFile.bad() || sourceFile.bad()) {
				lastError = "д��ֿ��ļ�ʱ��������";
				return false;
			}

			// ���½���
			updateProgress(chunkIndex + 1, totalChunks, showProgress);
		}

		// ȷ����ʾ100%���
		if (showProgress) {
			updateProgress(totalChunks, totalChunks, true);
			cout << endl;
		}

		return true;
	}
	catch (const exception& e) {
		lastError = string("�ָ��ļ�ʱ�����쳣: ") + e.what();
		return false;
	}
}

bool SplitFile::mergeFiles(
	const vector<string>& chunkFiles,
	const string& outputFilePath,
	bool showProgress)
{
	// ���ô�����Ϣ
	lastError.clear();

	try {
		// ��������ļ��б��Ƿ�Ϊ��
		if (chunkFiles.empty()) {
			lastError = "û��ָ��Ҫ�ϲ����ļ�";
			return false;
		}

		// ��֤����ļ���Ŀ¼
		string outputDir = filesystem::path(outputFilePath).parent_path().string();
		if (!outputDir.empty() && !validateOrCreateDirectory(outputDir)) {
			return false;
		}

		// ������ļ�
		ofstream outputFile(outputFilePath, ios::binary);
		if (!outputFile) {
			lastError = "�޷���������ļ�: " + outputFilePath;
			return false;
		}

		// ��������С
		const size_t bufferSize = 4096 * 1024; // 4MB ������
		vector<char> buffer(bufferSize);

		size_t totalFiles = chunkFiles.size();

		// �ϲ����зֿ�
		for (size_t i = 0; i < totalFiles; ++i) {
			const string& chunkPath = chunkFiles[i];

			// ��֤�ֿ��ļ��Ƿ����
			if (!filesystem::exists(chunkPath)) {
				lastError = "�ֿ��ļ�������: " + chunkPath;
				return false;
			}

			// �򿪷ֿ��ļ�
			ifstream chunkFile(chunkPath, ios::binary);
			if (!chunkFile) {
				lastError = "�޷��򿪷ֿ��ļ�: " + chunkPath;
				return false;
			}

			// ��ȡ��д������
			while (chunkFile) {
				chunkFile.read(buffer.data(), buffer.size());
				streamsize bytesRead = chunkFile.gcount();
				if (bytesRead > 0) {
					outputFile.write(buffer.data(), bytesRead);
					if (outputFile.bad()) {
						lastError = "д������ļ�ʱ��������";
						return false;
					}
				}
			}

			// ���½���
			updateProgress(i + 1, totalFiles, showProgress);
		}

		// ȷ����ʾ100%���
		if (showProgress) {
			updateProgress(totalFiles, totalFiles, true);
			cout << endl;
		}

		return true;
	}
	catch (const exception& e) {
		lastError = string("�ϲ��ļ�ʱ�����쳣: ") + e.what();
		return false;
	}
}

bool SplitFile::mergeChunks(
	const string& baseChunkPath,
	const string& outputFilePath,
	bool showProgress)
{
	// ��ȡĿ¼�ͻ�������
	filesystem::path path(baseChunkPath);
	string directory = path.parent_path().string();
	if (directory.empty()) {
		directory = ".";
	}
	string baseName = path.filename().string();

	// ����������ʽ��ƥ��ֿ��ļ�
	// ��ʽ: basename.partXXX_of_YYY
	regex chunkPattern(baseName + "\\.part(\\d+)_of_(\\d+)");

	// ��������ƥ����ļ�
	vector<string> chunkFiles;
	size_t totalChunks = 0;

	try {
		// ����Ŀ¼
		for (const auto& entry : filesystem::directory_iterator(directory)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			string fileName = entry.path().filename().string();
			smatch matches;

			if (regex_match(fileName, matches, chunkPattern) && matches.size() == 3) {
				// �������ź��ܿ���
				size_t chunkIndex = stoul(matches[1].str());
				size_t chunks = stoul(matches[2].str());

				// �����ܿ���
				if (totalChunks == 0) {
					totalChunks = chunks;
				}
				else if (totalChunks != chunks) {
					lastError = "�ֿ��ļ��е��ܿ�����һ��";
					return false;
				}

				// ��ӵ��ļ��б�����¼������
				chunkFiles.push_back(entry.path().string());
			}
		}

		// ����Ƿ��ҵ����п�
		if (chunkFiles.empty()) {
			lastError = "δ�ҵ����������ƥ��ķֿ��ļ�";
			return false;
		}

		if (chunkFiles.size() != totalChunks) {
			lastError = "�ҵ��ķֿ��ļ�������Ԥ�ڲ���";
			return false;
		}

		// ���ݿ����������ļ�
		sort(chunkFiles.begin(), chunkFiles.end(),
			[](const string& a, const string& b) {
				regex pattern("\\.part(\\d+)_of_");
				smatch matchesA, matchesB;
				regex_search(a, matchesA, pattern);
				regex_search(b, matchesB, pattern);

				if (matchesA.size() > 1 && matchesB.size() > 1) {
					return stoul(matchesA[1].str()) < stoul(matchesB[1].str());
				}
				return a < b;
			});



		// �ϲ����зֿ��ļ�
		return mergeFiles(chunkFiles, outputFilePath, showProgress);
	}
	catch (const exception& e) {
		lastError = string("�Զ��ϲ��ļ�ʱ�����쳣: ") + e.what();
		return false;
	}
}

void SplitFile::setProgressCallback(function<void(int)> callback)
{
	progressCallback = callback;
}

string SplitFile::getLastError() const
{
	return lastError;
}

size_t SplitFile::estimateChunkCount(const string& filePath, size_t chunkSizeBytes)
{
	if (chunkSizeBytes == 0) {
		return 0;
	}

	try {
		size_t fileSize = getFileSize(filePath);
		return (fileSize + chunkSizeBytes - 1) / chunkSizeBytes;
	}
	catch (...) {
		return 0;
	}
}

string SplitFile::generateChunkFileName(
	const string& baseFileName,
	size_t chunkIndex,
	size_t totalChunks)
{
	// ������ʽ: baseFileName.partXXX_of_YYY
	// ����������ʹ����ͬ��λ������ȷ����ȷ����
	string totalStr = to_string(totalChunks);
	string indexStr = to_string(chunkIndex + 1); // ��1��ʼ

	// ���ǰ����
	while (indexStr.length() < totalStr.length()) {
		indexStr = "0" + indexStr;
	}

	return baseFileName + ".part" + indexStr + "_of_" + totalStr;
}

void SplitFile::updateProgress(size_t current, size_t total, bool showProgress)
{
	if (total == 0) return;

	int progressPercent = static_cast<int>((static_cast<double>(current) / total) * 100);

	// ���ûص�����
	if (progressCallback) {
		progressCallback(progressPercent);
	}

	// ��ʾ������
	if (showProgress) {
		cout << "\r����: [";
		int barWidth = 50;
		int pos = barWidth * progressPercent / 100;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) cout << "=";
			else if (i == pos) cout << ">";
			else cout << " ";
		}
		cout << "] " << progressPercent << "%" << flush;
	}
}

size_t SplitFile::getFileSize(const string& filePath)
{
	try {
		return static_cast<size_t>(filesystem::file_size(filePath));
	}
	catch (const exception& e) {
		lastError = string("�޷���ȡ�ļ���С: ") + e.what();
		return 0;
	}
}

bool SplitFile::validateOrCreateDirectory(const string& dirPath)
{
	try {
		// ���Ŀ¼�����ڣ����Դ���
		if (!filesystem::exists(dirPath)) {
			if (!filesystem::create_directories(dirPath)) {
				lastError = "�޷�����Ŀ¼: " + dirPath;
				return false;
			}
		}
		// ���·���Ƿ���Ŀ¼
		else if (!filesystem::is_directory(dirPath)) {
			lastError = "ָ����·������Ŀ¼: " + dirPath;
			return false;
		}
		return true;
	}
	catch (const exception& e) {
		lastError = string("��֤Ŀ¼ʱ�����쳣: ") + e.what();
		return false;
	}
}