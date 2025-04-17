#ifndef STEGO_CORE_H
#define STEGO_CORE_H

#include "BmpImage.h"
#include <string>
#include <vector>
#include <cstdint>
#include <limits>

/**
 * @file StegoCore.h
 * @brief BMPͼ����д����������
 * @version 1.0
 * @copyright Copyright 2025, ��Ϣ���ص�6��
 *
 * ���ļ������˻���LSB(�����Чλ)�㷨��BMPͼ����д���Ĺ��ܣ�
 * ֧�ֶ�����дģʽ��ͨ��ѡ�񣬰�������������У��ͼ��ܻ��ơ�
 */

 /**
  * @enum SteganoMode
  * @brief ��д�㷨ģʽö��
  *
  * �������ֲ�ͬ��LSB��дʵ�ַ�ʽ�������ڲ�ͬ��ȫ�����Ӧ�ó�����
  */
enum SteganoMode : uint16_t {
	LSB_SEQUENTIAL = 0, ///< ˳��LSBģʽ(1 bit/ͨ��)����д������󵫰�ȫ�����
	LSB_RANDOM = 1,     ///< ���LSBģʽ(1 bit/ͨ��)����Ҫ����������ֲ�����
	LSB_ENHANCED = 2    ///< ��ǿLSBģʽ(2 bit/ͨ��)��ƽ��������������
};

/**
 * @struct StegoContext
 * @brief ��д��������ʱ��������
 *
 * ������д���������п����ò�������ΪhideData/extractData��������������ġ�
 */
struct StegoContext {
	SteganoMode mode = LSB_SEQUENTIAL; ///< ��ǰ��дģʽ��Ĭ��Ϊ˳��LSB
	uint16_t    channelMask = 0x01;    ///< ��ɫͨ������(B=0x1,G=0x2,R=0x4)
	std::string password;              ///< ��������(�������ģʽ�����ݼ���)
	bool        autoDetect = false;    ///< �Ƿ��Զ������дģʽ��ͨ������
};

#pragma pack(push, 1)
/**
 * @struct StegoHeader
 * @brief ��д����ͷ���ṹ(16�ֽ�)
 *
 * Ƕ�뵽BMP��������ǰ��Ԫ��Ϣͷ�����ڴ洢��д���ݵ�������Ϣ��У��ֵ��
 * ���ý����ڴ沼��(#pragma pack(1))ȷ����ƽ̨�����ԡ�
 */
struct StegoHeader {
	char     signature[4];  ///< �ļ���ʶħ��"STEG"(0x53 0x54 0x45 0x47)
	uint32_t dataLength;    ///< ��д����ʵ�ʳ���(�ֽ�)������ͷ��
	uint32_t crc32Value;    ///< ԭʼ���ݵ�CRC32У��ֵ(������������֤)
	uint16_t stegoMode;     ///< ʵ��ʹ�õ�SteganoModeö��ֵ
	uint16_t channelMask;   ///< ʵ��ʹ�õ�ͨ���������
};
#pragma pack(pop)
static_assert(sizeof(StegoHeader) == 16, "StegoHeader ��С����Ϊ 16 �ֽ�");

/**
 * @class StegoCore
 * @brief BMPͼ��LSB��д�㷨����ʵ��
 *
 * �ṩ��������д���ܣ�
 * 1. ֧�ֶ���LSB��дģʽ(˳��/���/��ǿ)
 * 2. ��ѡ��RGBͨ�����(B/G/R�������)
 * 3. ���ݼ���(XOR����)��CRC32У��
 * 4. �Զ����������ģʽ���
 * 5. �����Ĵ�����ͱ߽���
 */
class StegoCore {
public:
	/**
	 * @brief ���������ص�BMPͼ����
	 * @param[in,out] bmp �Ѽ��ص�BMPͼ�����(�����޸�)
	 * @param[in] data ���������ݵ�ֻ��ָ��
	 * @param[in] length ���������ݳ���(�ֽ�)
	 * @param[in] ctx ��д�����Ĳ�������
	 * @return �ɹ�����true��ʧ�ܷ���false
	 * @exception ���쳣�׳�������ִ���ϸ�Ĳ���У��
	 * @note ʵ����������=16�ֽ�ͷ+ԭʼ���ݣ���������С��ͼ���������
	 * @warning ��ֱ���޸�BMP�������ݣ��������ǰ����ԭͼ
	 */
	bool hideData(BmpImage& bmp, const char* data, size_t length, const StegoContext& ctx);

	/**
	 * @brief ��BMPͼ����ȡ��������
	 * @param[in] bmp �Ѽ��ص�BMPͼ�����(ֻ��)
	 * @param[out] outData ������ݻ�����ָ��(�������delete[])
	 * @param[out] outLength �������ʵ�ʳ���
	 * @param[in,out] ctx ��д������(autoDetect=trueʱ�����mode��channelMask)
	 * @return �ɹ�����true��ʧ�ܷ���false
	 * @exception ���쳣�׳�������У�����������Ժ�������ȷ��
	 * @note ����������ɱ��������䣬�����߸����ͷ�
	 * @warning ��autoDetect=trueʱ�������޸�ctx�е�mode��channelMaskֵ
	 */
	bool extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx);

private:
	/**
	 * @brief �������ݵ�CRC32У��ֵ
	 * @param buffer �������ݻ�����
	 * @param length ���ݳ���(�ֽ�)
	 * @return 32λCRCУ��ֵ
	 */
	uint32_t calcCRC32(const char* buffer, size_t length) const;

	/**
	 * @brief ʹ������Ի���������XOR����/����
	 * @param[in,out] buffer �������������(ԭ���޸�)
	 * @param length ����������(�ֽ�)
	 * @param password ��������(������ʱ��ִ�в���)
	 */
	void xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const;

	/**
	 * @brief ����BMPͼ�����д����
	 * @param bmp BMPͼ�����
	 * @param ctx ��д�����Ĳ���
	 * @return ��������������(�ֽ�)����16�ֽ�ͷ
	 */
	size_t calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const;

	/* ���Ķ�дʵ�ַ��� */
	bool writeAll(BmpImage& bmp, const StegoHeader& header,
		const char* data, size_t length, const StegoContext& ctx);
	bool readHeader(const BmpImage& bmp, StegoHeader& headerOut,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password, bool checkSignature) const;
	bool readDataSection(const BmpImage& bmp, const StegoHeader& header,
		char* dataOut, size_t length,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password) const;

	/* ��ģʽ�ľ���ʵ�� */
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