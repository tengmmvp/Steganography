#include "StegoCore.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <numeric>
#include <cstring>
#include <stdexcept>

/**
 * @file StegoCore.cpp
 * @brief BMP图像LSB隐写算法核心实现
 * @version 1.0
 * @copyright Copyright 2025, 信息隐藏第6组
 *
 * 本文件实现了StegoCore类的所有功能，包括：
 * 1. 基于LSB的数据隐藏与提取算法
 * 2. 数据加密与完整性校验
 * 3. 多种隐写模式的具体实现
 */

using namespace std;

/**
 * @brief CRC32校验表（256项）
 *
 * 用于快速计算CRC32校验值的预计算表，基于多项式0xEDB88320
 * 表中每个值对应一个可能的字节值(0-255)的CRC32部分计算结果
 */
static const uint32_t crcTable[256] = {
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
	0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
	0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
	0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
	0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
	0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
	0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
	0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
	0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
	0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
	0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
	0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
	0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
	0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
	0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
	0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
	0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
	0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
	0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
	0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
	0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
	0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
	0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
	0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
	0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
	0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
	0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
	0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
	0xA00AE278,0xD70DD2EE,0x4E048354,0x3903C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
	0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
	0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
	0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
};

/**
 * @brief 计算数据的CRC32校验值
 *
 * 使用标准CRC32算法计算数据缓冲区的校验值，用于验证数据完整性。
 *
 * @param[in] buffer 输入数据缓冲区
 * @param[in] length 数据长度(字节)
 * @return 32位CRC校验值
 */
uint32_t StegoCore::calcCRC32(const char* buffer, size_t length) const
{
	uint32_t crc = 0xFFFFFFFF; // 初始值
	for (size_t i = 0; i < length; ++i) {
		// 查表法计算CRC32
		unsigned char idx = static_cast<unsigned char>((crc ^ buffer[i]) & 0xFF);
		crc = (crc >> 8) ^ crcTable[idx];
	}
	return crc ^ 0xFFFFFFFF; // 最终异或值
}

/**
 * @brief 使用密码对缓冲区进行XOR加密/解密
 *
 * 对数据进行简单的XOR加密，使用密码作为密钥。
 * 加密和解密操作相同，重复执行会还原原始数据。
 *
 * @param[in,out] buffer 输入输出缓冲区(原地修改)
 * @param[in] length 缓冲区长度(字节)
 * @param[in] password 加密密码(空密码时不执行操作)
 */
void StegoCore::xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const
{
	if (password.empty() || length == 0) return;

	size_t plen = password.size();
	for (size_t i = 0; i < length; ++i) {
		// 循环使用密码字符进行XOR操作
		buffer[i] ^= password[i % plen];
	}
}

/**
 * @brief 计算BMP图像的隐写容量
 *
 * 根据图像尺寸、通道数和隐写模式计算最大可隐藏数据量。
 *
 * @param[in] bmp BMP图像对象
 * @param[in] ctx 隐写上下文参数
 * @return 最大可隐藏数据量(字节)，已减去16字节头部
 */
size_t StegoCore::calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const
{
	// 检查图像格式是否支持(仅支持24位或32位)
	int bpp = bmp.getBitCount();
	if (bpp != 24 && bpp != 32) return 0;

	int channels = bpp / 8;
	size_t dataSize = bmp.getPixelDataSize();
	if (dataSize == 0) return 0;

	// 计算实际使用的通道数
	int used = 0;
	if (ctx.channelMask & 0x01) ++used; // 蓝色通道
	if (ctx.channelMask & 0x02) ++used; // 绿色通道
	if (ctx.channelMask & 0x04) ++used; // 红色通道

	// 根据模式确定每通道使用的位数
	int bitsPer = (ctx.mode == LSB_ENHANCED) ? 2 : 1;

	// 计算总容量
	size_t pixels = dataSize / channels;
	size_t totalBits = pixels * used * bitsPer;
	size_t bytes = totalBits / 8;

	// 减去头部大小，返回实际可用容量
	return (bytes >= sizeof(StegoHeader)) ? (bytes - sizeof(StegoHeader)) : 0;
}

/**
 * @brief 将数据隐藏到BMP图像中
 *
 * 实现数据隐写的主要流程：
 * 1. 验证数据长度和图像容量
 * 2. 准备隐写头部信息
 * 3. 对数据进行加密处理
 * 4. 根据隐写模式调用相应的写入函数
 *
 * @param[in,out] bmp 已加载的BMP图像对象(将被修改)
 * @param[in] data 待隐藏数据的只读指针
 * @param[in] length 待隐藏数据长度(字节)
 * @param[in] ctx 隐写上下文参数配置
 * @return 成功返回true，失败返回false
 */
