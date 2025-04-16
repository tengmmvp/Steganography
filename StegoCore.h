#ifndef STEGO_CORE_H
#define STEGO_CORE_H

#include "BmpImage.h"
#include <string>
#include <vector>
#include <cstdint>
#include <limits>

/**
 * @enum SteganoMode
 * @brief ������д�㷨�����ֹ���ģʽ
 *
 * @var LSB_SEQUENTIAL ˳�������Чλ��д��ÿ����ɫͨ��ʹ�����1λ��
 * @var LSB_RANDOM     ������������λ����д��ÿ����ɫͨ�����1λ��
 * @var LSB_ENHANCED   ������˳����д��ÿ����ɫͨ�����2λ��
 */
enum SteganoMode : uint16_t {
	LSB_SEQUENTIAL = 0,
	LSB_RANDOM = 1,
	LSB_ENHANCED = 2
};

/**
 * @struct StegoContext
 * @brief ��д��������ʱ���ò�����
 *
 * @var mode       ��д�㷨ģʽѡ��
 * @var channelMask ��ɫͨ��ѡ�����루λ��ϣ�0x01�� 0x02�� 0x04�죩
 * @var password   ����/���ģʽ���루���ַ�����ʾ���ã�
 * @var autoDetect ��ȡʱ�Ƿ������Զ�ģʽ���
 */
struct StegoContext {
	SteganoMode mode = LSB_SEQUENTIAL;
	uint16_t    channelMask = 0x01;
	std::string password;
	bool        autoDetect = false;
};

/**
 * @struct StegoHeader
 * @brief ��д����ͷ���ṹ��16�ֽڹ̶���С��
 *
 * @note ʹ��#pragma packȷ���ڴ����
 * @var signature   �ļ�ħ����"STEG"��
 * @var dataLength  ��д����ʵ�ʳ��ȣ��ֽڣ�
 * @var crc32Value  ԭʼ����CRC32У��ֵ������ǰ��
 * @var stegoMode   ʵ��ʹ�õ���дģʽ
 * @var channelMask ʵ��ʹ�õ�ͨ������
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

static_assert(sizeof(StegoHeader) == 16, "StegoHeader�����ϸ�16�ֽ�");

/**
 * @class StegoCore
 * @brief BMPͼ����д������ʵ����
 *
 * �ṩ����LSB�㷨����������/��ȡ���ܣ�֧��������дģʽ��
 * 1. ˳��LSB��д������ģʽ��
 * 2. ���λ��LSB��д����ȫ����ǿ��
 * 3. ��ǿ����LSB��д������������
 */
class StegoCore
{
public:
	/**
	 * @brief ִ���������ز���
	 *
	 * @param[in] bmp     �Ѽ��ص�BMPͼ�����
	 * @param[in] data    ����������ָ��
	 * @param[in] length  ���ݳ��ȣ��ֽڣ�
	 * @param[in] ctx     ��д���ò���
	 * @return bool       �����Ƿ�ɹ�
	 *
	 * @note �Զ����16�ֽ�ͷ����Ϣ��ʵ��������۳�ͷ����С
	 */
	bool hideData(BmpImage& bmp, const char* data, size_t length, const StegoContext& ctx);

	/**
	 * @brief ִ��������ȡ����
	 *
	 * @param[in] bmp        �Ѽ��ص�BMPͼ�����
	 * @param[out] outData   ��ȡ���ݻ�����ָ�루��������ͷţ�
	 * @param[out] outLength ��ȡ����ʵ�ʳ���
	 * @param[in,out] ctx    ��д���ò�����autoDetect=trueʱ���޸�ģʽ������
	 * @return bool          �����Ƿ�ɹ�
	 */
	bool extractData(const BmpImage& bmp, char*& outData, size_t& outLength, StegoContext& ctx);

private:
	/**
	 * @brief �������ݵ�CRC32У��ֵ
	 *
	 * @param buffer  ���ݻ�����
	 * @param length  ���ݳ��ȣ��ֽڣ�
	 * @return uint32_t ����õ���CRC32ֵ
	 */
	uint32_t calcCRC32(const char* buffer, size_t length) const;

	/**
	 * @brief ִ��XOR������/���ܣ�ԭ�ز�����
	 *
	 * @param buffer   ���������ݻ�����
	 * @param length   ���ݳ��ȣ��ֽڣ�
	 * @param password �������루���ַ�������������
	 */
	void xorEncryptBuffer(char* buffer, size_t length, const std::string& password) const;

	/**
	 * @brief д����дͷ������Ч����
	 *
	 * @param bmp    Ŀ��BMPͼ��
	 * @param header 16�ֽ�ͷ����Ϣ
	 * @param data   ��Ч����ָ��
	 * @param length ��Ч���ݳ���
	 * @param ctx    ��д���ò���
	 * @return bool  �����Ƿ�ɹ�
	 */
	bool writeAll(BmpImage& bmp, const StegoHeader& header, const char* data, size_t length, const StegoContext& ctx);

