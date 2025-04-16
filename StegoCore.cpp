#include "StegoCore.h"
#include <iostream>
#include <vector>
#include <random>   // ���� mt19937 �� uniform_int_distribution (��Ȼ������shuffle)
#include <algorithm> // ���� shuffle
#include <cstring>   // ���� memcpy, memset
#include <set>       // ���� generateRandomPositions (���Ƴ�)
#include <numeric>   // ���� accumulate (��ǰδʹ��)
#include <iterator>  // ���� back_inserter (��ǰδʹ��)

using namespace std;

// CRC32���ұ� (��׼����ʽ 0xEDB88320)
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
	uint32_t crc = 0xFFFFFFFF; // ��ʼֵ
	for (size_t i = 0; i < length; i++) {
		// ʹ�ò��ұ����
		unsigned char index = static_cast<unsigned char>((crc ^ buffer[i]) & 0xFF);
		crc = (crc >> 8) ^ crcTable[index];
	}
	crc ^= 0xFFFFFFFF; // �����ȡ��
	return crc;
}

void StegoCore::xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const
{
	if (password.empty() || length == 0) {
		return; // �������Ϊ�ջ����ݳ���Ϊ0���򲻽����κβ���
	}
	size_t pwdLen = password.size();
	for (size_t i = 0; i < length; i++) {
		buffer[i] ^= password[i % pwdLen]; // ѭ��ʹ�������ֽڽ���XOR
	}
}

