/**
 * @file BmpImage.cpp
 * @brief BMP图像文件的加载与保存实现
 */

#include "BmpImage.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>

using namespace std;

/**
 * @brief 从文件加载BMP图像
 * @param filename 要加载的BMP文件路径
 * @return 加载成功返回true，失败返回false
 */
bool BmpImage::load(const std::string& filename)
{
	ifstream fin(filename, ios::binary);
	if (!fin.is_open()) {
		cerr << "[错误] 无法打开文件: " << filename << endl;
		return false;
	}

	// 1) 读取文件头
	fin.read(reinterpret_cast<char*>(&m_fileHeader), sizeof(BmpFileHeader));
	if (!fin) { // 检查读取文件头是否成功
		cerr << "[错误] 读取BMP文件头失败: " << filename << endl;
		fin.close(); return false;
	}
	// 检查BMP标识 'BM'
	if (m_fileHeader.bfType != 0x4D42) {
		cerr << "[错误] 无效的BMP文件(bfType != 'BM'): " << filename << endl;
		fin.close();
		return false;
	}

	// 2) 读取信息头 (基准40字节)
	fin.read(reinterpret_cast<char*>(&m_infoHeader), sizeof(BmpInfoHeader));
	if (!fin) { // 检查读取信息头是否成功
		cerr << "[错误] 读取BMP信息头失败: " << filename << endl;
		fin.close(); return false;
	}

	// 处理可能的额外头/调色板
	size_t actualDataOffset = m_fileHeader.bfOffBits; // 像素数据实际开始位置
	streamoff currentPos = fin.tellg();
	streamoff expectedHeaderSize = sizeof(BmpFileHeader) + m_infoHeader.biSize; // 理论上头的大小和

	// 检查 bfOffBits 相对于当前读取位置和理论头大小的合理性
	if (actualDataOffset < currentPos) {
		// bfOffBits 指向已读过的头内部或之前，文件结构可能错误
		cerr << "[警告] bfOffBits (" << actualDataOffset << ") 指向已读取的头部内部或之前 (当前位置: " << currentPos << ") 文件: " << filename << "。尝试回退。" << endl;
		fin.seekg(actualDataOffset, ios::beg);
		if (!fin) {
			cerr << "[错误] 无法回退到头部指定的bfOffBits位置。无法加载像素数据。" << endl;
			fin.close(); return false;
		}
		currentPos = fin.tellg(); // 更新当前位置
		// 如果seek后仍然不对，下面读取extra header的逻辑也可能出错
	}

	if (actualDataOffset > (size_t)currentPos) {
		size_t extraSizeToRead = actualDataOffset - (size_t)currentPos;
		m_extraHeader.resize(extraSizeToRead);
		// 从当前位置读取直到像素数据开始前的所有内容
		// cout << "[调试] 正在读取 " << extraSizeToRead << " 字节的额外头部/调色板数据。" << endl;
		fin.read(reinterpret_cast<char*>(m_extraHeader.data()), extraSizeToRead);
		if (!fin || (size_t)fin.gcount() != extraSizeToRead) {
			cerr << "[错误] 读取额外头部/调色板数据失败 (预期 " << extraSizeToRead << " 字节) 文件: " << filename << "。实际读取 " << fin.gcount() << " 字节。" << endl;
			fin.close(); return false;
		}
	}
	// 如果 actualDataOffset == currentPos，则没有额外数据需要读取

	// 3) 读取像素数据
	size_t pixelDataSize = 0;
	if (m_fileHeader.bfSize > actualDataOffset) { // 使用实际偏移量
		pixelDataSize = m_fileHeader.bfSize - actualDataOffset;
	}

	if (pixelDataSize == 0 && m_infoHeader.biSizeImage > 0) {
		pixelDataSize = m_infoHeader.biSizeImage;
		cerr << "[警告] 从bfSize计算的像素数据大小为零/负值，改用biSizeImage (" << pixelDataSize << " 字节) 文件: " << filename << endl;
	}
	else if (m_fileHeader.bfSize <= actualDataOffset && m_infoHeader.biSizeImage == 0) {
		cerr << "[错误] BMP头部指示无效的像素数据大小 (bfSize <= bfOffBits 且 biSizeImage 为 0) 文件: " << filename << endl;
		fin.close();
		return false; // 无法确定数据大小
	}

	// 定位到像素数据开始处 (再次确认)
	fin.seekg(actualDataOffset, ios::beg);
	if (!fin) {
		cerr << "[错误] 无法定位到像素数据偏移位置 (bfOffBits = " << actualDataOffset << ") 文件: " << filename << endl;
		fin.close();
		return false;
	}


	m_pixelData.resize(pixelDataSize);
	if (pixelDataSize > 0) {
		// cout << "[调试] 尝试读取 " << pixelDataSize << " 字节的像素数据。" << endl; // 添加Debug信息
		fin.read(reinterpret_cast<char*>(m_pixelData.data()), pixelDataSize);
		// 检查读取操作是否成功，并且是否读取了期望的字节数
		if (!fin || (size_t)fin.gcount() != pixelDataSize) {
			cerr << "[错误] 读取预期的像素数据失败 (" << pixelDataSize << " 字节) 文件: " << filename << "。实际读取 " << fin.gcount() << " 字节。" << endl;
			fin.close();
			m_pixelData.clear(); // 读取不完整的数据是无效的
			return false;
		}
	}
	else {
		cerr << "[警告] 像素数据大小为零，文件: " << filename << "。" << endl;
	}


	fin.close();
	return true;
}

