#ifndef STEGO_CORE_H
#define STEGO_CORE_H

#include "BmpImage.h"
#include <string>
#include <vector>
#include <cstdint>
#include <limits>

/**
 * @enum SteganoMode
 * @brief 定义隐写算法的三种工作模式
 *
 * @var LSB_SEQUENTIAL 顺序最低有效位隐写（每个颜色通道使用最低1位）
 * @var LSB_RANDOM     基于密码的随机位置隐写（每个颜色通道最低1位）
 * @var LSB_ENHANCED   高容量顺序隐写（每个颜色通道最低2位）
 */
enum SteganoMode : uint16_t {
	LSB_SEQUENTIAL = 0,
	LSB_RANDOM = 1,
	LSB_ENHANCED = 2
};

/**
 * @struct StegoContext
 * @brief 隐写操作运行时配置参数集
 *
 * @var mode       隐写算法模式选择
 * @var channelMask 颜色通道选择掩码（位组合：0x01蓝 0x02绿 0x04红）
 * @var password   加密/随机模式密码（空字符串表示禁用）
 * @var autoDetect 提取时是否启用自动模式检测
 */
struct StegoContext {
	SteganoMode mode = LSB_SEQUENTIAL;
	uint16_t    channelMask = 0x01;
	std::string password;
	bool        autoDetect = false;
};

/**
 * @struct StegoHeader
 * @brief 隐写数据头部结构（16字节固定大小）
 *
 * @note 使用#pragma pack确保内存对齐
 * @var signature   文件魔数（"STEG"）
 * @var dataLength  隐写数据实际长度（字节）
 * @var crc32Value  原始数据CRC32校验值（加密前）
 * @var stegoMode   实际使用的隐写模式
 * @var channelMask 实际使用的通道掩码
 */
#pragma pack(push, 1)
struct StegoHeader {
	char     signature[4];
	uint32_t dataLength;
	uint32_t crc32Value;
	uint16_t stegoMode;
	uint16_t channelMask;
};
#pragma pack(pop)

static_assert(sizeof(StegoHeader) == 16, "StegoHeader必须严格16字节");

/**
 * @class StegoCore
 * @brief BMP图像隐写术核心实现类
 *
 * 提供基于LSB算法的数据隐藏/提取功能，支持三种隐写模式：
 * 1. 顺序LSB隐写（基础模式）
 * 2. 随机位置LSB隐写（安全性增强）
 * 3. 增强容量LSB隐写（容量倍增）
 */
class StegoCore
{
public:
	/**
	 * @brief 执行数据隐藏操作
	 *
	 * @param[in] bmp     已加载的BMP图像对象
	 * @param[in] data    待隐藏数据指针
	 * @param[in] length  数据长度（字节）
	 * @param[in] ctx     隐写配置参数
	 * @return bool       操作是否成功
	 *
	 * @note 自动添加16字节头部信息，实际容量需扣除头部大小
	 */
	bool hideData(BmpImage& bmp, const char* data, size_t length, const StegoContext& ctx);

	/**
	 * @brief 执行数据提取操作
	 *
	 * @param[in] bmp        已加载的BMP图像对象
	 * @param[out] outData   提取数据缓冲区指针（需调用者释放）
	 * @param[out] outLength 提取数据实际长度
	 * @param[in,out] ctx    隐写配置参数（autoDetect=true时会修改模式参数）
	 * @return bool          操作是否成功
	 */
	bool extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx);