bool StegoCore::hideData(BmpImage& bmp, const char* data, size_t length, const StegoContext& ctx)
{
	// 验证数据长度
	if (length == 0 || length > numeric_limits<uint32_t>::max()) {
		cerr << "[错误] 数据长度不合法: " << length << " 字节" << endl;
		return false;
	}

	// 检查容量是否足够
	size_t cap = calculateCapacity(bmp, ctx);
	if (length > cap) {
		cerr << "[错误] 容量不足: 需要 " << length << " 字节， 可用 " << cap << " 字节" << endl;
		return false;
	}

	// 准备隐写头部
	StegoHeader header;
	memcpy(header.signature, "STEG", 4);           // 魔数标识
	header.dataLength = static_cast<uint32_t>(length);
	header.stegoMode = ctx.mode;
	header.channelMask = ctx.channelMask;
	header.crc32Value = calcCRC32(data, length);   // 计算原始数据校验值

	// 复制并加密数据
	vector<char> buf(data, data + length);
	xorEncryptBuffer(buf.data(), length, ctx.password);

	// 写入头部和加密后的数据
	return writeAll(bmp, header, buf.data(), length, ctx);
}

/**
 * @brief 从BMP图像提取隐藏数据
 *
 * 实现数据提取的主要流程：
 * 1. 尝试读取隐写头部信息
 * 2. 验证头部有效性
 * 3. 读取并解密数据
 * 4. 验证数据完整性
 *
 * 支持自动检测模式，会尝试多种隐写模式和通道组合。
 *
 * @param[in] bmp 已加载的BMP图像对象(只读)
 * @param[out] outData 输出数据缓冲区指针(需调用者delete[])
 * @param[out] outLength 输出数据实际长度
 * @param[in,out] ctx 隐写上下文(autoDetect=true时会更新mode和channelMask)
 * @return 成功返回true，失败返回false
 */
bool StegoCore::extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx)
{
	// 初始化输出参数
	outData = nullptr;
	outLength = 0;

	// 确定要尝试的模式和通道掩码
	vector<SteganoMode> modes;
	vector<uint16_t>    masks;

	if (ctx.autoDetect) {
		// 自动检测模式：尝试所有可能的模式和通道组合
		modes = { LSB_SEQUENTIAL, LSB_ENHANCED, LSB_RANDOM };
		masks = { 0x01,0x02,0x04,0x03,0x05,0x06,0x07 }; // 所有可能的通道组合
	}
	else {
		// 指定模式：仅尝试用户指定的模式和通道
		modes = { ctx.mode };
		masks = { ctx.channelMask };
	}

	// 尝试所有可能的模式和通道组合
	for (auto m : modes) {
		for (auto mask : masks) {
			// 尝试读取头部
			StegoHeader hdr;
			if (!readHeader(bmp, hdr, m, mask, ctx.password, true)) continue;

			// 验证数据长度合理性
			if (hdr.dataLength == 0 || hdr.dataLength > 100 * 1024 * 1024) continue;

			// 分配内存
			size_t len = hdr.dataLength;
			char* buf = nullptr;
			try {
				buf = new char[len];
			}
			catch (...) {
				cerr << "[错误] 内存分配失败: " << len << " 字节" << endl;
				continue;
			}

			// 读取数据部分
			if (!readDataSection(bmp, hdr, buf, len, m, mask, ctx.password)) {
				delete[] buf;
				continue;
			}

			// 解密数据
			xorEncryptBuffer(buf, len, ctx.password);

			// 验证CRC32校验值
			if (calcCRC32(buf, len) != hdr.crc32Value) {
				delete[] buf;
				continue;
			}

			// 提取成功
			outData = buf;
			outLength = len;
			ctx.mode = static_cast<SteganoMode>(hdr.stegoMode);
			ctx.channelMask = hdr.channelMask;
			return true;
		}
	}

	// 所有尝试都失败
	if (!ctx.autoDetect) {
		cerr << "[警告] 提取失败，请检查设置或密码" << endl;
	}
	return false;
}

/**
 * @brief 写入完整的隐写数据(头部+数据)
 *
 * 根据隐写模式选择相应的写入算法，将头部和数据写入图像。
 *
 * @param[in,out] bmp BMP图像对象
 * @param[in] header 隐写头部结构
 * @param[in] data 待隐藏数据(已加密)
 * @param[in] length 数据长度
 * @param[in] ctx 隐写上下文
 * @return 成功返回true，失败返回false
 */
