#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

/**
 * @file BmpImage.h
 * @brief BMP图像文件读写处理类声明
 * @version 1.0
 * @copyright Copyright 2025, 信息隐藏第6组
 *
 * 本文件定义了BMP图像文件格式的结构体和处理类，支持24位和32位未压缩BMP格式的读写操作。
 * 符合Windows BMP文件格式规范，支持自下而上的像素存储顺序。
 */

#pragma pack(push, 1)
 /**
  * @struct BmpFileHeader
  * @brief BMP文件头结构体（14字节）
  *
  * 该结构体对应BMP文件的文件头部分，包含文件标识、大小和像素数据偏移量等基本信息。
  * 所有字段采用小端序存储，使用#pragma pack(1)确保内存对齐为1字节。
  */
struct BmpFileHeader {
	uint16_t bfType;      ///< 文件类型标识，必须为'BM'(0x4D42)
	uint32_t bfSize;      ///< 文件总大小（字节），包括所有头和数据
	uint16_t bfReserved1; ///< 保留字段1，必须初始化为0
	uint16_t bfReserved2; ///< 保留字段2，必须初始化为0
	uint32_t bfOffBits;   ///< 从文件头到像素数据的偏移量（字节）
};

/**
 * @struct BmpInfoHeader
 * @brief BMP信息头结构体（40字节）
 *
 * 该结构体包含BMP图像的尺寸、色彩深度和压缩方式等核心信息。
 * 这是Windows BITMAPINFOHEADER的标准实现，支持最常见的未压缩格式。
 */
struct BmpInfoHeader {
	uint32_t biSize;          ///< 本结构体大小（固定为40字节）
	int32_t  biWidth;         ///< 图像宽度（像素），必须为正数
	int32_t  biHeight;        ///< 图像高度（像素），正数表示自下向上存储
	uint16_t biPlanes;        ///< 颜色平面数（固定为1）
	uint16_t biBitCount;      ///< 每像素位数（支持24或32）
	uint32_t biCompression;   ///< 压缩类型（0表示BI_RGB无压缩）
	uint32_t biSizeImage;     ///< 像素数据大小（字节），无压缩时可设为0
	int32_t  biXPelsPerMeter; ///< 水平分辨率（像素/米），可设为0
	int32_t  biYPelsPerMeter; ///< 垂直分辨率（像素/米），可设为0
	uint32_t biClrUsed;       ///< 实际使用的调色板颜色数，0表示使用全部
	uint32_t biClrImportant;  ///< 重要颜色数，0表示所有颜色都重要
};
#pragma pack(pop)

/**
 * @class BmpImage
 * @brief BMP图像处理类
 *
 * 提供BMP格式图像的加载、保存和基本信息查询功能。支持24位RGB和32位RGBA格式，
 * 自动处理像素对齐（每行像素数据按4字节对齐）。内存中维护完整的文件头、信息头和像素数据。
 */
class BmpImage {
public:
	/**
	 * @brief 从文件加载BMP图像
	 * @param[in] filename 要加载的BMP文件路径
	 * @return 加载成功返回true，失败返回false
	 * @exception 无异常抛出，但会设置错误状态
	 * @note 仅支持未压缩的24/32位BMP格式，高度值为正的自下向上存储方式
	 */
	bool load(const std::string& filename);

	/**
	 * @brief 将图像保存为BMP文件
	 * @param[in] filename 输出文件路径
	 * @return 保存成功返回true，失败返回false
	 * @exception 无异常抛出，但会设置错误状态
	 * @note 输出文件将采用未压缩格式，自动计算像素对齐
	 */
	bool save(const std::string& filename) const;

	/**
	 * @brief 获取图像宽度
	 * @return 图像宽度（像素）
	 */
	int getWidth() const { return m_infoHeader.biWidth; }

	/**
	 * @brief 获取图像高度
	 * @return 图像高度（像素）
	 * @note 返回值为正表示自下向上存储
	 */
	int getHeight() const { return m_infoHeader.biHeight; }

	/**
	 * @brief 获取色深
	 * @return 每像素位数（24或32）
	 */
	int getBitCount() const { return m_infoHeader.biBitCount; }

	/**
	 * @brief 获取可写像素数据指针
	 * @return 指向像素数据的可写指针
	 * @warning 直接修改像素数据可能破坏图像完整性
	 */
	unsigned char* getPixelData() { return m_pixelData.data(); }

	/**
	 * @brief 获取只读像素数据指针
	 * @return 指向像素数据的只读指针
	 */
	const unsigned char* getPixelData() const { return m_pixelData.data(); }

	/**
	 * @brief 获取像素数据大小
	 * @return 像素数据占用的字节数
	 */
	size_t getPixelDataSize() const { return m_pixelData.size(); }

	/**
	 * @brief 获取像素数据偏移量
	 * @return 从文件头到像素数据的字节偏移量
	 */
	int getPixelDataOffset() const { return static_cast<int>(m_fileHeader.bfOffBits); }

	/**
	 * @brief 估算文件总大小
	 * @return 包含所有头和数据的文件总大小（字节）
	 */
	size_t getEstimatedFileSize() const {
		return static_cast<size_t>(m_fileHeader.bfOffBits) + m_pixelData.size();
	}

	/**
	 * @brief 获取文件头引用
	 * @return BMP文件头的可修改引用
	 * @warning 直接修改可能破坏文件完整性
	 */
	BmpFileHeader& fileHeader() { return m_fileHeader; }

	/**
	 * @brief 获取信息头引用
	 * @return BMP信息头的可修改引用
	 * @warning 直接修改可能破坏文件完整性
	 */
	BmpInfoHeader& infoHeader() { return m_infoHeader; }

private:
	BmpFileHeader m_fileHeader{};             ///< BMP文件头结构体实例
	BmpInfoHeader m_infoHeader{};             ///< BMP信息头结构体实例
	std::vector<unsigned char> m_extraHeader; ///< 扩展头或调色板数据（如有）
	std::vector<unsigned char> m_pixelData;   ///< 像素数据存储区
};

#endif // BMP_IMAGE_H