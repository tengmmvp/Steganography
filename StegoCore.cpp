#include "StegoCore.h"
#include <iostream>
#include <vector>
#include <random>   // 用于 mt19937 和 uniform_int_distribution (虽然现在用shuffle)
#include <algorithm> // 用于 shuffle
#include <cstring>   // 用于 memcpy, memset
#include <set>       // 用于 generateRandomPositions (已移除)
#include <numeric>   // 包含 accumulate (当前未使用)
#include <iterator>  // 包含 back_inserter (当前未使用)

using namespace std;

// CRC32查找表 (标准多项式 0xEDB88320)
static const uint32_t crcTable[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

uint32_t StegoCore::calcCRC32(const char* buffer, size_t length) const
{
	uint32_t crc = 0xFFFFFFFF; // 初始值
	for (size_t i = 0; i < length; i++) {
		// 使用查找表计算
		unsigned char index = static_cast<unsigned char>((crc ^ buffer[i]) & 0xFF);
		crc = (crc >> 8) ^ crcTable[index];
	}
	crc ^= 0xFFFFFFFF; // 最后结果取反
	return crc;
}

void StegoCore::xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const
{
	if (password.empty() || length == 0) {
		return; // 如果密码为空或数据长度为0，则不进行任何操作
	}
	size_t pwdLen = password.size();
	for (size_t i = 0; i < length; i++) {
		buffer[i] ^= password[i % pwdLen]; // 循环使用密码字节进行XOR
	}
}

size_t StegoCore::calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const
{
	int bitCount = bmp.getBitCount();
	if (bitCount != 24 && bitCount != 32) return 0; // 仅支持24/32位
	int channelCount = bitCount / 8; // 3 或 4
	size_t pixelDataSize = bmp.getPixelDataSize();
	if (pixelDataSize == 0) return 0;

	// 计算实际使用的颜色通道数量
	int usedChannels = 0;
	if (ctx.channelMask & 0x01) usedChannels++; // 蓝
	if (ctx.channelMask & 0x02) usedChannels++; // 绿
	if (ctx.channelMask & 0x04) usedChannels++; // 红
	// Alpha通道(如果存在)通常不用于隐写，除非特殊指定，这里不计入

	// 确定每个可用通道隐藏的位数
	// 注意：随机LSB模式当前也只实现了1bit隐藏
	int bitsPerChannel = (ctx.mode == LSB_ENHANCED) ? 2 : 1;

	// 计算总像素数
	size_t totalPixels = pixelDataSize / channelCount;

	// 计算总共可用的隐藏比特数
	size_t totalBitsAvailable = totalPixels * usedChannels * bitsPerChannel;

	// 转换为字节容量
	size_t capacityBytes = totalBitsAvailable / 8;

	// 减去头部占用的字节
	if (capacityBytes >= sizeof(StegoHeader)) {
		return capacityBytes - sizeof(StegoHeader);
	}
	else {
		return 0; // 容量不足以存放头部
	}
}

bool StegoCore::hideData(BmpImage& bmp, const char* data, size_t length, const StegoContext& ctx)
{
	if (length == 0) {
		cerr << "[Error] Data to hide has zero length." << endl;
		return false;
	}
	if (length > numeric_limits<uint32_t>::max()) {
		cerr << "[Error] Data length exceeds maximum allowed (uint32_t)." << endl;
		return false;
	}

	// 1) 检查容量是否足够
	size_t capacity = calculateCapacity(bmp, ctx);
	if (length > capacity) {
		cerr << "[Error] Not enough capacity in BMP image for the given data and settings." << endl;
		cerr << "  Required: " << length << " bytes, Available: " << capacity << " bytes (excluding header)." << endl;
		return false;
	}

	// 2) 准备 StegoHeader (16字节)
	StegoHeader header;
	memcpy(header.signature, "STEG", 4); // 设置签名
	header.dataLength = static_cast<uint32_t>(length);
	header.stegoMode = ctx.mode;
	header.channelMask = ctx.channelMask;

	// 3) 准备数据: 复制原始数据，然后进行XOR加密
	vector<char> buffer(data, data + length); // 使用 vector 管理内存
	xorEncryptBuffer(buffer.data(), length, ctx.password); // 加密

	// 4) 计算加密【前】数据的 CRC32
	// 注意：CRC应该在加密前计算，以确保解密后能正确校验
	header.crc32Value = calcCRC32(data, length);


	// 5) 将头部和加密后的数据写入 BMP
	return writeAll(bmp, header, buffer.data(), length, ctx);
}


bool StegoCore::extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx)
{
	// 初始化输出参数
	outData = nullptr;
	outLength = 0;

	// 确定要尝试的模式和通道掩码
	vector<SteganoMode> modesToTry;
	vector<uint16_t> masksToTry;

	if (ctx.autoDetect) {
		// 自动检测模式: 尝试所有已知模式和常用掩码组合
		modesToTry = { LSB_SEQUENTIAL, LSB_ENHANCED, LSB_RANDOM };
		masksToTry = { 0x01, 0x02, 0x04, 0x07 }; // 尝试单通道和全通道
	}
	else {
		// 指定模式: 只尝试用户指定的模式和掩码
		modesToTry = { ctx.mode };
		masksToTry = { ctx.channelMask };
	}

	bool success = false;
	for (auto modeTry : modesToTry) {
		for (auto maskTry : masksToTry) {
			// 尝试读取头部
			StegoHeader tmpHeader;
			if (!readHeader(bmp, tmpHeader, modeTry, maskTry, ctx.password, true)) {
				continue; // 读取失败或签名不匹配，尝试下一个组合
			}

			// 检查数据长度是否合理 (避免分配过多内存)
			const size_t MAX_REASONABLE_SIZE = 100 * 1024 * 1024; // 100MB 限制，可调整
			if (tmpHeader.dataLength == 0 || tmpHeader.dataLength > MAX_REASONABLE_SIZE) {
				// cout << "[Debug] Header found but data length (" << tmpHeader.dataLength << ") seems unreasonable. Skipping." << endl;
				continue;
			}

			// 分配内存用于存储提取的数据
			size_t len = tmpHeader.dataLength;
			char* buf = nullptr;
			try {
				buf = new char[len]; // 在堆上分配内存
			}
			catch (const std::bad_alloc& e) {
				cerr << "[Error] Failed to allocate memory for extracted data: " << len << " bytes. " << e.what() << endl;
				continue; // 内存分配失败，尝试下一个组合
			}

			// 根据头部信息，尝试读取数据部分
			if (!readDataSection(bmp, tmpHeader, buf, len, modeTry, maskTry, ctx.password)) {
				delete[] buf; // 读取数据失败，释放内存
				continue;
			}

			// 对提取的数据进行XOR解密 (使用与隐藏时相同的密码)
			xorEncryptBuffer(buf, len, ctx.password);

			// 计算解密后数据的CRC32，并与头部中的值比较
			uint32_t realCrc = calcCRC32(buf, len);
			if (realCrc != tmpHeader.crc32Value) {
				// CRC校验失败，说明数据错误或密码/模式/掩码不匹配
				// cout << "[Debug] CRC mismatch for mode=" << modeTry << ", mask=" << maskTry << ". Expected=" << tmpHeader.crc32Value << ", Calculated=" << realCrc << endl;
				delete[] buf; // 释放内存
				continue;
			}

			// CRC校验成功! 找到了有效数据
			outData = buf;     // 将分配的缓冲区指针交给调用者
			outLength = len;   // 设置数据长度
			ctx.mode = (SteganoMode)tmpHeader.stegoMode; // 更新上下文为实际检测到的模式
			ctx.channelMask = tmpHeader.channelMask;   // 更新上下文为实际检测到的掩码
			success = true;
			break; // 成功提取，跳出内层循环
		}
		if (success) {
			break; // 成功提取，跳出外层循环
		}
	}

	if (!success) {
		// 如果尝试了所有组合都失败了
		outData = nullptr;
		outLength = 0;
		if (!ctx.autoDetect) {
			cerr << "[Warning] Failed to extract data with the specified settings." << endl;
		}
		else {
			// cout << "[Info] No hidden data detected or settings/password mismatch." << endl;
		}
		return false;
	}

	return true; // 提取成功
}


