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
	// 重置错误信息
	lastError.clear();

	try {
		// 验证源文件是否存在
		if (!filesystem::exists(sourceFilePath)) {
			lastError = "源文件不存在: " + sourceFilePath;
			return false;
		}

		// 获取文件大小
		size_t fileSize = getFileSize(sourceFilePath);
		if (fileSize == 0) {
			lastError = "文件大小为0或无法获取文件大小";
			return false;
		}

		// 验证分块大小
		if (chunkSizeBytes == 0) {
			lastError = "分块大小不能为0";
			return false;
		}

		// 验证或创建输出目录
		if (!validateOrCreateDirectory(outputDir)) {
			return false;
		}

		// 计算总块数
		size_t totalChunks = (fileSize + chunkSizeBytes - 1) / chunkSizeBytes;
		if (totalChunks == 0) totalChunks = 1;

		// 获取基本文件名（不含路径）
		string baseFileName = filesystem::path(sourceFilePath).filename().string();

		// 打开源文件
		ifstream sourceFile(sourceFilePath, ios::binary);
		if (!sourceFile) {
			lastError = "无法打开源文件: " + sourceFilePath;
			return false;
		}

		// 创建缓冲区
		vector<char> buffer(chunkSizeBytes);

		// 分割处理
		for (size_t chunkIndex = 0; chunkIndex < totalChunks; ++chunkIndex) {
			// 生成输出文件名
			string outputFilePath = outputDir + "/" +
				generateChunkFileName(baseFileName, chunkIndex, totalChunks);

			// 打开输出文件
			ofstream outFile(outputFilePath, ios::binary);
			if (!outFile) {
				lastError = "无法创建分块文件: " + outputFilePath;
				return false;
			}

			// 计算当前块的大小（最后一块可能较小）
			size_t currentChunkSize = (chunkIndex == totalChunks - 1) ?
				(fileSize - chunkIndex * chunkSizeBytes) : chunkSizeBytes;

			// 读取和写入数据
			sourceFile.read(buffer.data(), currentChunkSize);
			outFile.write(buffer.data(), sourceFile.gcount());

			// 检查是否成功
			if (outFile.bad() || sourceFile.bad()) {
				lastError = "写入分块文件时发生错误";
				return false;
			}

			// 更新进度
			updateProgress(chunkIndex + 1, totalChunks, showProgress);
		}

		// 确保显示100%完成
		if (showProgress) {
			updateProgress(totalChunks, totalChunks, true);
			cout << endl;
		}

		return true;
	}
	catch (const exception& e) {
		lastError = string("分割文件时发生异常: ") + e.what();
		return false;
	}
}

bool SplitFile::mergeFiles(
	const vector<string>& chunkFiles,
	const string& outputFilePath,
	bool showProgress)
{
	// 重置错误信息
	lastError.clear();

	try {
		// 检查输入文件列表是否为空
		if (chunkFiles.empty()) {
			lastError = "没有指定要合并的文件";
			return false;
		}

		// 验证输出文件的目录
		string outputDir = filesystem::path(outputFilePath).parent_path().string();
		if (!outputDir.empty() && !validateOrCreateDirectory(outputDir)) {
			return false;
		}

		// 打开输出文件
		ofstream outputFile(outputFilePath, ios::binary);
		if (!outputFile) {
			lastError = "无法创建输出文件: " + outputFilePath;
			return false;
		}

		// 缓冲区大小
		const size_t bufferSize = 4096 * 1024; // 4MB 缓冲区
		vector<char> buffer(bufferSize);

		size_t totalFiles = chunkFiles.size();

		// 合并所有分块
		for (size_t i = 0; i < totalFiles; ++i) {
			const string& chunkPath = chunkFiles[i];

			// 验证分块文件是否存在
			if (!filesystem::exists(chunkPath)) {
				lastError = "分块文件不存在: " + chunkPath;
				return false;
			}

			// 打开分块文件
			ifstream chunkFile(chunkPath, ios::binary);
			if (!chunkFile) {
				lastError = "无法打开分块文件: " + chunkPath;
				return false;
			}

			// 读取并写入数据
			while (chunkFile) {
				chunkFile.read(buffer.data(), buffer.size());
				streamsize bytesRead = chunkFile.gcount();
				if (bytesRead > 0) {
					outputFile.write(buffer.data(), bytesRead);
					if (outputFile.bad()) {
						lastError = "写入输出文件时发生错误";
						return false;
					}
				}
			}

			// 更新进度
			updateProgress(i + 1, totalFiles, showProgress);
		}

		// 确保显示100%完成
		if (showProgress) {
			updateProgress(totalFiles, totalFiles, true);
			cout << endl;
		}

		return true;
	}
	catch (const exception& e) {
		lastError = string("合并文件时发生异常: ") + e.what();
		return false;
	}
}

