#ifndef STEGO_CORE_H
#define STEGO_CORE_H

#include "BmpImage.h"
#include <string>
#include <vector>
#include <cstdint>
#include <limits>

/**
 * @file StegoCore.h
 * @brief BMP图像隐写核心类声明
 * @version 1.0
 * @copyright Copyright 2025, 信息隐藏第6组
 *
 * 本文件定义了基于LSB(最低有效位)算法的BMP图像隐写核心功能，
 * 支持多种隐写模式和通道选择，包含完整的数据校验和加密机制。
 */

 /**
  * @enum SteganoMode
  * @brief 隐写算法模式枚举
  *
  * 定义三种不同的LSB隐写实现方式，适用于不同安全级别的应用场景。
  */
enum SteganoMode : uint16_t {
	LSB_SEQUENTIAL = 0, ///< 顺序LSB模式(1 bit/通道)，隐写容量最大但安全性最低
	LSB_RANDOM = 1,     ///< 随机LSB模式(1 bit/通道)，需要密码且随机分布像素
	LSB_ENHANCED = 2    ///< 增强LSB模式(2 bit/通道)，平衡容量和隐蔽性
};

/**
 * @struct StegoContext
 * @brief 隐写操作运行时参数配置
 *
 * 包含隐写操作的所有可配置参数，作为hideData/extractData的输入输出上下文。
 */
struct StegoContext {
	SteganoMode mode = LSB_SEQUENTIAL; ///< 当前隐写模式，默认为顺序LSB
	uint16_t    channelMask = 0x01;    ///< 颜色通道掩码(B=0x1,G=0x2,R=0x4)
	std::string password;              ///< 加密密码(用于随机模式或数据加密)
	bool        autoDetect = false;    ///< 是否自动检测隐写模式和通道掩码
};

#pragma pack(push, 1)
/**
 * @struct StegoHeader
 * @brief 隐写数据头部结构(16字节)
 *
 * 嵌入到BMP像素数据前的元信息头，用于存储隐写数据的描述信息和校验值。
 * 采用紧凑内存布局(#pragma pack(1))确保跨平台兼容性。
 */
struct StegoHeader {
	char     signature[4];  ///< 文件标识魔数"STEG"(0x53 0x54 0x45 0x47)
	uint32_t dataLength;    ///< 隐写数据实际长度(字节)，不含头部
	uint32_t crc32Value;    ///< 原始数据的CRC32校验值(用于完整性验证)
	uint16_t stegoMode;     ///< 实际使用的SteganoMode枚举值
	uint16_t channelMask;   ///< 实际使用的通道掩码组合
};
#pragma pack(pop)
static_assert(sizeof(StegoHeader) == 16, "StegoHeader 大小必须为 16 字节");

/**
 * @class StegoCore
 * @brief BMP图像LSB隐写算法核心实现
 *
 * 提供完整的隐写功能：
 * 1. 支持多种LSB隐写模式(顺序/随机/增强)
 * 2. 可选择RGB通道组合(B/G/R任意组合)
 * 3. 数据加密(XOR混淆)和CRC32校验
 * 4. 自动容量计算和模式检测
 * 5. 完整的错误处理和边界检查
 */
class StegoCore {
public:
	/**
	 * @brief 将数据隐藏到BMP图像中
	 * @param[in,out] bmp 已加载的BMP图像对象(将被修改)
	 * @param[in] data 待隐藏数据的只读指针
	 * @param[in] length 待隐藏数据长度(字节)
	 * @param[in] ctx 隐写上下文参数配置
	 * @return 成功返回true，失败返回false
	 * @exception 无异常抛出，但会执行严格的参数校验
	 * @note 实际隐藏数据=16字节头+原始数据，总容量需小于图像最大容量
	 * @warning 会直接修改BMP像素数据，建议操作前备份原图
	 */
	bool hideData(BmpImage& bmp, const char* data, size_t length, const StegoContext& ctx);

	/**
	 * @brief 从BMP图像提取隐藏数据
	 * @param[in] bmp 已加载的BMP图像对象(只读)
	 * @param[out] outData 输出数据缓冲区指针(需调用者delete[])
	 * @param[out] outLength 输出数据实际长度
	 * @param[in,out] ctx 隐写上下文(autoDetect=true时会更新mode和channelMask)
	 * @return 成功返回true，失败返回false
	 * @exception 无异常抛出，但会校验数据完整性和密码正确性
	 * @note 输出缓冲区由本函数分配，调用者负责释放
	 * @warning 当autoDetect=true时，可能修改ctx中的mode和channelMask值
	 */
	bool extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx);

private:
	/**
	 * @brief 计算数据的CRC32校验值
	 * @param buffer 输入数据缓冲区
	 * @param length 数据长度(字节)
	 * @return 32位CRC校验值
	 */
	uint32_t calcCRC32(const char* buffer, size_t length) const;

	/**
	 * @brief 使用密码对缓冲区进行XOR加密/解密
	 * @param[in,out] buffer 输入输出缓冲区(原地修改)
	 * @param length 缓冲区长度(字节)
	 * @param password 加密密码(空密码时不执行操作)
	 */
	void xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const;

	/**
	 * @brief 计算BMP图像的隐写容量
	 * @param bmp BMP图像对象
	 * @param ctx 隐写上下文参数
	 * @return 最大可隐藏数据量(字节)，含16字节头
	 */
	size_t calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const;

	/* 核心读写实现方法 */
	bool writeAll(BmpImage& bmp, const StegoHeader& header,
		const char* data, size_t length, const StegoContext& ctx);
	bool readHeader(const BmpImage& bmp, StegoHeader& headerOut,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password, bool checkSignature) const;
	bool readDataSection(const BmpImage& bmp, const StegoHeader& header,
		char* dataOut, size_t length,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password) const;

	/* 各模式的具体实现 */
	bool writeSequentialLSB(unsigned char* pixelData, size_t pixelDataSize,
		const char* src, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel) const;
	bool readSequentialLSB(const unsigned char* pixelData, size_t pixelDataSize,
		char* dst, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel) const;
	bool writeRandomLSB(unsigned char* pixelData, size_t pixelDataSize,
		const char* src, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel,
		const std::string& password, size_t offsetBits = 0) const;
	bool readRandomLSB(const unsigned char* pixelData, size_t pixelDataSize,
		char* dst, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel,
		const std::string& password, size_t offsetBits = 0) const;
};

#endif // STEGO_CORE_H