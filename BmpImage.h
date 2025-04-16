#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

/**
 * @brief Windows标准BMP文件头结构体 (14字节)
 * @details 定义了BMP文件的基本信息，包括文件类型、大小和数据偏移量
 * @note 使用 #pragma pack(push,1) 确保结构体按1字节对齐，避免编译器优化导致的内存对齐问题
 */
#pragma pack(push, 1)
struct BmpFileHeader {
	uint16_t bfType;      // 必须为 'BM' (即 0x4D42)，标识BMP文件格式
	uint32_t bfSize;      // BMP文件总大小(字节)，包含所有头信息和像素数据
	uint16_t bfReserved1; // 保留字段(必须为0)，用于未来扩展
	uint16_t bfReserved2; // 保留字段(必须为0)，用于未来扩展
	uint32_t bfOffBits;   // 从文件头到像素数据的偏移量(字节)，指示像素数据的起始位置
};

/**
 * @brief BMP信息头结构体 (40字节)
 * @details 包含图像的详细信息，如尺寸、颜色深度、压缩方式等
 * @note 对应Windows平台上的 BITMAPINFOHEADER 结构体，是最常用的BMP信息头格式
 */
struct BmpInfoHeader {
	uint32_t biSize;          // 信息头大小(固定为40字节)
	int32_t  biWidth;         // 图像宽度(像素)，正值
	int32_t  biHeight;        // 图像高度(像素)，正值表示从下到上存储，负值表示从上到下存储
	uint16_t biPlanes;        // 颜色平面数(固定为1)
	uint16_t biBitCount;      // 每像素位数(1,4,8,16,24,32)
	uint32_t biCompression;   // 压缩类型(0=BI_RGB表示不压缩)
	uint32_t biSizeImage;     // 图像数据大小(字节)，不压缩时可为0
	int32_t  biXPelsPerMeter; // 水平分辨率(像素/米)，可为0
	int32_t  biYPelsPerMeter; // 垂直分辨率(像素/米)，可为0
	uint32_t biClrUsed;       // 实际使用的调色板索引数，0表示使用所有
	uint32_t biClrImportant;  // 重要颜色索引数，0表示所有颜色都重要
};
#pragma pack(pop)

/**
 * @class BmpImage
 * @brief BMP图像文件处理类
 * @details 提供BMP格式图像的基本读写操作，支持24位和32位色深的未压缩BMP图像处理
 * @note 此实现主要面向Windows BITMAPINFOHEADER格式的BMP文件，不支持其他变体
 */
class BmpImage {
public:
	/**
	 * @brief 从文件加载BMP图像
	 * @param filename 待加载的BMP文件路径
	 * @return bool 加载是否成功
	 * @throw 可能抛出std::runtime_error异常（文件操作失败时）
	 */
	bool load(const std::string& filename);

	/**
	 * @brief 将图像保存为BMP文件
	 * @param filename 目标文件路径
	 * @return bool 保存是否成功
	 * @throw 可能抛出std::runtime_error异常（文件操作失败时）
	 */
	bool save(const std::string& filename) const;

	/**
	 * @brief 获取图像宽度
	 * @return int 图像宽度(像素)
	 */
	int getWidth() const { return m_infoHeader.biWidth; }

	/**
	 * @brief 获取图像高度
	 * @return int 图像高度(像素)
	 */
	int getHeight() const { return m_infoHeader.biHeight; }

	/**
	 * @brief 获取图像色深
	 * @return int 每像素位数(通常为24或32)
	 */
	int getBitCount() const { return m_infoHeader.biBitCount; }

	/**
	 * @brief 获取像素数据的可写指针
	 * @return unsigned char* 像素数据起始地址
	 * @warning 直接操作像素数据时需谨慎，确保不越界
	 */
	unsigned char* getPixelData() { return m_pixelData.data(); }

	/**
	 * @brief 获取像素数据的只读指针
	 * @return const unsigned char* 像素数据起始地址
	 */
	const unsigned char* getPixelData() const { return m_pixelData.data(); }

	/**
	 * @brief 获取像素数据大小
	 * @return size_t 像素数据总字节数
	 */
	size_t getPixelDataSize() const { return m_pixelData.size(); }

	/**
	 * @brief 获取像素数据偏移量
	 * @return int 像素数据在文件中的起始位置(字节)
	 */
	int getPixelDataOffset() const { return static_cast<int>(m_fileHeader.bfOffBits); }

	/**
	 * @brief 计算预期的文件大小
	 * @return size_t 预计的文件总大小(字节)
	 * @note 基于当前头信息和像素数据计算
	 */
	size_t getEstimatedFileSize() const {
		return static_cast<size_t>(m_fileHeader.bfOffBits) + m_pixelData.size();
	}

	/**
	 * @brief 获取文件头引用
	 * @return BmpFileHeader& 文件头结构体引用
	 */
	BmpFileHeader& fileHeader() { return m_fileHeader; }

	/**
	 * @brief 获取信息头引用
	 * @return BmpInfoHeader& 信息头结构体引用
	 */
	BmpInfoHeader& infoHeader() { return m_infoHeader; }

private:
	BmpFileHeader m_fileHeader{};             ///< BMP文件头
	BmpInfoHeader m_infoHeader{};             ///< BMP信息头
	std::vector<unsigned char> m_extraHeader;  ///< 额外头信息或调色板数据
	std::vector<unsigned char> m_pixelData;    ///< 图像像素数据
};

#endif // BMP_IMAGE_H
