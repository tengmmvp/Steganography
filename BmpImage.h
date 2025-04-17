#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

/**
 * @file BmpImage.h
 * @brief BMPͼ���ļ���д����������
 * @version 1.0
 * @copyright Copyright 2025, ��Ϣ���ص�6��
 *
 * ���ļ�������BMPͼ���ļ���ʽ�Ľṹ��ʹ����֧࣬��24λ��32λδѹ��BMP��ʽ�Ķ�д������
 * ����Windows BMP�ļ���ʽ�淶��֧�����¶��ϵ����ش洢˳��
 */

#pragma pack(push, 1)
 /**
  * @struct BmpFileHeader
  * @brief BMP�ļ�ͷ�ṹ�壨14�ֽڣ�
  *
  * �ýṹ���ӦBMP�ļ����ļ�ͷ���֣������ļ���ʶ����С����������ƫ�����Ȼ�����Ϣ��
  * �����ֶβ���С����洢��ʹ��#pragma pack(1)ȷ���ڴ����Ϊ1�ֽڡ�
  */
struct BmpFileHeader {
	uint16_t bfType;      ///< �ļ����ͱ�ʶ������Ϊ'BM'(0x4D42)
	uint32_t bfSize;      ///< �ļ��ܴ�С���ֽڣ�����������ͷ������
	uint16_t bfReserved1; ///< �����ֶ�1�������ʼ��Ϊ0
	uint16_t bfReserved2; ///< �����ֶ�2�������ʼ��Ϊ0
	uint32_t bfOffBits;   ///< ���ļ�ͷ���������ݵ�ƫ�������ֽڣ�
};

/**
 * @struct BmpInfoHeader
 * @brief BMP��Ϣͷ�ṹ�壨40�ֽڣ�
 *
 * �ýṹ�����BMPͼ��ĳߴ硢ɫ����Ⱥ�ѹ����ʽ�Ⱥ�����Ϣ��
 * ����Windows BITMAPINFOHEADER�ı�׼ʵ�֣�֧�������δѹ����ʽ��
 */
struct BmpInfoHeader {
	uint32_t biSize;          ///< ���ṹ���С���̶�Ϊ40�ֽڣ�
	int32_t  biWidth;         ///< ͼ���ȣ����أ�������Ϊ����
	int32_t  biHeight;        ///< ͼ��߶ȣ����أ���������ʾ�������ϴ洢
	uint16_t biPlanes;        ///< ��ɫƽ�������̶�Ϊ1��
	uint16_t biBitCount;      ///< ÿ����λ����֧��24��32��
	uint32_t biCompression;   ///< ѹ�����ͣ�0��ʾBI_RGB��ѹ����
	uint32_t biSizeImage;     ///< �������ݴ�С���ֽڣ�����ѹ��ʱ����Ϊ0
	int32_t  biXPelsPerMeter; ///< ˮƽ�ֱ��ʣ�����/�ף�������Ϊ0
	int32_t  biYPelsPerMeter; ///< ��ֱ�ֱ��ʣ�����/�ף�������Ϊ0
	uint32_t biClrUsed;       ///< ʵ��ʹ�õĵ�ɫ����ɫ����0��ʾʹ��ȫ��
	uint32_t biClrImportant;  ///< ��Ҫ��ɫ����0��ʾ������ɫ����Ҫ
};
#pragma pack(pop)

/**
 * @class BmpImage
 * @brief BMPͼ������
 *
 * �ṩBMP��ʽͼ��ļ��ء�����ͻ�����Ϣ��ѯ���ܡ�֧��24λRGB��32λRGBA��ʽ��
 * �Զ��������ض��루ÿ���������ݰ�4�ֽڶ��룩���ڴ���ά���������ļ�ͷ����Ϣͷ���������ݡ�
 */
class BmpImage {
public:
	/**
	 * @brief ���ļ�����BMPͼ��
	 * @param[in] filename Ҫ���ص�BMP�ļ�·��
	 * @return ���سɹ�����true��ʧ�ܷ���false
	 * @exception ���쳣�׳����������ô���״̬
	 * @note ��֧��δѹ����24/32λBMP��ʽ���߶�ֵΪ�����������ϴ洢��ʽ
	 */
	bool load(const std::string& filename);

	/**
	 * @brief ��ͼ�񱣴�ΪBMP�ļ�
	 * @param[in] filename ����ļ�·��
	 * @return ����ɹ�����true��ʧ�ܷ���false
	 * @exception ���쳣�׳����������ô���״̬
	 * @note ����ļ�������δѹ����ʽ���Զ��������ض���
	 */
	bool save(const std::string& filename) const;

	/**
	 * @brief ��ȡͼ����
	 * @return ͼ���ȣ����أ�
	 */
	int getWidth() const { return m_infoHeader.biWidth; }

	/**
	 * @brief ��ȡͼ��߶�
	 * @return ͼ��߶ȣ����أ�
	 * @note ����ֵΪ����ʾ�������ϴ洢
	 */
	int getHeight() const { return m_infoHeader.biHeight; }

	/**
	 * @brief ��ȡɫ��
	 * @return ÿ����λ����24��32��
	 */
	int getBitCount() const { return m_infoHeader.biBitCount; }

	/**
	 * @brief ��ȡ��д��������ָ��
	 * @return ָ���������ݵĿ�дָ��
	 * @warning ֱ���޸��������ݿ����ƻ�ͼ��������
	 */
	unsigned char* getPixelData() { return m_pixelData.data(); }

	/**
	 * @brief ��ȡֻ����������ָ��
	 * @return ָ���������ݵ�ֻ��ָ��
	 */
	const unsigned char* getPixelData() const { return m_pixelData.data(); }

	/**
	 * @brief ��ȡ�������ݴ�С
	 * @return ��������ռ�õ��ֽ���
	 */
	size_t getPixelDataSize() const { return m_pixelData.size(); }

	/**
	 * @brief ��ȡ��������ƫ����
	 * @return ���ļ�ͷ���������ݵ��ֽ�ƫ����
	 */
	int getPixelDataOffset() const { return static_cast<int>(m_fileHeader.bfOffBits); }

	/**
	 * @brief �����ļ��ܴ�С
	 * @return ��������ͷ�����ݵ��ļ��ܴ�С���ֽڣ�
	 */
	size_t getEstimatedFileSize() const {
		return static_cast<size_t>(m_fileHeader.bfOffBits) + m_pixelData.size();
	}

	/**
	 * @brief ��ȡ�ļ�ͷ����
	 * @return BMP�ļ�ͷ�Ŀ��޸�����
	 * @warning ֱ���޸Ŀ����ƻ��ļ�������
	 */
	BmpFileHeader& fileHeader() { return m_fileHeader; }

	/**
	 * @brief ��ȡ��Ϣͷ����
	 * @return BMP��Ϣͷ�Ŀ��޸�����
	 * @warning ֱ���޸Ŀ����ƻ��ļ�������
	 */
	BmpInfoHeader& infoHeader() { return m_infoHeader; }

private:
	BmpFileHeader m_fileHeader{};             ///< BMP�ļ�ͷ�ṹ��ʵ��
	BmpInfoHeader m_infoHeader{};             ///< BMP��Ϣͷ�ṹ��ʵ��
	std::vector<unsigned char> m_extraHeader; ///< ��չͷ���ɫ�����ݣ����У�
	std::vector<unsigned char> m_pixelData;   ///< �������ݴ洢��
};

#endif // BMP_IMAGE_H