/**
 * @brief 将BMP图像保存到文件
 * @param filename 要保存的文件路径
 * @return 保存成功返回true，失败返回false
 */
bool BmpImage::save(const std::string& filename) const
{
	ofstream fout(filename, ios::binary | ios::trunc); // 添加 trunc 确保覆盖旧文件
	if (!fout.is_open()) {
		cerr << "[错误] 无法创建文件进行保存: " << filename << endl;
		return false;
	}

	// --- 准备要写入的头信息 ---
	BmpFileHeader fileHeader = m_fileHeader;
	BmpInfoHeader infoHeader = m_infoHeader;

	// 1. 确认数据偏移量 (bfOffBits) - 通常使用加载时的值
	uint32_t actualOffBits = m_fileHeader.bfOffBits; // 保留加载时的偏移量

	// (可选) 校验bfOffBits是否与头大小+额外头大小匹配
	uint32_t calculatedOffBits = sizeof(BmpFileHeader) + infoHeader.biSize + static_cast<uint32_t>(m_extraHeader.size());
	if (actualOffBits != calculatedOffBits) {
		cerr << "[警告] 头部中的bfOffBits (" << actualOffBits << ") 与基于biSize+extraHeader计算的值 (" << calculatedOffBits << ") 不匹配。使用头部值。" << endl;
	}
	fileHeader.bfOffBits = actualOffBits; // 确保写入的是这个值

	// 2. 计算文件总大小 (bfSize)
	uint32_t actualPixelDataSize = static_cast<uint32_t>(m_pixelData.size());
	fileHeader.bfSize = actualOffBits + actualPixelDataSize;

	// 3. (可选) 更新 biSizeImage
	if (infoHeader.biSizeImage != 0 && infoHeader.biSizeImage != actualPixelDataSize) {
		cerr << "[警告] 更新biSizeImage，从 " << infoHeader.biSizeImage << " 到 " << actualPixelDataSize << "。" << endl;
		infoHeader.biSizeImage = actualPixelDataSize;
	}
	else if (infoHeader.biSizeImage == 0 && infoHeader.biCompression == 0) {
		// 考虑为未压缩图像填充 biSizeImage
		infoHeader.biSizeImage = actualPixelDataSize;
	}

	// --- [调试] 打印将要写入的信息 ---
	// cout << "[调试] 保存BMP:" << endl;
	// cout << "  bfOffBits: " << fileHeader.bfOffBits << endl;
	// cout << "  PixelDataSize: " << actualPixelDataSize << endl;
	// cout << "  Calculated bfSize: " << fileHeader.bfSize << endl;
	// cout << "  biSizeImage: " << infoHeader.biSizeImage << endl;
	// --- 调试信息结束 ---

	// 1) 写入文件头
	fout.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BmpFileHeader));
	if (!fout) { cerr << "[错误] 写入文件头失败: " << filename << endl; fout.close(); return false; }

	// 2) 写入信息头 (写入biSize指定的字节数，但我们只有标准部分在infoHeader)
	if (infoHeader.biSize >= sizeof(BmpInfoHeader)) {
		fout.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BmpInfoHeader));
		if (!fout) { cerr << "[错误] 写入标准信息头失败: " << filename << endl; fout.close(); return false; }

		// 如果 biSize > 40，假设剩余部分在 m_extraHeader 的开头
		if (infoHeader.biSize > sizeof(BmpInfoHeader)) {
			size_t remainingHeaderSize = infoHeader.biSize - sizeof(BmpInfoHeader);
			if (remainingHeaderSize <= m_extraHeader.size()) {
				// cout << "[调试] 正在从m_extraHeader写入 " << remainingHeaderSize << " 字节的扩展信息头。" << endl;
				fout.write(reinterpret_cast<const char*>(m_extraHeader.data()), remainingHeaderSize);
				if (!fout) { cerr << "[错误] 写入扩展信息头失败: " << filename << endl; fout.close(); return false; }
			}
			else {
				cerr << "[错误] biSize指示更大的头部，但m_extraHeader太小！" << endl;
				fout.close(); return false; // 不一致，无法安全保存
			}
		}
	}
	else {
		cerr << "[错误] 信息头中的biSize无效 (" << infoHeader.biSize << ")。" << endl;
		fout.close(); return false;
	}

	// 3) 写入剩余的额外头/调色板数据 (如果 biSize > 40，这应该是 m_extraHeader 中扩展头之后的部分)
	size_t extraHeaderOffset = (infoHeader.biSize > sizeof(BmpInfoHeader)) ? (infoHeader.biSize - sizeof(BmpInfoHeader)) : 0;
	if (extraHeaderOffset < m_extraHeader.size()) {
		size_t paletteSize = m_extraHeader.size() - extraHeaderOffset;
		// cout << "[调试] 正在从m_extraHeader写入 " << paletteSize << " 字节的调色板数据。" << endl;
		fout.write(reinterpret_cast<const char*>(m_extraHeader.data() + extraHeaderOffset), paletteSize);
		if (!fout) { cerr << "[错误] 写入调色板数据失败: " << filename << endl; fout.close(); return false; }
	}

	// 检查写入指针是否在预期的 bfOffBits
	streampos currentWritePos = fout.tellp();
	if ((uint32_t)currentWritePos != fileHeader.bfOffBits) {
		cerr << "[警告] 文件指针 (" << currentWritePos
			<< ") 不在写入像素前的预期数据偏移位置 (bfOffBits=" << fileHeader.bfOffBits
			<< ")。头部大小可能错误或写入失败。" << endl;
		// 强制定位可能掩盖问题，但可以尝试
		// fout.seekp(fileHeader.bfOffBits, ios::beg);
	}


	// 4) 写入像素数据
	if (!m_pixelData.empty()) {
		// cout << "[调试] 正在写入 " << m_pixelData.size() << " 字节的像素数据。" << endl; // 调试
		fout.write(reinterpret_cast<const char*>(m_pixelData.data()), m_pixelData.size());
		// --- [调试] 检查写入操作后的流状态 ---
		if (!fout) {
			cerr << "[错误] 写入像素数据后立即发生流错误: " << filename << endl;
			// 可以尝试获取更详细的错误信息，但这依赖于具体实现
			fout.close();
			return false;
		}
	}

	// --- [调试] 尝试强制刷新缓冲区 ---
	// cout << "[调试] 刷新输出流..." << endl;
	fout.flush();
	if (!fout) {
		cerr << "[错误] 刷新后发生流错误: " << filename << endl;
		fout.close();
		return false;
	}

	// 关闭文件
	// cout << "[调试] 关闭输出流..." << endl;
	fout.close();

	// 检查关闭后的最终状态
	if (!fout) {
		cerr << "[警告] 关闭后报告文件流错误: " << filename << "。文件可能不完整。" << endl;
		// 即使关闭后报告错误，也返回值，让上层决定如何处理
	}

	// --- [调试] 重新打开文件并检查实际大小 ---