private:
	/**
	 * @brief 计算数据的CRC32校验值
	 *
	 * @param buffer  数据缓冲区
	 * @param length  数据长度（字节）
	 * @return uint32_t 计算得到的CRC32值
	 */
	uint32_t calcCRC32(const char* buffer, size_t length) const;

	/**
	 * @brief 执行XOR流加密/解密（原地操作）
	 *
	 * @param buffer   待处理数据缓冲区
	 * @param length   数据长度（字节）
	 * @param password 加密密码（空字符串跳过操作）
	 */
	void xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const;

	/**
	 * @brief 写入隐写头部和有效数据
	 *
	 * @param bmp    目标BMP图像
	 * @param header 16字节头部信息
	 * @param data   有效数据指针
	 * @param length 有效数据长度
	 * @param ctx    隐写配置参数
	 * @return bool  操作是否成功
	 */
	bool writeAll(BmpImage& bmp, const StegoHeader& header, const char* data, size_t length, const StegoContext& ctx);

	/**
	 * @brief 从BMP图像读取隐写头部
	 *
	 * @param[in] bmp              源BMP图像
	 * @param[out] headerOut       头部信息输出
	 * @param[in] modeToTry        尝试的隐写模式
	 * @param[in] channelMaskToTry 尝试的通道掩码
	 * @param[in] password         随机模式密码
	 * @param[in] checkSignature   是否验证魔数
	 * @return bool                是否成功读取
	 */
	bool readHeader(const BmpImage& bmp, StegoHeader& headerOut,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password, bool checkSignature) const;

	/**
	 * @brief 读取隐写数据部分
	 *
	 * @param[in] bmp              源BMP图像
	 * @param[in] header           已解析的头部信息
	 * @param[out] dataOut         数据输出缓冲区（需预分配）
	 * @param[in] length           预期数据长度
	 * @param[in] modeToTry        尝试的隐写模式
	 * @param[in] channelMaskToTry 尝试的通道掩码
	 * @param[in] password         随机模式密码
	 * @return bool                是否成功读取
	 */
	bool readDataSection(const BmpImage& bmp, const StegoHeader& header,
		char* dataOut, size_t length,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password) const;

	/**
	 * @brief 顺序LSB写入实现
	 *
	 * @param[in,out] pixelData    像素数据缓冲区
	 * @param[in] pixelDataSize    像素数据总大小
	 * @param[in] src              源数据指针
	 * @param[in] numBytes         写入字节数
	 * @param[in] channelMask      颜色通道掩码
	 * @param[in] bitsPerChannel   每通道写入位数（1或2）
	 * @return bool                操作是否成功
	 */
	bool writeSequentialLSB(unsigned char* pixelData, size_t pixelDataSize,
		const char* src, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel) const;

	/**
	 * @brief 顺序LSB读取实现
	 *
	 * @param[in] pixelData       像素数据缓冲区（只读）
	 * @param[out] dst            输出数据缓冲区
	 * @param[in] pixelDataSize   像素数据总大小
	 * @param[in] numBytes        读取字节数
	 * @param[in] channelMask     颜色通道掩码
	 * @param[in] bitsPerChannel  每通道读取位数（1或2）
	 * @return bool               操作是否成功
	 */
	bool readSequentialLSB(const unsigned char* pixelData, size_t pixelDataSize,
		char* dst, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel) const;

	/**
	 * @brief 随机LSB写入实现
	 *
	 * @param[in,out] pixelData    像素数据缓冲区
	 * @param[in] pixelDataSize    像素数据总大小
	 * @param[in] src              源数据指针
	 * @param[in] numBytes         写入字节数
	 * @param[in] channelMask      颜色通道掩码
	 * @param[in] bitsPerChannel   每通道写入位数（当前仅支持1）
	 * @param[in] password         随机序列生成密码
	 * @param[in] offsetBits       LSB流起始偏移（默认0）
	 * @return bool                操作是否成功
	 */
	bool writeRandomLSB(unsigned char* pixelData, size_t pixelDataSize,
		const char* src, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel,
		const std::string& password, size_t offsetBits = 0) const;

	/**
	 * @brief 随机LSB读取实现
	 *
	 * @param[in] pixelData       像素数据缓冲区（只读）
	 * @param[out] dst            输出数据缓冲区
	 * @param[in] pixelDataSize   像素数据总大小
	 * @param[in] numBytes        读取字节数
	 * @param[in] channelMask     颜色通道掩码
	 * @param[in] bitsPerChannel  每通道读取位数（当前仅支持1）
	 * @param[in] password        随机序列生成密码
	 * @param[in] offsetBits      LSB流起始偏移（默认0）
	 * @return bool               操作是否成功
	 */
	bool readRandomLSB(const unsigned char* pixelData, size_t pixelDataSize,
		char* dst, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel,
		const std::string& password, size_t offsetBits = 0) const;

	/**
	 * @brief 计算图像隐写容量
	 *
	 * @param[in] bmp  目标BMP图像
	 * @param[in] ctx  隐写配置参数
	 * @return size_t  最大可隐藏字节数（不含头部）
	 */
	size_t calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const;
};

#endif // STEGO_CORE_H