bool StegoCore::writeAll(BmpImage& bmp, const StegoHeader& header,
	const char* data, size_t length, const StegoContext& ctx)
{
	// 准备完整数据缓冲区(头部+数据)
	size_t total = sizeof(header) + length;
	vector<char> fullBuf(total);
	memcpy(fullBuf.data(), &header, sizeof(header));
	memcpy(fullBuf.data() + sizeof(header), data, length);

	// 获取像素数据
	unsigned char* pixels = bmp.getPixelData();
	size_t pdSize = bmp.getPixelDataSize();

	// 根据模式确定每通道使用的位数
	int bitsPer = (ctx.mode == LSB_ENHANCED) ? 2 : 1;
	if (ctx.mode == LSB_RANDOM) bitsPer = 1;

	// 根据模式选择写入算法
	bool ok = false;
	if (ctx.mode == LSB_SEQUENTIAL || ctx.mode == LSB_ENHANCED) {
		// 顺序模式或增强模式
		ok = writeSequentialLSB(pixels, pdSize, fullBuf.data(), total, ctx.channelMask, bitsPer);
	}
	else {
		// 随机模式
		ok = writeRandomLSB(pixels, pdSize, fullBuf.data(), total, ctx.channelMask, 1, ctx.password, 0);
	}

	if (!ok) {
		cerr << "[错误] 写入隐写数据失败" << endl;
	}
	return ok;
}

/**
 * @brief 读取隐写头部信息
 *
 * 尝试使用指定的模式和通道从图像中读取隐写头部。
 *
 * @param[in] bmp BMP图像对象
 * @param[out] headerOut 输出的头部结构
 * @param[in] modeToTry 尝试的隐写模式
 * @param[in] channelMaskToTry 尝试的通道掩码
 * @param[in] password 解密密码
 * @param[in] checkSignature 是否检查魔数标识
 * @return 成功返回true，失败返回false
 */
bool StegoCore::readHeader(const BmpImage& bmp, StegoHeader& headerOut,
	SteganoMode modeToTry, uint16_t channelMaskToTry,
	const std::string& password, bool checkSignature) const
{
	// 准备缓冲区接收头部数据
	vector<char> buf(sizeof(StegoHeader), 0);
	const unsigned char* pixels = bmp.getPixelData();
	size_t pdSize = bmp.getPixelDataSize();
	if (pdSize == 0) return false;

	// 根据模式确定每通道使用的位数
	int bitsPer = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	if (modeToTry == LSB_RANDOM) bitsPer = 1;

	// 根据模式选择读取算法
	bool ok = false;
	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		// 顺序模式或增强模式
		ok = readSequentialLSB(pixels, pdSize, buf.data(), buf.size(), channelMaskToTry, bitsPer);
	}
	else {
		// 随机模式
		ok = readRandomLSB(pixels, pdSize, buf.data(), buf.size(), channelMaskToTry, 1, password, 0);
	}
	if (!ok) return false;

	// 复制头部数据
	memcpy(&headerOut, buf.data(), sizeof(StegoHeader));

	// 验证魔数标识
	if (checkSignature && memcmp(headerOut.signature, "STEG", 4) != 0) {
		return false;
	}
	return true;
}

/**
 * @brief 读取隐写数据部分
 *
 * 根据已知的头部信息，从图像中读取隐写数据部分。
 *
 * @param[in] bmp BMP图像对象
 * @param[in] header 已读取的隐写头部
 * @param[out] dataOut 输出数据缓冲区
 * @param[in] length 要读取的数据长度
 * @param[in] modeToTry 尝试的隐写模式
 * @param[in] channelMaskToTry 尝试的通道掩码
 * @param[in] password 解密密码
 * @return 成功返回true，失败返回false
 */
bool StegoCore::readDataSection(const BmpImage& bmp, const StegoHeader& header,
	char* dataOut, size_t length,
	SteganoMode modeToTry, uint16_t channelMaskToTry,
	const std::string& password) const
{
	const unsigned char* pixels = bmp.getPixelData();
	size_t pdSize = bmp.getPixelDataSize();
	if (length == 0) return true;
	if (pdSize == 0) return false;

	// 根据模式确定每通道使用的位数
	int bitsPer = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	if (modeToTry == LSB_RANDOM) bitsPer = 1;

	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		// 顺序或增强模式：读取包括头在内的整个块，然后提取数据部分
		size_t totalBytesToRead = sizeof(StegoHeader) + length;

		// 读取完整数据(头部+数据)
		vector<char> fullBuf(totalBytesToRead);
		if (!readSequentialLSB(pixels, pdSize, fullBuf.data(), totalBytesToRead, channelMaskToTry, bitsPer))
			return false;

		// 提取数据部分
		memcpy(dataOut, fullBuf.data() + sizeof(StegoHeader), length);
		return true;
	}
	else { // LSB_RANDOM
		// 随机模式：读取包括头在内的整个块，然后提取数据部分
		size_t totalBytesToRead = sizeof(StegoHeader) + length;

		// 读取完整数据(头部+数据)
		vector<char> fullBuffer(totalBytesToRead);
		// 从随机序列的开头(offsetBits=0)开始读取
		if (!readRandomLSB(pixels, pdSize, fullBuffer.data(), totalBytesToRead, channelMaskToTry, 1, password, 0)) {
			return false;
		}

		// 提取数据部分
		memcpy(dataOut, fullBuffer.data() + sizeof(StegoHeader), length);
		return true;
	}
}