bool SplitFile::mergeChunks(
	const string& baseChunkPath,
	const string& outputFilePath,
	bool showProgress)
{
	// 获取目录和基本名称
	filesystem::path path(baseChunkPath);
	string directory = path.parent_path().string();
	if (directory.empty()) {
		directory = ".";
	}
	string baseName = path.filename().string();

	// 创建正则表达式以匹配分块文件
	// 格式: basename.partXXX_of_YYY
	regex chunkPattern(baseName + "\\.part(\\d+)_of_(\\d+)");

	// 查找所有匹配的文件
	vector<string> chunkFiles;
	size_t totalChunks = 0;

	try {
		// 遍历目录
		for (const auto& entry : filesystem::directory_iterator(directory)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			string fileName = entry.path().filename().string();
			smatch matches;

			if (regex_match(fileName, matches, chunkPattern) && matches.size() == 3) {
				// 解析块编号和总块数
				size_t chunkIndex = stoul(matches[1].str());
				size_t chunks = stoul(matches[2].str());

				// 更新总块数
				if (totalChunks == 0) {
					totalChunks = chunks;
				}
				else if (totalChunks != chunks) {
					lastError = "分块文件中的总块数不一致";
					return false;
				}

				// 添加到文件列表，并记录块索引
				chunkFiles.push_back(entry.path().string());
			}
		}

		// 检查是否找到所有块
		if (chunkFiles.empty()) {
			lastError = "未找到与基本名称匹配的分块文件";
			return false;
		}

		if (chunkFiles.size() != totalChunks) {
			lastError = "找到的分块文件数量与预期不符";
			return false;
		}

		// 根据块索引排序文件
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



		// 合并所有分块文件
		return mergeFiles(chunkFiles, outputFilePath, showProgress);
	}
	catch (const exception& e) {
		lastError = string("自动合并文件时发生异常: ") + e.what();
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
	// 创建格式: baseFileName.partXXX_of_YYY
	// 索引和总数使用相同的位数，以确保正确排序
	string totalStr = to_string(totalChunks);
	string indexStr = to_string(chunkIndex + 1); // 从1开始

	// 填充前导零
	while (indexStr.length() < totalStr.length()) {
		indexStr = "0" + indexStr;
	}

	return baseFileName + ".part" + indexStr + "_of_" + totalStr;
}

void SplitFile::updateProgress(size_t current, size_t total, bool showProgress)
{
	if (total == 0) return;

	int progressPercent = static_cast<int>((static_cast<double>(current) / total) * 100);

	// 调用回调函数
	if (progressCallback) {
		progressCallback(progressPercent);
	}

	// 显示进度条
	if (showProgress) {
		cout << "\r进度: [";
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
		lastError = string("无法获取文件大小: ") + e.what();
		return 0;
	}
}

bool SplitFile::validateOrCreateDirectory(const string& dirPath)
{
	try {
		// 如果目录不存在，尝试创建
		if (!filesystem::exists(dirPath)) {
			if (!filesystem::create_directories(dirPath)) {
				lastError = "无法创建目录: " + dirPath;
				return false;
			}
		}
		// 检查路径是否是目录
		else if (!filesystem::is_directory(dirPath)) {
			lastError = "指定的路径不是目录: " + dirPath;
			return false;
		}
		return true;
	}
	catch (const exception& e) {
		lastError = string("验证目录时发生异常: ") + e.what();
		return false;
	}
}