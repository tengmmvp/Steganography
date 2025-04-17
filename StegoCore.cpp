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
 * @brief BMPͼ��LSB��д�㷨����ʵ��
 * @version 1.0
 * @copyright Copyright 2025, ��Ϣ���ص�6��
 *
 * ���ļ�ʵ����StegoCore������й��ܣ�������
 * 1. ����LSB��������������ȡ�㷨
 * 2. ���ݼ�����������У��
 * 3. ������дģʽ�ľ���ʵ��
 */

using namespace std;

/**
 * @brief CRC32У���256�
 *
 * ���ڿ��ټ���CRC32У��ֵ��Ԥ��������ڶ���ʽ0xEDB88320
 * ����ÿ��ֵ��Ӧһ�����ܵ��ֽ�ֵ(0-255)��CRC32���ּ�����
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
 * @brief �������ݵ�CRC32У��ֵ
 *
 * ʹ�ñ�׼CRC32�㷨�������ݻ�������У��ֵ��������֤���������ԡ�
 *
 * @param[in] buffer �������ݻ�����
 * @param[in] length ���ݳ���(�ֽ�)
 * @return 32λCRCУ��ֵ
 */
uint32_t StegoCore::calcCRC32(const char* buffer, size_t length) const
{
	uint32_t crc = 0xFFFFFFFF; // ��ʼֵ
	for (size_t i = 0; i < length; ++i) {
		// �������CRC32
		unsigned char idx = static_cast<unsigned char>((crc ^ buffer[i]) & 0xFF);
		crc = (crc >> 8) ^ crcTable[idx];
	}
	return crc ^ 0xFFFFFFFF; // �������ֵ
}

/**
 * @brief ʹ������Ի���������XOR����/����
 *
 * �����ݽ��м򵥵�XOR���ܣ�ʹ��������Ϊ��Կ��
 * ���ܺͽ��ܲ�����ͬ���ظ�ִ�лỹԭԭʼ���ݡ�
 *
 * @param[in,out] buffer �������������(ԭ���޸�)
 * @param[in] length ����������(�ֽ�)
 * @param[in] password ��������(������ʱ��ִ�в���)
 */
void StegoCore::xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const
{
	if (password.empty() || length == 0) return;

	size_t plen = password.size();
	for (size_t i = 0; i < length; ++i) {
		// ѭ��ʹ�������ַ�����XOR����
		buffer[i] ^= password[i % plen];
	}
}

/**
 * @brief ����BMPͼ�����д����
 *
 * ����ͼ��ߴ硢ͨ��������дģʽ��������������������
 *
 * @param[in] bmp BMPͼ�����
 * @param[in] ctx ��д�����Ĳ���
 * @return ��������������(�ֽ�)���Ѽ�ȥ16�ֽ�ͷ��
 */
size_t StegoCore::calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const
{
	// ���ͼ���ʽ�Ƿ�֧��(��֧��24λ��32λ)
	int bpp = bmp.getBitCount();
	if (bpp != 24 && bpp != 32) return 0;

	int channels = bpp / 8;
	size_t dataSize = bmp.getPixelDataSize();
	if (dataSize == 0) return 0;

	// ����ʵ��ʹ�õ�ͨ����
	int used = 0;
	if (ctx.channelMask & 0x01) ++used; // ��ɫͨ��
	if (ctx.channelMask & 0x02) ++used; // ��ɫͨ��
	if (ctx.channelMask & 0x04) ++used; // ��ɫͨ��

	// ����ģʽȷ��ÿͨ��ʹ�õ�λ��
	int bitsPer = (ctx.mode == LSB_ENHANCED) ? 2 : 1;

	// ����������
	size_t pixels = dataSize / channels;
	size_t totalBits = pixels * used * bitsPer;
	size_t bytes = totalBits / 8;

	// ��ȥͷ����С������ʵ�ʿ�������
	return (bytes >= sizeof(StegoHeader)) ? (bytes - sizeof(StegoHeader)) : 0;
}