#ifdef _DEBUG // 只在Debug模式下执行检查，避免影响Release性能
// 或者直接用 #if 1 强制开启
	{
		ifstream checker(filename, ios::binary | ios::ate);
		if (checker.is_open()) {
			streampos realFileSize = checker.tellg();
			checker.close();
			cout << "[调试] 保存后文件大小验证:" << endl;
			cout << "  头部声明的bfSize: " << fileHeader.bfSize << endl;
			cout << "  磁盘上的实际文件大小: " << (uint32_t)realFileSize << endl;
			if ((uint32_t)realFileSize != fileHeader.bfSize) {
				cerr << "[严重错误] 保存后声明的bfSize与实际文件大小不匹配！预期 "
					<< fileHeader.bfSize << "，实际 " << (uint32_t)realFileSize << endl;
				// 这里可以考虑返回 false 或者抛出异常，因为文件肯定有问题了
				return false; // 返回失败，因为文件不一致
			}
			else {
				cout << "[调试] 文件大小验证成功。" << endl;
			}
		}
		else {
			cerr << "[错误] 无法重新打开已保存的文件 ('" << filename << "') 以验证大小。" << endl;
			// 无法验证，但不能断定保存失败，所以还是先返回true
		}
	}
#endif
	// --- 调试检查结束 ---

	return true; // 假设关闭成功（除非上面的debug检查返回false）
}