/**
 * @brief 顺序LSB写入算法
 *
 * 按顺序将数据位写入像素的最低有效位。
 *
 * @param[in,out] pixelData 像素数据缓冲区
 * @param[in] pixelDataSize 像素数据大小
 * @param[in] src 源数据缓冲区
 * @param[in] numBytes 要写入的字节数
 * @param[in] channelMask 通道掩码
 * @param[in] bitsPerChannel 每通道使用的位数(1或2)
 * @return 成功返回true，失败返回false
 */
bool StegoCore::writeSequentialLSB(unsigned char* pixelData, size_t pixelDataSize,
	const char* src, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if ((bitsPerChannel != 1 && bitsPerChannel != 2) || pixelDataSize == 0) return false;

	// 猜测图像通道数
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channels = bitCount / 8;

	// 初始化计数器
	size_t totalBits = numBytes * 8;
	size_t bitsWritten = 0;
	size_t srcByte = 0;
	int srcBit = 0;
	size_t pixByte = 0;

	// 逐位写入
	while (bitsWritten < totalBits && pixByte < pixelDataSize) {
		int ch = pixByte % channels;  // 当前通道索引

		// 检查当前通道是否在掩码中
		if ((channelMask >> ch) & 0x01) {
			unsigned char bits = 0;

			// 收集要写入的位
			for (int i = 0; i < bitsPerChannel && bitsWritten < totalBits; ++i) {
				int v = (src[srcByte] >> (7 - srcBit)) & 0x01;
				bits = (bits << 1) | v;
				++srcBit;
				if (srcBit == 8) { srcBit = 0; ++srcByte; }
				++bitsWritten;
			}

			// 写入位
			if (bitsPerChannel == 1) {
				// 1位LSB模式：替换最低位
				pixelData[pixByte] = (pixelData[pixByte] & 0xFE) | (bits & 0x01);
			}
			else {
				// 2位LSB模式：替换最低两位
				pixelData[pixByte] = (pixelData[pixByte] & 0xFC) | (bits & 0x03);
			}
		}
		++pixByte;
	}

	// 检查是否写入了所有位
	return bitsWritten == totalBits;
}

/**
 * @brief 顺序LSB读取算法
 *
 * 按顺序从像素的最低有效位读取数据位。
 *
 * @param[in] pixelData 像素数据缓冲区
 * @param[in] pixelDataSize 像素数据大小
 * @param[out] dst 目标数据缓冲区
 * @param[in] numBytes 要读取的字节数
 * @param[in] channelMask 通道掩码
 * @param[in] bitsPerChannel 每通道使用的位数(1或2)
 * @return 成功返回true，失败返回false
 */
bool StegoCore::readSequentialLSB(const unsigned char* pixelData, size_t pixelDataSize,
	char* dst, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if ((bitsPerChannel != 1 && bitsPerChannel != 2) || pixelDataSize == 0) return false;

	// 猜测图像通道数
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channels = bitCount / 8;

	// 初始化计数器
	size_t totalBits = numBytes * 8;
	size_t bitsRead = 0;
	size_t dstByte = 0;
	int dstBit = 0;
	size_t pixByte = 0;

	// 清空目标缓冲区
	memset(dst, 0, numBytes);

	// 逐位读取
	while (bitsRead < totalBits && pixByte < pixelDataSize) {
		int ch = pixByte % channels;  // 当前通道索引

		// 检查当前通道是否在掩码中
		if ((channelMask >> ch) & 0x01) {
			// 读取位
			unsigned char val = pixelData[pixByte] & ((bitsPerChannel == 1) ? 0x01 : 0x03);

			// 将读取的位写入目标缓冲区
			for (int i = bitsPerChannel - 1; i >= 0 && bitsRead < totalBits; --i) {
				dst[dstByte] |= ((val >> i) & 0x01) << (7 - dstBit);
				++dstBit; ++bitsRead;
				if (dstBit == 8) { dstBit = 0; ++dstByte; }
			}
		}
		++pixByte;
	}

	// 检查是否读取了所有位
	return bitsRead == totalBits;
}