/**
 * @brief ���������ص�BMPͼ����
 *
 * ʵ��������д����Ҫ���̣�
 * 1. ��֤���ݳ��Ⱥ�ͼ������
 * 2. ׼����дͷ����Ϣ
 * 3. �����ݽ��м��ܴ���
 * 4. ������дģʽ������Ӧ��д�뺯��
 *
 * @param[in,out] bmp �Ѽ��ص�BMPͼ�����(�����޸�)
 * @param[in] data ���������ݵ�ֻ��ָ��
 * @param[in] length ���������ݳ���(�ֽ�)
 * @param[in] ctx ��д�����Ĳ�������
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::hideData(BmpImage& bmp, const char* data, size_t length, const StegoContext& ctx)
{
	// ��֤���ݳ���
	if (length == 0 || length > numeric_limits<uint32_t>::max()) {
		cerr << "[����] ���ݳ��Ȳ��Ϸ�: " << length << " �ֽ�" << endl;
		return false;
	}

	// ��������Ƿ��㹻
	size_t cap = calculateCapacity(bmp, ctx);
	if (length > cap) {
		cerr << "[����] ��������: ��Ҫ " << length << " �ֽڣ� ���� " << cap << " �ֽ�" << endl;
		return false;
	}

	// ׼����дͷ��
	StegoHeader header;
	memcpy(header.signature, "STEG", 4);           // ħ����ʶ
	header.dataLength = static_cast<uint32_t>(length);
	header.stegoMode = ctx.mode;
	header.channelMask = ctx.channelMask;
	header.crc32Value = calcCRC32(data, length);   // ����ԭʼ����У��ֵ

	// ���Ʋ���������
	vector<char> buf(data, data + length);
	xorEncryptBuffer(buf.data(), length, ctx.password);

	// д��ͷ���ͼ��ܺ������
	return writeAll(bmp, header, buf.data(), length, ctx);
}

/**
 * @brief ��BMPͼ����ȡ��������
 *
 * ʵ��������ȡ����Ҫ���̣�
 * 1. ���Զ�ȡ��дͷ����Ϣ
 * 2. ��֤ͷ����Ч��
 * 3. ��ȡ����������
 * 4. ��֤����������
 *
 * ֧���Զ����ģʽ���᳢�Զ�����дģʽ��ͨ����ϡ�
 *
 * @param[in] bmp �Ѽ��ص�BMPͼ�����(ֻ��)
 * @param[out] outData ������ݻ�����ָ��(�������delete[])
 * @param[out] outLength �������ʵ�ʳ���
 * @param[in,out] ctx ��д������(autoDetect=trueʱ�����mode��channelMask)
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx)
{
	// ��ʼ���������
	outData = nullptr;
	outLength = 0;

	// ȷ��Ҫ���Ե�ģʽ��ͨ������
	vector<SteganoMode> modes;
	vector<uint16_t>    masks;

	if (ctx.autoDetect) {
		// �Զ����ģʽ���������п��ܵ�ģʽ��ͨ�����
		modes = { LSB_SEQUENTIAL, LSB_ENHANCED, LSB_RANDOM };
		masks = { 0x01,0x02,0x04,0x03,0x05,0x06,0x07 }; // ���п��ܵ�ͨ�����
	}
	else {
		// ָ��ģʽ���������û�ָ����ģʽ��ͨ��
		modes = { ctx.mode };
		masks = { ctx.channelMask };
	}

	// �������п��ܵ�ģʽ��ͨ�����
	for (auto m : modes) {
		for (auto mask : masks) {
			// ���Զ�ȡͷ��
			StegoHeader hdr;
			if (!readHeader(bmp, hdr, m, mask, ctx.password, true)) continue;

			// ��֤���ݳ��Ⱥ�����
			if (hdr.dataLength == 0 || hdr.dataLength > 100 * 1024 * 1024) continue;

			// �����ڴ�
			size_t len = hdr.dataLength;
			char* buf = nullptr;
			try {
				buf = new char[len];
			}
			catch (...) {
				cerr << "[����] �ڴ����ʧ��: " << len << " �ֽ�" << endl;
				continue;
			}

			// ��ȡ���ݲ���
			if (!readDataSection(bmp, hdr, buf, len, m, mask, ctx.password)) {
				delete[] buf;
				continue;
			}

			// ��������
			xorEncryptBuffer(buf, len, ctx.password);

			// ��֤CRC32У��ֵ
			if (calcCRC32(buf, len) != hdr.crc32Value) {
				delete[] buf;
				continue;
			}

			// ��ȡ�ɹ�
			outData = buf;
			outLength = len;
			ctx.mode = static_cast<SteganoMode>(hdr.stegoMode);
			ctx.channelMask = hdr.channelMask;
			return true;
		}
	}

	// ���г��Զ�ʧ��
	if (!ctx.autoDetect) {
		cerr << "[����] ��ȡʧ�ܣ��������û�����" << endl;
	}
	return false;
}

/**
 * @brief д����������д����(ͷ��+����)
 *
 * ������дģʽѡ����Ӧ��д���㷨����ͷ��������д��ͼ��
 *
 * @param[in,out] bmp BMPͼ�����
 * @param[in] header ��дͷ���ṹ
 * @param[in] data ����������(�Ѽ���)
 * @param[in] length ���ݳ���
 * @param[in] ctx ��д������
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::writeAll(BmpImage& bmp, const StegoHeader& header,
	const char* data, size_t length, const StegoContext& ctx)
{
	// ׼���������ݻ�����(ͷ��+����)
	size_t total = sizeof(header) + length;
	vector<char> fullBuf(total);
	memcpy(fullBuf.data(), &header, sizeof(header));
	memcpy(fullBuf.data() + sizeof(header), data, length);

	// ��ȡ��������
	unsigned char* pixels = bmp.getPixelData();
	size_t pdSize = bmp.getPixelDataSize();

	// ����ģʽȷ��ÿͨ��ʹ�õ�λ��
	int bitsPer = (ctx.mode == LSB_ENHANCED) ? 2 : 1;
	if (ctx.mode == LSB_RANDOM) bitsPer = 1;

	// ����ģʽѡ��д���㷨
	bool ok = false;
	if (ctx.mode == LSB_SEQUENTIAL || ctx.mode == LSB_ENHANCED) {
		// ˳��ģʽ����ǿģʽ
		ok = writeSequentialLSB(pixels, pdSize, fullBuf.data(), total, ctx.channelMask, bitsPer);
	}
	else {
		// ���ģʽ
		ok = writeRandomLSB(pixels, pdSize, fullBuf.data(), total, ctx.channelMask, 1, ctx.password, 0);
	}

	if (!ok) {
		cerr << "[����] д����д����ʧ��" << endl;
	}
	return ok;
}

/**
 * @brief ��ȡ��дͷ����Ϣ
 *
 * ����ʹ��ָ����ģʽ��ͨ����ͼ���ж�ȡ��дͷ����
 *
 * @param[in] bmp BMPͼ�����
 * @param[out] headerOut �����ͷ���ṹ
 * @param[in] modeToTry ���Ե���дģʽ
 * @param[in] channelMaskToTry ���Ե�ͨ������
 * @param[in] password ��������
 * @param[in] checkSignature �Ƿ���ħ����ʶ
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::readHeader(const BmpImage& bmp, StegoHeader& headerOut,
	SteganoMode modeToTry, uint16_t channelMaskToTry,
	const std::string& password, bool checkSignature) const
{
	// ׼������������ͷ������
	vector<char> buf(sizeof(StegoHeader), 0);
	const unsigned char* pixels = bmp.getPixelData();
	size_t pdSize = bmp.getPixelDataSize();
	if (pdSize == 0) return false;

	// ����ģʽȷ��ÿͨ��ʹ�õ�λ��
	int bitsPer = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	if (modeToTry == LSB_RANDOM) bitsPer = 1;

	// ����ģʽѡ���ȡ�㷨
	bool ok = false;
	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		// ˳��ģʽ����ǿģʽ
		ok = readSequentialLSB(pixels, pdSize, buf.data(), buf.size(), channelMaskToTry, bitsPer);
	}
	else {
		// ���ģʽ
		ok = readRandomLSB(pixels, pdSize, buf.data(), buf.size(), channelMaskToTry, 1, password, 0);
	}
	if (!ok) return false;

	// ����ͷ������
	memcpy(&headerOut, buf.data(), sizeof(StegoHeader));

	// ��֤ħ����ʶ
	if (checkSignature && memcmp(headerOut.signature, "STEG", 4) != 0) {
		return false;
	}
	return true;
}

/**
 * @brief ��ȡ��д���ݲ���
 *
 * ������֪��ͷ����Ϣ����ͼ���ж�ȡ��д���ݲ��֡�
 *
 * @param[in] bmp BMPͼ�����
 * @param[in] header �Ѷ�ȡ����дͷ��
 * @param[out] dataOut ������ݻ�����
 * @param[in] length Ҫ��ȡ�����ݳ���
 * @param[in] modeToTry ���Ե���дģʽ
 * @param[in] channelMaskToTry ���Ե�ͨ������
 * @param[in] password ��������
 * @return �ɹ�����true��ʧ�ܷ���false
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

	// ����ģʽȷ��ÿͨ��ʹ�õ�λ��
	int bitsPer = (modeToTry == LSB_ENHANCED) ? 2 : 1;
	if (modeToTry == LSB_RANDOM) bitsPer = 1;

	if (modeToTry == LSB_SEQUENTIAL || modeToTry == LSB_ENHANCED) {
		// ˳�����ǿģʽ����ȡ����ͷ���ڵ������飬Ȼ����ȡ���ݲ���
		size_t totalBytesToRead = sizeof(StegoHeader) + length;

		// ��ȡ��������(ͷ��+����)
		vector<char> fullBuf(totalBytesToRead);
		if (!readSequentialLSB(pixels, pdSize, fullBuf.data(), totalBytesToRead, channelMaskToTry, bitsPer))
			return false;

		// ��ȡ���ݲ���
		memcpy(dataOut, fullBuf.data() + sizeof(StegoHeader), length);
		return true;
	}
	else { // LSB_RANDOM
		// ���ģʽ����ȡ����ͷ���ڵ������飬Ȼ����ȡ���ݲ���
		size_t totalBytesToRead = sizeof(StegoHeader) + length;

		// ��ȡ��������(ͷ��+����)
		vector<char> fullBuffer(totalBytesToRead);
		// ��������еĿ�ͷ(offsetBits=0)��ʼ��ȡ
		if (!readRandomLSB(pixels, pdSize, fullBuffer.data(), totalBytesToRead, channelMaskToTry, 1, password, 0)) {
			return false;
		}

		// ��ȡ���ݲ���
		memcpy(dataOut, fullBuffer.data() + sizeof(StegoHeader), length);
		return true;
	}
}

/**
 * @brief ˳��LSBд���㷨
 *
 * ��˳������λд�����ص������Чλ��
 *
 * @param[in,out] pixelData �������ݻ�����
 * @param[in] pixelDataSize �������ݴ�С
 * @param[in] src Դ���ݻ�����
 * @param[in] numBytes Ҫд����ֽ���
 * @param[in] channelMask ͨ������
 * @param[in] bitsPerChannel ÿͨ��ʹ�õ�λ��(1��2)
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::writeSequentialLSB(unsigned char* pixelData, size_t pixelDataSize,
	const char* src, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if ((bitsPerChannel != 1 && bitsPerChannel != 2) || pixelDataSize == 0) return false;

	// �²�ͼ��ͨ����
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channels = bitCount / 8;

	// ��ʼ��������
	size_t totalBits = numBytes * 8;
	size_t bitsWritten = 0;
	size_t srcByte = 0;
	int srcBit = 0;
	size_t pixByte = 0;

	// ��λд��
	while (bitsWritten < totalBits && pixByte < pixelDataSize) {
		int ch = pixByte % channels;  // ��ǰͨ������

		// ��鵱ǰͨ���Ƿ���������
		if ((channelMask >> ch) & 0x01) {
			unsigned char bits = 0;

			// �ռ�Ҫд���λ
			for (int i = 0; i < bitsPerChannel && bitsWritten < totalBits; ++i) {
				int v = (src[srcByte] >> (7 - srcBit)) & 0x01;
				bits = (bits << 1) | v;
				++srcBit;
				if (srcBit == 8) { srcBit = 0; ++srcByte; }
				++bitsWritten;
			}

			// д��λ
			if (bitsPerChannel == 1) {
				// 1λLSBģʽ���滻���λ
				pixelData[pixByte] = (pixelData[pixByte] & 0xFE) | (bits & 0x01);
			}
			else {
				// 2λLSBģʽ���滻�����λ
				pixelData[pixByte] = (pixelData[pixByte] & 0xFC) | (bits & 0x03);
			}
		}
		++pixByte;
	}

	// ����Ƿ�д��������λ
	return bitsWritten == totalBits;
}

/**
 * @brief ˳��LSB��ȡ�㷨
 *
 * ��˳������ص������Чλ��ȡ����λ��
 *
 * @param[in] pixelData �������ݻ�����
 * @param[in] pixelDataSize �������ݴ�С
 * @param[out] dst Ŀ�����ݻ�����
 * @param[in] numBytes Ҫ��ȡ���ֽ���
 * @param[in] channelMask ͨ������
 * @param[in] bitsPerChannel ÿͨ��ʹ�õ�λ��(1��2)
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::readSequentialLSB(const unsigned char* pixelData, size_t pixelDataSize,
	char* dst, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel) const
{
	if (numBytes == 0) return true;
	if ((bitsPerChannel != 1 && bitsPerChannel != 2) || pixelDataSize == 0) return false;

	// �²�ͼ��ͨ����
	int bitCount = (pixelDataSize % 4 == 0 && pixelDataSize / 4 > 0) ? 32 : 24;
	int channels = bitCount / 8;

	// ��ʼ��������
	size_t totalBits = numBytes * 8;
	size_t bitsRead = 0;
	size_t dstByte = 0;
	int dstBit = 0;
	size_t pixByte = 0;

	// ���Ŀ�껺����
	memset(dst, 0, numBytes);

	// ��λ��ȡ
	while (bitsRead < totalBits && pixByte < pixelDataSize) {
		int ch = pixByte % channels;  // ��ǰͨ������

		// ��鵱ǰͨ���Ƿ���������
		if ((channelMask >> ch) & 0x01) {
			// ��ȡλ
			unsigned char val = pixelData[pixByte] & ((bitsPerChannel == 1) ? 0x01 : 0x03);

			// ����ȡ��λд��Ŀ�껺����
			for (int i = bitsPerChannel - 1; i >= 0 && bitsRead < totalBits; --i) {
				dst[dstByte] |= ((val >> i) & 0x01) << (7 - dstBit);
				++dstBit; ++bitsRead;
				if (dstBit == 8) { dstBit = 0; ++dstByte; }
			}
		}
		++pixByte;
	}

	// ����Ƿ��ȡ������λ
	return bitsRead == totalBits;
}

/**
 * @brief ���LSBд���㷨
 *
 * ʹ���������ɵ�α�������ȷ��д��λ�ã���ǿ�����ԡ�
 *
 * @param[in,out] pixelData �������ݻ�����
 * @param[in] pixelDataSize �������ݴ�С
 * @param[in] src Դ���ݻ�����
 * @param[in] numBytes Ҫд����ֽ���
 * @param[in] channelMask ͨ������
 * @param[in] bitsPerChannel ÿͨ��ʹ�õ�λ��(ͨ��Ϊ1)
 * @param[in] password ��������������е�����
 * @param[in] offsetBits ��ʼλƫ����
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::writeRandomLSB(unsigned char* pixelData, size_t pixelDataSize,
	const char* src, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel,
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (password.empty()) return false;

	// �������λ������
	vector<size_t> positions(pixelDataSize);
	iota(positions.begin(), positions.end(), 0);  // ���0��pixelDataSize-1

	// ʹ��������Ϊ�������
	std::seed_seq seq(password.begin(), password.end());
	mt19937 rng(seq);
	shuffle(positions.begin(), positions.end(), rng);  // �������λ��

	// ��ʼ��������
	size_t totalBits = numBytes * 8;
	size_t bitsWritten = 0;
	size_t srcByte = 0;
	int srcBit = 0;

	// ��ָ��ƫ������ʼ����λд��
	for (size_t i = offsetBits; bitsWritten < totalBits && i < pixelDataSize; ++i) {
		size_t idx = positions[i];  // ���λ��

		// ȷ����ǰλ�ö�Ӧ��ͨ��
		int ch = idx % ((pixelDataSize % 4 == 0) ? 4 : 3);

		// ��鵱ǰͨ���Ƿ���������
		if ((channelMask >> ch) & 0x01) {
			// ��ȡԴ����λ
			int v = (src[srcByte] >> (7 - srcBit)) & 0x01;

			// д��λ
			pixelData[idx] = (pixelData[idx] & 0xFE) | v;

			// ���¼�����
			++srcBit; ++bitsWritten;
			if (srcBit == 8) { srcBit = 0; ++srcByte; }
		}
	}

	// ����Ƿ�д��������λ
	return bitsWritten == totalBits;
}

/**
 * @brief ���LSB��ȡ�㷨
 *
 * ʹ����д����ͬ����������α������У�ȷ����ȡλ�á�
 *
 * @param[in] pixelData �������ݻ�����
 * @param[in] pixelDataSize �������ݴ�С
 * @param[out] dst Ŀ�����ݻ�����
 * @param[in] numBytes Ҫ��ȡ���ֽ���
 * @param[in] channelMask ͨ������
 * @param[in] bitsPerChannel ÿͨ��ʹ�õ�λ��(ͨ��Ϊ1)
 * @param[in] password ��������������е�����
 * @param[in] offsetBits ��ʼλƫ����
 * @return �ɹ�����true��ʧ�ܷ���false
 */
