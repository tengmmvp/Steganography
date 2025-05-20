#include "BmpImage.h"
#include <fstream>
#include <iostream>
#include <cstring>

/**
 * @file BmpImage.cpp
 * @brief BMP图像文件加载与保存实现
 * @copyright Copyright 2025, 信息隐藏第6组
 *
 * 本文件实现了BmpImage类的所有功能，包括BMP文件的加载解析和保存操作。
 * 严格遵循Windows BMP文件格式规范，处理各种边界条件和错误情况。
 */

using namespace std;

/**
 * @brief 从文件加载BMP图像
 * @param[in] filename 要加载的BMP文件路径
 * @return 加载成功返回true，失败返回false
 *
 * 该方法执行以下操作：
 * 1. 打开文件并验证基本有效性
 * 2. 读取并校验文件头和信息头
 * 3. 处理可能的扩展头或调色板数据
 * 4. 读取像素数据并验证完整性
 *
 * @note 文件打开失败或格式错误会输出详细错误信息到cerr
 * @warning 加载失败会清空现有图像数据
 */
bool BmpImage::load(const std::string& filename)
{
	// 以二进制模式打开文件
	ifstream fin(filename, ios::binary);
	if (!fin.is_open()) {
		cerr << "[错误] 无法打开文件: " << filename << endl;
		return false;
	}

	/* 文件头读取与验证 */
	fin.read(reinterpret_cast<char*>(&m_fileHeader), sizeof(m_fileHeader));
	if (!fin || m_fileHeader.bfType != 0x4D42) {
		cerr << "[错误] 无效的BMP文件: " << filename
			<< " (文件类型标识应为0x4D42)" << endl;
		fin.close();
		return false;
	}

	/* 信息头读取与验证 */
	fin.read(reinterpret_cast<char*>(&m_infoHeader), sizeof(m_infoHeader));
	if (!fin) {
		cerr << "[错误] 读取信息头失败: " << filename << endl;
		fin.close();
		return false;
	}

	/* 处理扩展头/调色板数据 */
	size_t dataOffset = m_fileHeader.bfOffBits;
	streamoff curPos = fin.tellg();

	// 检查偏移量是否合理并定位读取位置
	if (dataOffset < static_cast<size_t>(curPos)) {
		// 回退到指定偏移量（处理异常情况）
		fin.seekg(dataOffset, ios::beg);
	}
	else if (dataOffset > static_cast<size_t>(curPos)) {
		// 读取扩展头数据
		size_t extraSize = dataOffset - static_cast<size_t>(curPos);
		m_extraHeader.resize(extraSize);
		fin.read(reinterpret_cast<char*>(m_extraHeader.data()), extraSize);
		if (!fin || static_cast<size_t>(fin.gcount()) != extraSize) {
			cerr << "[错误] 读取扩展头失败: " << filename
				<< " (期望大小: " << extraSize
				<< ", 实际读取: " << fin.gcount() << ")" << endl;
			fin.close();
			return false;
		}
	}

	/* 像素数据读取 */
	size_t pixelSize = 0;
	// 优先使用文件头中的总大小计算
	if (m_fileHeader.bfSize > dataOffset) {
		pixelSize = m_fileHeader.bfSize - dataOffset;
	}
	// 次选信息头中的像素数据大小
	if (pixelSize == 0 && m_infoHeader.biSizeImage > 0) {
		pixelSize = m_infoHeader.biSizeImage;
	}

	m_pixelData.resize(pixelSize);
	fin.seekg(dataOffset, ios::beg);

	if (pixelSize > 0) {
		fin.read(reinterpret_cast<char*>(m_pixelData.data()), pixelSize);
		if (!fin || static_cast<size_t>(fin.gcount()) != pixelSize) {
			cerr << "[错误] 读取像素数据失败: " << filename
				<< " (期望大小: " << pixelSize
				<< ", 实际读取: " << fin.gcount() << ")" << endl;
			fin.close();
			m_pixelData.clear();
			return false;
		}
	}

	fin.close();
	return true;
}

/**
 * @brief 将图像保存为BMP文件
 * @param[in] filename 输出文件路径
 * @return 保存成功返回true，失败返回false
 *
 * 该方法执行以下操作：
 * 1. 准备并验证头信息
 * 2. 写入文件头和信息头
 * 3. 写入扩展头或调色板数据
 * 4. 写入像素数据
 *
 * @note 保存失败会输出详细错误信息到cerr
 * @warning 会覆盖已存在的同名文件
 */
bool BmpImage::save(const std::string& filename) const
{
	// 以二进制模式创建/截断文件
	ofstream fout(filename, ios::binary | ios::trunc);
	if (!fout.is_open()) {
		cerr << "[错误] 无法创建文件: " << filename << endl;
		return false;
	}

	/* 准备头信息 */
	BmpFileHeader fh = m_fileHeader;
	BmpInfoHeader ih = m_infoHeader;
	uint32_t offBits = fh.bfOffBits;
	uint32_t pixelSize = static_cast<uint32_t>(m_pixelData.size());

	// 更新文件总大小
	fh.bfSize = offBits + pixelSize;

	/* 写入文件头 */
	fout.write(reinterpret_cast<const char*>(&fh), sizeof(fh));
	if (!fout) {
		cerr << "[错误] 写入文件头失败" << endl;
		fout.close();
		return false;
	}

	/* 写入信息头及扩展部分 */
	if (ih.biSize >= sizeof(BmpInfoHeader)) {
		fout.write(reinterpret_cast<const char*>(&ih), sizeof(BmpInfoHeader));
		if (!fout) {
			cerr << "[错误] 写入信息头失败" << endl;
			fout.close();
			return false;
		}

		// 处理可能的扩展头
		if (ih.biSize > sizeof(BmpInfoHeader)) {
			size_t ext = ih.biSize - sizeof(BmpInfoHeader);
			if (ext <= m_extraHeader.size()) {
				fout.write(reinterpret_cast<const char*>(m_extraHeader.data()), ext);
				if (!fout) {
					cerr << "[错误] 写入扩展头失败" << endl;
					fout.close();
					return false;
				}
			}
			else {
				cerr << "[错误] 扩展头大小与实际不符 (需要: " << ext
					<< ", 实际: " << m_extraHeader.size() << ")" << endl;
				fout.close();
				return false;
			}
		}
	}
	else {
		cerr << "[错误] 信息头大小无效: " << ih.biSize
			<< " (最小应为: " << sizeof(BmpInfoHeader) << ")" << endl;
		fout.close();
		return false;
	}

	/* 写入剩余调色板数据 */
	size_t paletteOff = (ih.biSize > sizeof(BmpInfoHeader))
		? ih.biSize - sizeof(BmpInfoHeader) : 0;
	if (paletteOff < m_extraHeader.size()) {
		fout.write(reinterpret_cast<const char*>(m_extraHeader.data() + paletteOff),
			m_extraHeader.size() - paletteOff);
		if (!fout) {
			cerr << "[错误] 写入调色板数据失败" << endl;
			fout.close();
			return false;
		}
	}

	/* 写入像素数据 */
	fout.seekp(offBits, ios::beg);
	if (!m_pixelData.empty()) {
		fout.write(reinterpret_cast<const char*>(m_pixelData.data()), m_pixelData.size());
		if (!fout) {
			cerr << "[错误] 写入像素数据失败 (大小: "
				<< m_pixelData.size() << " 字节)" << endl;
			fout.close();
			return false;
		}
	}

	fout.close();
	return true;
}