/**
 * @brief 随机LSB写入算法
 *
 * 使用密码生成的伪随机序列确定写入位置，增强隐蔽性。
 *
 * @param[in,out] pixelData 像素数据缓冲区
 * @param[in] pixelDataSize 像素数据大小
 * @param[in] src 源数据缓冲区
 * @param[in] numBytes 要写入的字节数
 * @param[in] channelMask 通道掩码
 * @param[in] bitsPerChannel 每通道使用的位数(通常为1)
 * @param[in] password 用于生成随机序列的密码
 * @param[in] offsetBits 起始位偏移量
 * @return 成功返回true，失败返回false
 */
bool StegoCore::writeRandomLSB(unsigned char* pixelData, size_t pixelDataSize,
	const char* src, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel,
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (password.empty()) return false;

	// 生成随机位置数组
	vector<size_t> positions(pixelDataSize);
	iota(positions.begin(), positions.end(), 0);  // 填充0到pixelDataSize-1

	// 使用密码作为随机种子
	std::seed_seq seq(password.begin(), password.end());
	mt19937 rng(seq);
	shuffle(positions.begin(), positions.end(), rng);  // 随机打乱位置

	// 初始化计数器
	size_t totalBits = numBytes * 8;
	size_t bitsWritten = 0;
	size_t srcByte = 0;
	int srcBit = 0;

	// 从指定偏移量开始，逐位写入
	for (size_t i = offsetBits; bitsWritten < totalBits && i < pixelDataSize; ++i) {
		size_t idx = positions[i];  // 随机位置

		// 确定当前位置对应的通道
		int ch = idx % ((pixelDataSize % 4 == 0) ? 4 : 3);

		// 检查当前通道是否在掩码中
		if ((channelMask >> ch) & 0x01) {
			// 获取源数据位
			int v = (src[srcByte] >> (7 - srcBit)) & 0x01;

			// 写入位
			pixelData[idx] = (pixelData[idx] & 0xFE) | v;

			// 更新计数器
			++srcBit; ++bitsWritten;
			if (srcBit == 8) { srcBit = 0; ++srcByte; }
		}
	}

	// 检查是否写入了所有位
	return bitsWritten == totalBits;
}

/**
 * @brief 随机LSB读取算法
 *
 * 使用与写入相同的密码生成伪随机序列，确定读取位置。
 *
 * @param[in] pixelData 像素数据缓冲区
 * @param[in] pixelDataSize 像素数据大小
 * @param[out] dst 目标数据缓冲区
 * @param[in] numBytes 要读取的字节数
 * @param[in] channelMask 通道掩码
 * @param[in] bitsPerChannel 每通道使用的位数(通常为1)
 * @param[in] password 用于生成随机序列的密码
 * @param[in] offsetBits 起始位偏移量
 * @return 成功返回true，失败返回false
 */
bool StegoCore::readRandomLSB(const unsigned char* pixelData, size_t pixelDataSize,
	char* dst, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel,
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (password.empty()) return false;

	// 生成随机位置数组
	vector<size_t> positions(pixelDataSize);
	iota(positions.begin(), positions.end(), 0);  // 填充0到pixelDataSize-1

	// 使用密码作为随机种子
	std::seed_seq seq(password.begin(), password.end());
	mt19937 rng(seq);
	shuffle(positions.begin(), positions.end(), rng);  // 随机打乱位置

	// 初始化计数器
	size_t totalBits = numBytes * 8;
	size_t bitsRead = 0;
	size_t dstByte = 0;
	int dstBit = 0;

	// 清空目标缓冲区
	memset(dst, 0, numBytes);

	// 从指定偏移量开始，逐位读取
	for (size_t i = offsetBits; bitsRead < totalBits && i < pixelDataSize; ++i) {
		size_t idx = positions[i];  // 随机位置

		// 确定当前位置对应的通道
		int ch = idx % ((pixelDataSize % 4 == 0) ? 4 : 3);

		// 检查当前通道是否在掩码中
		if ((channelMask >> ch) & 0x01) {
			// 读取位
			int v = pixelData[idx] & 0x01;

			// 将读取的位写入目标缓冲区
			dst[dstByte] |= v << (7 - dstBit);

			// 更新计数器
			++dstBit; ++bitsRead;
			if (dstBit == 8) { dstBit = 0; ++dstByte; }
		}
	}

	// 检查是否读取了所有位
	return bitsRead == totalBits;
}