bool StegoCore::readRandomLSB(const unsigned char* pixelData, size_t pixelDataSize,
	char* dst, size_t numBytes,
	uint16_t channelMask, int bitsPerChannel,
	const std::string& password, size_t offsetBits) const
{
	if (numBytes == 0) return true;
	if (pixelDataSize == 0) return false;
	if (password.empty()) return false;

	// �������λ������
	vector<size_t> positions(pixelDataSize);
	iota(positions.begin(), positions.end(), 0);  // ���0��pixelDataSize-1

	// ʹ��������Ϊ�������
	std::seed_seq seq(password.begin(), password.end());
	mt19937 rng(seq);
	shuffle(positions.begin(), positions.end(), rng);  // �������λ��

	// ��ʼ��������
	size_t totalBits = numBytes * 8;
	size_t bitsRead = 0;
	size_t dstByte = 0;
	int dstBit = 0;

	// ���Ŀ�껺����
	memset(dst, 0, numBytes);

	// ��ָ��ƫ������ʼ����λ��ȡ
	for (size_t i = offsetBits; bitsRead < totalBits && i < pixelDataSize; ++i) {
		size_t idx = positions[i];  // ���λ��

		// ȷ����ǰλ�ö�Ӧ��ͨ��
		int ch = idx % ((pixelDataSize % 4 == 0) ? 4 : 3);

		// ��鵱ǰͨ���Ƿ���������
		if ((channelMask >> ch) & 0x01) {
			// ��ȡλ
			int v = pixelData[idx] & 0x01;

			// ����ȡ��λд��Ŀ�껺����
			dst[dstByte] |= v << (7 - dstBit);

			// ���¼�����
			++dstBit; ++bitsRead;
			if (dstBit == 8) { dstBit = 0; ++dstByte; }
		}
	}

	// ����Ƿ��ȡ������λ
	return bitsRead == totalBits;
}