size_t StegoCore::calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const
{
	int bitCount = bmp.getBitCount();
	if (bitCount != 24 && bitCount != 32) return 0; // ��֧��24/32λ
	int channelCount = bitCount / 8; // 3 �� 4
	size_t pixelDataSize = bmp.getPixelDataSize();
	if (pixelDataSize == 0) return 0;

	// ����ʵ��ʹ�õ���ɫͨ������
	int usedChannels = 0;
	if (ctx.channelMask & 0x01) usedChannels++; // ��
	if (ctx.channelMask & 0x02) usedChannels++; // ��
	if (ctx.channelMask & 0x04) usedChannels++; // ��
	// Alphaͨ��(�������)ͨ����������д����������ָ�������ﲻ����

	// ȷ��ÿ������ͨ�����ص�λ��
	// ע�⣺���LSBģʽ��ǰҲֻʵ����1bit����
	int bitsPerChannel = (ctx.mode == LSB_ENHANCED) ? 2 : 1;

	// ������������
	size_t totalPixels = pixelDataSize / channelCount;

	// �����ܹ����õ����ر�����
	size_t totalBitsAvailable = totalPixels * usedChannels * bitsPerChannel;

	// ת��Ϊ�ֽ�����
	size_t capacityBytes = totalBitsAvailable / 8;

	// ��ȥͷ��ռ�õ��ֽ�
	if (capacityBytes >= sizeof(StegoHeader)) {
		return capacityBytes - sizeof(StegoHeader);
	}
	else {
		return 0; // ���������Դ��ͷ��
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

	// 1) ��������Ƿ��㹻
	size_t capacity = calculateCapacity(bmp, ctx);
	if (length > capacity) {
		cerr << "[Error] Not enough capacity in BMP image for the given data and settings." << endl;
		cerr << "  Required: " << length << " bytes, Available: " << capacity << " bytes (excluding header)." << endl;
		return false;
	}

	// 2) ׼�� StegoHeader (16�ֽ�)
	StegoHeader header;
	memcpy(header.signature, "STEG", 4); // ����ǩ��
	header.dataLength = static_cast<uint32_t>(length);
	header.stegoMode = ctx.mode;
	header.channelMask = ctx.channelMask;

	// 3) ׼������: ����ԭʼ���ݣ�Ȼ�����XOR����
	vector<char> buffer(data, data + length); // ʹ�� vector �����ڴ�
	xorEncryptBuffer(buffer.data(), length, ctx.password); // ����

	// 4) ������ܡ�ǰ�����ݵ� CRC32
	// ע�⣺CRCӦ���ڼ���ǰ���㣬��ȷ�����ܺ�����ȷУ��
	header.crc32Value = calcCRC32(data, length);


	// 5) ��ͷ���ͼ��ܺ������д�� BMP
	return writeAll(bmp, header, buffer.data(), length, ctx);
}


bool StegoCore::extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx)
{
	// ��ʼ���������
	outData = nullptr;
	outLength = 0;

	// ȷ��Ҫ���Ե�ģʽ��ͨ������
	vector<SteganoMode> modesToTry;
	vector<uint16_t> masksToTry;

	if (ctx.autoDetect) {
		// �Զ����ģʽ: ����������֪ģʽ�ͳ����������
		modesToTry = { LSB_SEQUENTIAL, LSB_ENHANCED, LSB_RANDOM };
		masksToTry = { 0x01, 0x02, 0x04, 0x07 }; // ���Ե�ͨ����ȫͨ��
	}
	else {
		// ָ��ģʽ: ֻ�����û�ָ����ģʽ������
		modesToTry = { ctx.mode };
		masksToTry = { ctx.channelMask };
	}

	bool success = false;
	for (auto modeTry : modesToTry) {
		for (auto maskTry : masksToTry) {
			// ���Զ�ȡͷ��
			StegoHeader tmpHeader;
			if (!readHeader(bmp, tmpHeader, modeTry, maskTry, ctx.password, true)) {
				continue; // ��ȡʧ�ܻ�ǩ����ƥ�䣬������һ�����
			}

			// ������ݳ����Ƿ���� (�����������ڴ�)
			const size_t MAX_REASONABLE_SIZE = 100 * 1024 * 1024; // 100MB ���ƣ��ɵ���
			if (tmpHeader.dataLength == 0 || tmpHeader.dataLength > MAX_REASONABLE_SIZE) {
				// cout << "[Debug] Header found but data length (" << tmpHeader.dataLength << ") seems unreasonable. Skipping." << endl;
				continue;
			}

			// �����ڴ����ڴ洢��ȡ������
			size_t len = tmpHeader.dataLength;
			char* buf = nullptr;
			try {
				buf = new char[len]; // �ڶ��Ϸ����ڴ�
			}
			catch (const std::bad_alloc& e) {
				cerr << "[Error] Failed to allocate memory for extracted data: " << len << " bytes. " << e.what() << endl;
				continue; // �ڴ����ʧ�ܣ�������һ�����
			}

			// ����ͷ����Ϣ�����Զ�ȡ���ݲ���
			if (!readDataSection(bmp, tmpHeader, buf, len, modeTry, maskTry, ctx.password)) {
				delete[] buf; // ��ȡ����ʧ�ܣ��ͷ��ڴ�
				continue;
			}

			// ����ȡ�����ݽ���XOR���� (ʹ��������ʱ��ͬ������)
			xorEncryptBuffer(buf, len, ctx.password);

			// ������ܺ����ݵ�CRC32������ͷ���е�ֵ�Ƚ�
			uint32_t realCrc = calcCRC32(buf, len);
			if (realCrc != tmpHeader.crc32Value) {
				// CRCУ��ʧ�ܣ�˵�����ݴ��������/ģʽ/���벻ƥ��
				// cout << "[Debug] CRC mismatch for mode=" << modeTry << ", mask=" << maskTry << ". Expected=" << tmpHeader.crc32Value << ", Calculated=" << realCrc << endl;
				delete[] buf; // �ͷ��ڴ�
				continue;
			}

			// CRCУ��ɹ�! �ҵ�����Ч����
			outData = buf;     // ������Ļ�����ָ�뽻��������
			outLength = len;   // �������ݳ���
			ctx.mode = (SteganoMode)tmpHeader.stegoMode; // ����������Ϊʵ�ʼ�⵽��ģʽ
			ctx.channelMask = tmpHeader.channelMask;   // ����������Ϊʵ�ʼ�⵽������
			success = true;
			break; // �ɹ���ȡ�������ڲ�ѭ��
		}
		if (success) {
			break; // �ɹ���ȡ���������ѭ��
		}
	}

	if (!success) {
		// ���������������϶�ʧ����
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

	return true; // ��ȡ�ɹ�
}


// ========== �ڲ���������ʵ�� ==========

bool StegoCore::writeAll(BmpImage& bmp, const StegoHeader& header,
	const char* data, size_t length, const StegoContext& ctx)
{
	// ׼��Ҫд�������������: ͷ��(16�ֽ�) + ����(length�ֽ�)
	size_t totalBytesToWrite = sizeof(StegoHeader) + length;
	vector<char> fullBuf(totalBytesToWrite);
	memcpy(fullBuf.data(), &header, sizeof(StegoHeader));
	memcpy(fullBuf.data() + sizeof(StegoHeader), data, length);

	unsigned char* pixelPtr = bmp.getPixelData();
	size_t pixelDataSize = bmp.getPixelDataSize();
	int bitsPerChannel = (ctx.mode == LSB_ENHANCED) ? 2 : 1;
	// LSB_RANDOM ��ǰҲֻ֧�� 1 bit
	if (ctx.mode == LSB_RANDOM) bitsPerChannel = 1;

	bool ret = false;
	if (ctx.mode == LSB_SEQUENTIAL || ctx.mode == LSB_ENHANCED) {
		ret = writeSequentialLSB(pixelPtr, pixelDataSize,
			fullBuf.data(), fullBuf.size(),
			ctx.channelMask, bitsPerChannel);
	}
	else { // LSB_RANDOM
		// ���ģʽд�룬��ƫ��0��ʼ (ͷ��������һ��д��)
		ret = writeRandomLSB(pixelPtr, pixelDataSize,
			fullBuf.data(), fullBuf.size(),
			ctx.channelMask, 1, // ǿ��1 bit
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
	// ׼��һ��16�ֽڵĻ��������洢��ȡ��ͷ������
	vector<char> buf(sizeof(StegoHeader), 0);

	const unsigned char* pixelPtr = bmp.getPixelData();
	size_t pixelDataSize = bmp.getPixelDataSize();
	if (pixelDataSize == 0) return false;

	int bitsPerChannel = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	// LSB_RANDOM ��ǰҲֻ֧�� 1 bit
	if (modeToTry == LSB_RANDOM) bitsPerChannel = 1;

	bool ret = false;
	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		ret = readSequentialLSB(pixelPtr, pixelDataSize,
			buf.data(), buf.size(), // ��ȡ16�ֽ�
			channelMaskToTry, bitsPerChannel);
	}
	else { // LSB_RANDOM
		ret = readRandomLSB(pixelPtr, pixelDataSize,
			buf.data(), buf.size(), // ��ȡ16�ֽ�
			channelMaskToTry, 1, // ǿ��1 bit
			password, 0);       // ��ƫ��0��ʼ��ȡͷ��
	}

	if (!ret) {
		return false; // ��ȡ����ʧ��
	}

	// ����ȡ�����ݸ��Ƶ� headerOut �ṹ��
	memcpy(&headerOut, buf.data(), sizeof(StegoHeader));

	// �����Ҫ�����ǩ���Ƿ�Ϊ "STEG"
	if (checkSignature && memcmp(headerOut.signature, "STEG", 4) != 0) {
		return false; // ǩ����ƥ��
	}

	return true; // ��ȡ�ɹ� (�����Ƿ���Ч���һ�����)
}


bool StegoCore::readDataSection(const BmpImage& bmp, const StegoHeader& header,
	char* dataOut, size_t length,
	SteganoMode modeToTry, uint16_t channelMaskToTry,
	const std::string& password) const
{
	if (length == 0) return true; // �������Ϊ0��ֱ�ӷ��سɹ�

	const unsigned char* pixelPtr = bmp.getPixelData();
	size_t pixelDataSize = bmp.getPixelDataSize();
	if (pixelDataSize == 0 && length > 0) return false; // û���������ݵ���Ҫ��ȡ

	int bitsPerChannel = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	// LSB_RANDOM ��ǰҲֻ֧�� 1 bit
	if (modeToTry == LSB_RANDOM) bitsPerChannel = 1;

	// ����ͷ��ռ�õı���������Ϊ���ݶ�ȡ����ʼƫ����
	size_t headerSizeInBits = sizeof(StegoHeader) * 8;

	bool ret = false;
	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		// ˳��ģʽ�£���Ҫ�ҵ�ͷ��������λ�ü�����ȡ
		// �����ʵ�֣����¶�ȡ������ͷ+���ݣ���Ȼ������ͷ��
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
		// ���ģʽ�£�ʹ��ͷ����������Ϊƫ����������ȡ
		ret = readRandomLSB(pixelPtr, pixelDataSize,
			dataOut, length, // ֱ�Ӷ�ȡ���ݵ�Ŀ�껺����
			channelMaskToTry, 1, // ǿ��1 bit
			password,
			headerSizeInBits); // ��ͷ��֮��ʼ��ȡ
	}

	return ret;
}

// ========== LSB ���Ķ�д���� ==========

bool StegoCore::writeSequentialLSB(unsigned char* pixelData, size_t pixelDataSize,
	const char* src, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if (bitsPerChannel != 1 && bitsPerChannel != 2) return false; // ֻ֧��1��2λ

	int bitCount = 0; // �����Զ����λ��
	// ͨ�� BMP ͼ���СӦ���� Width * Height * (BytesPerPixel) + padding
	// ͨ�� total size / (3 �� 4) ���²� BytesPerPixel ������ɿ��ķ�ʽ
	// ���ɿ����Ǵ� BmpImage �����ȡ biBitCount
	// �����������ߴ���� bmp ��������Ч�� BmpImage ����
	// ���˺���ǩ��û�� BmpImage ����ֻ�� pixelData���˻ص����ڴ�С�Ĳ²⡣
	if (pixelDataSize > 0) {
		// �򵥵Ĳ²ⷽ��������ܴ�С�� 4 �ı�������������Ϊ�� 32 λ (4 ͨ��)
		// ������Ϊ�� 24 λ (3 ͨ��)���ⲻ��ȫ׼ȷ(���ܴ���padding)��
		bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	}
	if (bitCount == 0) return false; // �޷�ȷ��ͨ����

	int channelsPerPixel = bitCount / 8; // 3 �� 4

	size_t totalBitsToWrite = numBytes * 8;
	size_t bitsWritten = 0;
	size_t srcByteIndex = 0;
	int srcBitIndex = 0; // �����λ��ʼ (7 down to 0)

	size_t pixelByteIndex = 0;

	while (bitsWritten < totalBitsToWrite && pixelByteIndex < pixelDataSize) {
		// ȷ����ǰ�����ֽ������ĸ�ͨ�� (0=B, 1=G, 2=R, 3=A)
		int channelIndex = pixelByteIndex % channelsPerPixel;

		// ����ͨ���Ƿ�����
		if ((channelMask >> channelIndex) & 0x01) {
			// ��Դ���ݻ�ȡ bitsPerChannel λ
			unsigned char bitsToWrite = 0;
			for (int i = 0; i < bitsPerChannel; ++i) {
				if (bitsWritten >= totalBitsToWrite) break; // �������λ��д����

				// �� src[srcByteIndex] ��ȡ�� srcBitIndex λ
				int currentBitValue = (src[srcByteIndex] >> (7 - srcBitIndex)) & 0x01;
				bitsToWrite = (bitsToWrite << 1) | currentBitValue;

				// �ƶ���Դ���ݵ���һλ
				srcBitIndex++;
				if (srcBitIndex == 8) {
					srcBitIndex = 0;
					srcByteIndex++;
				}
				bitsWritten++;
			}

			// ����ȡ���� bitsToWrite (1��2λ) д��Ŀ�������ֽڵ����λ
			if (bitsPerChannel == 1) {
				// ������λ��Ȼ������
				pixelData[pixelByteIndex] = (pixelData[pixelByteIndex] & 0xFE) | (bitsToWrite & 0x01);
			}
			else { // bitsPerChannel == 2
				// ��������λ��Ȼ������
				pixelData[pixelByteIndex] = (pixelData[pixelByteIndex] & 0xFC) | (bitsToWrite & 0x03);
			}
		}

		pixelByteIndex++; // �ƶ�����һ�������ֽ�
	}

	// ����Ƿ����б��ض��ɹ�д��
	return bitsWritten == totalBitsToWrite;
}


bool StegoCore::readSequentialLSB(const unsigned char* pixelData, size_t pixelDataSize,
	char* dst, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if (bitsPerChannel != 1 && bitsPerChannel != 2) return false;
	if (pixelDataSize == 0) return false;

	// ͬ�������Բ²�ͨ����
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channelsPerPixel = bitCount / 8;

	size_t totalBitsToRead = numBytes * 8;
	size_t bitsRead = 0;
	size_t dstByteIndex = 0;
	int dstBitIndex = 0; // �����λ��ʼ�� (0 to 7)

	memset(dst, 0, numBytes); // ��ʼ��Ŀ�껺����

	size_t pixelByteIndex = 0;

	while (bitsRead < totalBitsToRead && pixelByteIndex < pixelDataSize) {
		int channelIndex = pixelByteIndex % channelsPerPixel;

		if ((channelMask >> channelIndex) & 0x01) {
			// �ӵ�ǰ�����ֽڶ�ȡ bitsPerChannel λ
			unsigned char readValue = 0;
			if (bitsPerChannel == 1) {
				readValue = pixelData[pixelByteIndex] & 0x01;
			}
			else { // bitsPerChannel == 2
				readValue = pixelData[pixelByteIndex] & 0x03;
			}

			// ����ȡ����λд��Ŀ�껺���� dst
			for (int i = 0; i < bitsPerChannel; ++i) {
				if (bitsRead >= totalBitsToRead) break;

				// �� readValue ����ȡ�� i λ (�Ӹ�λ����λ��ȡ����ΪreadValue�ǵ�λ�����)
				int currentBitValue = (readValue >> (bitsPerChannel - 1 - i)) & 0x01;

				// ���� dst[dstByteIndex] �ĵ� dstBitIndex λ
				dst[dstByteIndex] |= (currentBitValue << (7 - dstBitIndex));

				// �ƶ���Ŀ���ֽڵ���һλ
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
	uint16_t channelMask, int bitsPerChannel, // bitsPerChannel ��ǰ�̶�Ϊ 1
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (bitsPerChannel != 1) { // ��ǰ���ģʽֻ֧��1bit
		cerr << "[Error] Random LSB currently only supports 1 bit per channel." << endl;
		return false;
	}


	// 1. ȷ�����õ������ֽ�λ��
	vector<size_t> availablePositions;
	// �²�ͨ����
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channelsPerPixel = bitCount / 8;

	for (size_t i = 0; i < pixelDataSize; ++i) {
		int channelIndex = i % channelsPerPixel;
		if ((channelMask >> channelIndex) & 0x01) {
			availablePositions.push_back(i);
		}
	}

	// 2. ��������Ƿ��㹻 (������ʼƫ��)
	size_t totalBitsToWrite = numBytes * 8;
	if (offsetBits + totalBitsToWrite > availablePositions.size()) {
		cerr << "[Error] Not enough capacity for random LSB with the given offset." << endl;
		return false;
	}

	// 3. ʹ�������������ӣ������ҿ���λ��
	uint32_t seedVal = password.empty() ? 0 : calcCRC32(password.c_str(), password.size());
	mt19937 rng(seedVal);
	shuffle(availablePositions.begin(), availablePositions.end(), rng);

	// 4. д������
	size_t bitsWritten = 0;
	size_t srcByteIndex = 0;
	int srcBitIndex = 0; // 7 down to 0

	while (bitsWritten < totalBitsToWrite) {
		// ��ȡ��ǰҪд�������λ�� (����ƫ��)
		size_t currentPixelIndex = availablePositions[offsetBits + bitsWritten];

		// ��Դ���ݻ�ȡ1bit
		int bitValue = (src[srcByteIndex] >> (7 - srcBitIndex)) & 0x01;

		// д���������ݵ����λ
		pixelData[currentPixelIndex] = (pixelData[currentPixelIndex] & 0xFE) | bitValue;

		// �ƶ���Դ���ݵ���һλ
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
	uint16_t channelMask, int bitsPerChannel, // bitsPerChannel ��ǰ�̶�Ϊ 1
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (bitsPerChannel != 1) { // ��ǰ���ģʽֻ֧��1bit
		return false;
	}

	// 1. ȷ�����õ������ֽ�λ�� (��д��ʱ�߼���ͬ)
	vector<size_t> availablePositions;
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channelsPerPixel = bitCount / 8;
	for (size_t i = 0; i < pixelDataSize; ++i) {
		int channelIndex = i % channelsPerPixel;
		if ((channelMask >> channelIndex) & 0x01) {
			availablePositions.push_back(i);
		}
	}

	// 2. ����Ƿ����㹻��λ�ÿɹ���ȡ (����ƫ��)
	size_t totalBitsToRead = numBytes * 8;
	if (offsetBits + totalBitsToRead > availablePositions.size()) {
		// cerr << "[Error] Not enough available pixels to read the requested amount of data with random LSB." << endl;
		return false; // λ�ò��㣬�޷���ȡ
	}

	// 3. ʹ�������������ӣ�����λ�� (������д��ʱ��ȫһ��)
	uint32_t seedVal = password.empty() ? 0 : calcCRC32(password.c_str(), password.size());
	mt19937 rng(seedVal);
	shuffle(availablePositions.begin(), availablePositions.end(), rng);

	// 4. ��ȡ����
	memset(dst, 0, numBytes); // ��ʼ��Ŀ�껺����
	size_t bitsRead = 0;
	size_t dstByteIndex = 0;
	int dstBitIndex = 0; // 0 to 7

	while (bitsRead < totalBitsToRead) {
		// ��ȡ��ǰҪ��ȡ������λ�� (����ƫ��)
		size_t currentPixelIndex = availablePositions[offsetBits + bitsRead];

		// ��ȡ�������ݵ����λ
		int bitValue = pixelData[currentPixelIndex] & 0x01;

		// д��Ŀ�껺����
		dst[dstByteIndex] |= (bitValue << (7 - dstBitIndex));

		// �ƶ���Ŀ���ֽڵ���һλ
		dstBitIndex++;
		if (dstBitIndex == 8) {
			dstBitIndex = 0;
			dstByteIndex++;
		}
		bitsRead++;
	}

	return true;
}