	/**
	 * @brief ��BMPͼ���ȡ��дͷ��
	 *
	 * @param[in] bmp              ԴBMPͼ��
	 * @param[out] headerOut       ͷ����Ϣ���
	 * @param[in] modeToTry        ���Ե���дģʽ
	 * @param[in] channelMaskToTry ���Ե�ͨ������
	 * @param[in] password         ���ģʽ����
	 * @param[in] checkSignature   �Ƿ���֤ħ��
	 * @return bool                �Ƿ�ɹ���ȡ
	 */
	bool readHeader(const BmpImage& bmp, StegoHeader& headerOut,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password, bool checkSignature) const;

	/**
	 * @brief ��ȡ��д���ݲ���
	 *
	 * @param[in] bmp              ԴBMPͼ��
	 * @param[in] header           �ѽ�����ͷ����Ϣ
	 * @param[out] dataOut         �����������������Ԥ���䣩
	 * @param[in] length           Ԥ�����ݳ���
	 * @param[in] modeToTry        ���Ե���дģʽ
	 * @param[in] channelMaskToTry ���Ե�ͨ������
	 * @param[in] password         ���ģʽ����
	 * @return bool                �Ƿ�ɹ���ȡ
	 */
	bool readDataSection(const BmpImage& bmp, const StegoHeader& header,
		char* dataOut, size_t length,
		SteganoMode modeToTry, uint16_t channelMaskToTry,
		const std::string& password) const;

	/**
	 * @brief ˳��LSBд��ʵ��
	 *
	 * @param[in,out] pixelData    �������ݻ�����
	 * @param[in] pixelDataSize    ���������ܴ�С
	 * @param[in] src              Դ����ָ��
	 * @param[in] numBytes         д���ֽ���
	 * @param[in] channelMask      ��ɫͨ������
	 * @param[in] bitsPerChannel   ÿͨ��д��λ����1��2��
	 * @return bool                �����Ƿ�ɹ�
	 */
	bool writeSequentialLSB(unsigned char* pixelData, size_t pixelDataSize,
		const char* src, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel) const;

	/**
	 * @brief ˳��LSB��ȡʵ��
	 *
	 * @param[in] pixelData       �������ݻ�������ֻ����
	 * @param[out] dst            ������ݻ�����
	 * @param[in] pixelDataSize   ���������ܴ�С
	 * @param[in] numBytes        ��ȡ�ֽ���
	 * @param[in] channelMask     ��ɫͨ������
	 * @param[in] bitsPerChannel  ÿͨ����ȡλ����1��2��
	 * @return bool               �����Ƿ�ɹ�
	 */
	bool readSequentialLSB(const unsigned char* pixelData, size_t pixelDataSize,
		char* dst, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel) const;

	/**
	 * @brief ���LSBд��ʵ��
	 *
	 * @param[in,out] pixelData    �������ݻ�����
	 * @param[in] pixelDataSize    ���������ܴ�С
	 * @param[in] src              Դ����ָ��
	 * @param[in] numBytes         д���ֽ���
	 * @param[in] channelMask      ��ɫͨ������
	 * @param[in] bitsPerChannel   ÿͨ��д��λ������ǰ��֧��1��
	 * @param[in] password         ���������������
	 * @param[in] offsetBits       LSB����ʼƫ�ƣ�Ĭ��0��
	 * @return bool                �����Ƿ�ɹ�
	 */
	bool writeRandomLSB(unsigned char* pixelData, size_t pixelDataSize,
		const char* src, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel,
		const std::string& password, size_t offsetBits = 0) const;

	/**
	 * @brief ���LSB��ȡʵ��
	 *
	 * @param[in] pixelData       �������ݻ�������ֻ����
	 * @param[out] dst            ������ݻ�����
	 * @param[in] pixelDataSize   ���������ܴ�С
	 * @param[in] numBytes        ��ȡ�ֽ���
	 * @param[in] channelMask     ��ɫͨ������
	 * @param[in] bitsPerChannel  ÿͨ����ȡλ������ǰ��֧��1��
	 * @param[in] password        ���������������
	 * @param[in] offsetBits      LSB����ʼƫ�ƣ�Ĭ��0��
	 * @return bool               �����Ƿ�ɹ�
	 */
	bool readRandomLSB(const unsigned char* pixelData, size_t pixelDataSize,
		char* dst, size_t numBytes,
		uint16_t channelMask, int bitsPerChannel,
		const std::string& password, size_t offsetBits = 0) const;

	/**
	 * @brief ����ͼ����д����
	 *
	 * @param[in] bmp  Ŀ��BMPͼ��
	 * @param[in] ctx  ��д���ò���
	 * @return size_t  ���������ֽ���������ͷ����
	 */
	size_t calculateCapacity(const BmpImage& bmp, const StegoContext& ctx) const;
};

#endif // STEGO_CORE_H