// ========== 内部辅助函数实现 ==========

bool StegoCore::writeAll(BmpImage& bmp, const StegoHeader& header,
	const char* data, size_t length, const StegoContext& ctx)
{
	// 准备要写入的完整数据流: 头部(16字节) + 数据(length字节)
	size_t totalBytesToWrite = sizeof(StegoHeader) + length;
	vector<char> fullBuf(totalBytesToWrite);
	memcpy(fullBuf.data(), &header, sizeof(StegoHeader));
	memcpy(fullBuf.data() + sizeof(StegoHeader), data, length);

	unsigned char* pixelPtr = bmp.getPixelData();
	size_t pixelDataSize = bmp.getPixelDataSize();
	int bitsPerChannel = (ctx.mode == LSB_ENHANCED) ? 2 : 1;
	// LSB_RANDOM 当前也只支持 1 bit
	if (ctx.mode == LSB_RANDOM) bitsPerChannel = 1;

	bool ret = false;
	if (ctx.mode == LSB_SEQUENTIAL || ctx.mode == LSB_ENHANCED) {
		ret = writeSequentialLSB(pixelPtr, pixelDataSize,
			fullBuf.data(), fullBuf.size(),
			ctx.channelMask, bitsPerChannel);
	}
	else { // LSB_RANDOM
		// 随机模式写入，从偏移0开始 (头部和数据一起写入)
		ret = writeRandomLSB(pixelPtr, pixelDataSize,
			fullBuf.data(), fullBuf.size(),
			ctx.channelMask, 1, // 强制1 bit
			ctx.password, 0);
	}

	if (!ret) {
		cerr << "[Error] Failed to write data block into BMP pixels." << endl;
		return false;
	}

	return true;
}


bool StegoCore::readHeader(const BmpImage& bmp, StegoHeader& headerOut,
	SteganoMode modeToTry, uint16_t channelMaskToTry,
	const std::string& password, bool checkSignature) const
{
	// 准备一个16字节的缓冲区来存储读取的头部数据
	vector<char> buf(sizeof(StegoHeader), 0);

	const unsigned char* pixelPtr = bmp.getPixelData();
	size_t pixelDataSize = bmp.getPixelDataSize();
	if (pixelDataSize == 0) return false;

	int bitsPerChannel = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	// LSB_RANDOM 当前也只支持 1 bit
	if (modeToTry == LSB_RANDOM) bitsPerChannel = 1;

	bool ret = false;
	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		ret = readSequentialLSB(pixelPtr, pixelDataSize,
			buf.data(), buf.size(), // 读取16字节
			channelMaskToTry, bitsPerChannel);
	}
	else { // LSB_RANDOM
		ret = readRandomLSB(pixelPtr, pixelDataSize,
			buf.data(), buf.size(), // 读取16字节
			channelMaskToTry, 1, // 强制1 bit
			password, 0);       // 从偏移0开始读取头部
	}

	if (!ret) {
		return false; // 读取像素失败
	}

	// 将读取的数据复制到 headerOut 结构体
	memcpy(&headerOut, buf.data(), sizeof(StegoHeader));

	// 如果需要，检查签名是否为 "STEG"
	if (checkSignature && memcmp(headerOut.signature, "STEG", 4) != 0) {
		return false; // 签名不匹配
	}

	return true; // 读取成功 (内容是否有效需进一步检查)
}


bool StegoCore::readDataSection(const BmpImage& bmp, const StegoHeader& header,
	char* dataOut, size_t length,
	SteganoMode modeToTry, uint16_t channelMaskToTry,
	const std::string& password) const
{
	if (length == 0) return true; // 如果长度为0，直接返回成功

	const unsigned char* pixelPtr = bmp.getPixelData();
	size_t pixelDataSize = bmp.getPixelDataSize();
	if (pixelDataSize == 0 && length > 0) return false; // 没有像素数据但需要读取

	int bitsPerChannel = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	// LSB_RANDOM 当前也只支持 1 bit
	if (modeToTry == LSB_RANDOM) bitsPerChannel = 1;

	// 计算头部占用的比特数，作为数据读取的起始偏移量
	size_t headerSizeInBits = sizeof(StegoHeader) * 8;

	bool ret = false;
	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		// 顺序模式下，需要找到头部结束的位置继续读取
		// 这里简化实现：重新读取整个（头+数据），然后跳过头部
		size_t totalBytesToRead = sizeof(StegoHeader) + length;
		vector<char> fullBuf(totalBytesToRead);
		ret = readSequentialLSB(pixelPtr, pixelDataSize,
			fullBuf.data(), totalBytesToRead,
			channelMaskToTry, bitsPerChannel);
		if (ret) {
			memcpy(dataOut, fullBuf.data() + sizeof(StegoHeader), length);
		}
	}
	else { // LSB_RANDOM
		// 随机模式下，使用头部比特数作为偏移量继续读取
		ret = readRandomLSB(pixelPtr, pixelDataSize,
			dataOut, length, // 直接读取数据到目标缓冲区
			channelMaskToTry, 1, // 强制1 bit
			password,
			headerSizeInBits); // 从头部之后开始读取
	}

	return ret;
}

// ========== LSB 核心读写函数 ==========

bool StegoCore::writeSequentialLSB(unsigned char* pixelData, size_t pixelDataSize,
	const char* src, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if (bitsPerChannel != 1 && bitsPerChannel != 2) return false; // 只支持1或2位

	int bitCount = 0; // 尝试自动检测位数
	// 通常 BMP 图像大小应该是 Width * Height * (BytesPerPixel) + padding
	// 通过 total size / (3 或 4) 来猜测 BytesPerPixel 不是最可靠的方式
	// 更可靠的是从 BmpImage 对象获取 biBitCount
	// 这里假设调用者传入的 bmp 参数是有效的 BmpImage 对象
	// 但此函数签名没有 BmpImage 对象，只有 pixelData。退回到基于大小的猜测。
	if (pixelDataSize > 0) {
		// 简单的猜测方法：如果总大小是 4 的倍数，倾向于认为是 32 位 (4 通道)
		// 否则认为是 24 位 (3 通道)。这不完全准确(可能存在padding)。
		bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	}
	if (bitCount == 0) return false; // 无法确定通道数

	int channelsPerPixel = bitCount / 8; // 3 或 4

	size_t totalBitsToWrite = numBytes * 8;
	size_t bitsWritten = 0;
	size_t srcByteIndex = 0;
	int srcBitIndex = 0; // 从最高位开始 (7 down to 0)

	size_t pixelByteIndex = 0;

	while (bitsWritten < totalBitsToWrite && pixelByteIndex < pixelDataSize) {
		// 确定当前像素字节属于哪个通道 (0=B, 1=G, 2=R, 3=A)
		int channelIndex = pixelByteIndex % channelsPerPixel;

		// 检查此通道是否启用
		if ((channelMask >> channelIndex) & 0x01) {
			// 从源数据获取 bitsPerChannel 位
			unsigned char bitsToWrite = 0;
			for (int i = 0; i < bitsPerChannel; ++i) {
				if (bitsWritten >= totalBitsToWrite) break; // 如果所有位都写完了

				// 从 src[srcByteIndex] 获取第 srcBitIndex 位
				int currentBitValue = (src[srcByteIndex] >> (7 - srcBitIndex)) & 0x01;
				bitsToWrite = (bitsToWrite << 1) | currentBitValue;

				// 移动到源数据的下一位
				srcBitIndex++;
				if (srcBitIndex == 8) {
					srcBitIndex = 0;
					srcByteIndex++;
				}
				bitsWritten++;
			}

			// 将获取到的 bitsToWrite (1或2位) 写入目标像素字节的最低位
			if (bitsPerChannel == 1) {
				// 清除最低位，然后设置
				pixelData[pixelByteIndex] = (pixelData[pixelByteIndex] & 0xFE) | (bitsToWrite & 0x01);
			}
			else { // bitsPerChannel == 2
				// 清除最低两位，然后设置
				pixelData[pixelByteIndex] = (pixelData[pixelByteIndex] & 0xFC) | (bitsToWrite & 0x03);
			}
		}

		pixelByteIndex++; // 移动到下一个像素字节
	}

	// 检查是否所有比特都成功写入
	return bitsWritten == totalBitsToWrite;
}


bool StegoCore::readSequentialLSB(const unsigned char* pixelData, size_t pixelDataSize,
	char* dst, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if (bitsPerChannel != 1 && bitsPerChannel != 2) return false;
	if (pixelDataSize == 0) return false;

	// 同样，尝试猜测通道数
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channelsPerPixel = bitCount / 8;

	size_t totalBitsToRead = numBytes * 8;
	size_t bitsRead = 0;
	size_t dstByteIndex = 0;
	int dstBitIndex = 0; // 从最高位开始填 (0 to 7)

	memset(dst, 0, numBytes); // 初始化目标缓冲区

	size_t pixelByteIndex = 0;

	while (bitsRead < totalBitsToRead && pixelByteIndex < pixelDataSize) {
		int channelIndex = pixelByteIndex % channelsPerPixel;

		if ((channelMask >> channelIndex) & 0x01) {
			// 从当前像素字节读取 bitsPerChannel 位
			unsigned char readValue = 0;
			if (bitsPerChannel == 1) {
				readValue = pixelData[pixelByteIndex] & 0x01;
			}
			else { // bitsPerChannel == 2
				readValue = pixelData[pixelByteIndex] & 0x03;
			}

			// 将读取到的位写入目标缓冲区 dst
			for (int i = 0; i < bitsPerChannel; ++i) {
				if (bitsRead >= totalBitsToRead) break;

				// 从 readValue 中提取第 i 位 (从高位到低位提取，因为readValue是低位对齐的)
				int currentBitValue = (readValue >> (bitsPerChannel - 1 - i)) & 0x01;

				// 设置 dst[dstByteIndex] 的第 dstBitIndex 位
				dst[dstByteIndex] |= (currentBitValue << (7 - dstBitIndex));

				// 移动到目标字节的下一位
				dstBitIndex++;
				if (dstBitIndex == 8) {
					dstBitIndex = 0;
					dstByteIndex++;
				}
				bitsRead++;
			}
		}
		pixelByteIndex++;
	}

	return bitsRead == totalBitsToRead;
}


bool StegoCore::writeRandomLSB(unsigned char* pixelData, size_t pixelDataSize,
	const char* src, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel, // bitsPerChannel 当前固定为 1
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (bitsPerChannel != 1) { // 当前随机模式只支持1bit
		cerr << "[Error] Random LSB currently only supports 1 bit per channel." << endl;
		return false;
	}


	// 1. 确定可用的像素字节位置
	vector<size_t> availablePositions;
	// 猜测通道数
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channelsPerPixel = bitCount / 8;

	for (size_t i = 0; i < pixelDataSize; ++i) {
		int channelIndex = i % channelsPerPixel;
		if ((channelMask >> channelIndex) & 0x01) {
			availablePositions.push_back(i);
		}
	}

	// 2. 检查容量是否足够 (考虑起始偏移)
	size_t totalBitsToWrite = numBytes * 8;
	if (offsetBits + totalBitsToWrite > availablePositions.size()) {
		cerr << "[Error] Not enough capacity for random LSB with the given offset." << endl;
		return false;
	}

	// 3. 使用密码生成种子，并打乱可用位置
	uint32_t seedVal = password.empty() ? 0 : calcCRC32(password.c_str(), password.size());
	mt19937 rng(seedVal);
	shuffle(availablePositions.begin(), availablePositions.end(), rng);

	// 4. 写入数据
	size_t bitsWritten = 0;
	size_t srcByteIndex = 0;
	int srcBitIndex = 0; // 7 down to 0

	while (bitsWritten < totalBitsToWrite) {
		// 获取当前要写入的像素位置 (考虑偏移)
		size_t currentPixelIndex = availablePositions[offsetBits + bitsWritten];

		// 从源数据获取1bit
		int bitValue = (src[srcByteIndex] >> (7 - srcBitIndex)) & 0x01;

		// 写入像素数据的最低位
		pixelData[currentPixelIndex] = (pixelData[currentPixelIndex] & 0xFE) | bitValue;

		// 移动到源数据的下一位
		srcBitIndex++;
		if (srcBitIndex == 8) {
			srcBitIndex = 0;
			srcByteIndex++;
		}
		bitsWritten++;
	}

	return true;
}


bool StegoCore::readRandomLSB(const unsigned char* pixelData, size_t pixelDataSize,
	char* dst, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel, // bitsPerChannel 当前固定为 1
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (bitsPerChannel != 1) { // 当前随机模式只支持1bit
		return false;
	}

	// 1. 确定可用的像素字节位置 (与写入时逻辑相同)
	vector<size_t> availablePositions;
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channelsPerPixel = bitCount / 8;
	for (size_t i = 0; i < pixelDataSize; ++i) {
		int channelIndex = i % channelsPerPixel;
		if ((channelMask >> channelIndex) & 0x01) {
			availablePositions.push_back(i);
		}
	}

	// 2. 检查是否有足够的位置可供读取 (考虑偏移)
	size_t totalBitsToRead = numBytes * 8;
	if (offsetBits + totalBitsToRead > availablePositions.size()) {
		// cerr << "[Error] Not enough available pixels to read the requested amount of data with random LSB." << endl;
		return false; // 位置不足，无法读取
	}

	// 3. 使用密码生成种子，打乱位置 (必须与写入时完全一致)
	uint32_t seedVal = password.empty() ? 0 : calcCRC32(password.c_str(), password.size());
	mt19937 rng(seedVal);
	shuffle(availablePositions.begin(), availablePositions.end(), rng);

	// 4. 读取数据
	memset(dst, 0, numBytes); // 初始化目标缓冲区
	size_t bitsRead = 0;
	size_t dstByteIndex = 0;
	int dstBitIndex = 0; // 0 to 7

	while (bitsRead < totalBitsToRead) {
		// 获取当前要读取的像素位置 (考虑偏移)
		size_t currentPixelIndex = availablePositions[offsetBits + bitsRead];

		// 读取像素数据的最低位
		int bitValue = pixelData[currentPixelIndex] & 0x01;

		// 写入目标缓冲区
		dst[dstByteIndex] |= (bitValue << (7 - dstBitIndex));

		// 移动到目标字节的下一位
		dstBitIndex++;
		if (dstBitIndex == 8) {
			dstBitIndex = 0;
			dstByteIndex++;
		}
		bitsRead++;
	}

	return true;
}