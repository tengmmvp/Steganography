#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

/**
 * @brief Windows��׼BMP�ļ�ͷ�ṹ�� (14�ֽ�)
 * @details ������BMP�ļ��Ļ�����Ϣ�������ļ����͡���С������ƫ����
 * @note ʹ�� #pragma pack(push,1) ȷ���ṹ�尴1�ֽڶ��룬����������Ż����µ��ڴ��������
 */
#pragma pack(push, 1)
struct BmpFileHeader {
	uint16_t bfType;      // ����Ϊ 'BM' (�� 0x4D42)����ʶBMP�ļ���ʽ
	uint32_t bfSize;      // BMP�ļ��ܴ�С(�ֽ�)����������ͷ��Ϣ����������
	uint16_t bfReserved1; // �����ֶ�(����Ϊ0)������δ����չ
	uint16_t bfReserved2; // �����ֶ�(����Ϊ0)������δ����չ
	uint32_t bfOffBits;   // ���ļ�ͷ���������ݵ�ƫ����(�ֽ�)��ָʾ�������ݵ���ʼλ��
};

/**
 * @brief BMP��Ϣͷ�ṹ�� (40�ֽ�)
 * @details ����ͼ�����ϸ��Ϣ����ߴ硢��ɫ��ȡ�ѹ����ʽ��
 * @note ��ӦWindowsƽ̨�ϵ� BITMAPINFOHEADER �ṹ�壬����õ�BMP��Ϣͷ��ʽ
 */
struct BmpInfoHeader {
	uint32_t biSize;          // ��Ϣͷ��С(�̶�Ϊ40�ֽ�)
	int32_t  biWidth;         // ͼ����(����)����ֵ
	int32_t  biHeight;        // ͼ��߶�(����)����ֵ��ʾ���µ��ϴ洢����ֵ��ʾ���ϵ��´洢
	uint16_t biPlanes;        // ��ɫƽ����(�̶�Ϊ1)
	uint16_t biBitCount;      // ÿ����λ��(1,4,8,16,24,32)
	uint32_t biCompression;   // ѹ������(0=BI_RGB��ʾ��ѹ��)
	uint32_t biSizeImage;     // ͼ�����ݴ�С(�ֽ�)����ѹ��ʱ��Ϊ0
	int32_t  biXPelsPerMeter; // ˮƽ�ֱ���(����/��)����Ϊ0
	int32_t  biYPelsPerMeter; // ��ֱ�ֱ���(����/��)����Ϊ0
	uint32_t biClrUsed;       // ʵ��ʹ�õĵ�ɫ����������0��ʾʹ������
	uint32_t biClrImportant;  // ��Ҫ��ɫ��������0��ʾ������ɫ����Ҫ
};
#pragma pack(pop)

/**
 * @class BmpImage
 * @brief BMPͼ���ļ�������
 * @details �ṩBMP��ʽͼ��Ļ�����д������֧��24λ��32λɫ���δѹ��BMPͼ����
 * @note ��ʵ����Ҫ����Windows BITMAPINFOHEADER��ʽ��BMP�ļ�����֧����������
 */
class BmpImage {
public:
	/**
	 * @brief ���ļ�����BMPͼ��
	 * @param filename �����ص�BMP�ļ�·��
	 * @return bool �����Ƿ�ɹ�
	 * @throw �����׳�std::runtime_error�쳣���ļ�����ʧ��ʱ��
	 */
	bool load(const std::string& filename);

	/**
	 * @brief ��ͼ�񱣴�ΪBMP�ļ�
	 * @param filename Ŀ���ļ�·��
	 * @return bool �����Ƿ�ɹ�
	 * @throw �����׳�std::runtime_error�쳣���ļ�����ʧ��ʱ��
	 */
	bool save(const std::string& filename) const;

	/**
	 * @brief ��ȡͼ����
	 * @return int ͼ����(����)
	 */
	int getWidth() const { return m_infoHeader.biWidth; }

	/**
	 * @brief ��ȡͼ��߶�
	 * @return int ͼ��߶�(����)
	 */
	int getHeight() const { return m_infoHeader.biHeight; }

	/**
	 * @brief ��ȡͼ��ɫ��
	 * @return int ÿ����λ��(ͨ��Ϊ24��32)
	 */
	int getBitCount() const { return m_infoHeader.biBitCount; }

	/**
	 * @brief ��ȡ�������ݵĿ�дָ��
	 * @return unsigned char* ����������ʼ��ַ
	 * @warning ֱ�Ӳ�����������ʱ�������ȷ����Խ��
	 */
	unsigned char* getPixelData() { return m_pixelData.data(); }

	/**
	 * @brief ��ȡ�������ݵ�ֻ��ָ��
	 * @return const unsigned char* ����������ʼ��ַ
	 */
	const unsigned char* getPixelData() const { return m_pixelData.data(); }

	/**
	 * @brief ��ȡ�������ݴ�С
	 * @return size_t �����������ֽ���
	 */
	size_t getPixelDataSize() const { return m_pixelData.size(); }

	/**
	 * @brief ��ȡ��������ƫ����
	 * @return int �����������ļ��е���ʼλ��(�ֽ�)
	 */
	int getPixelDataOffset() const { return static_cast<int>(m_fileHeader.bfOffBits); }

	/**
	 * @brief ����Ԥ�ڵ��ļ���С
	 * @return size_t Ԥ�Ƶ��ļ��ܴ�С(�ֽ�)
	 * @note ���ڵ�ǰͷ��Ϣ���������ݼ���
	 */
	size_t getEstimatedFileSize() const {
		return static_cast<size_t>(m_fileHeader.bfOffBits) + m_pixelData.size();
	}

	/**
	 * @brief ��ȡ�ļ�ͷ����
	 * @return BmpFileHeader& �ļ�ͷ�ṹ������
	 */
	BmpFileHeader& fileHeader() { return m_fileHeader; }

	/**
	 * @brief ��ȡ��Ϣͷ����
	 * @return BmpInfoHeader& ��Ϣͷ�ṹ������
	 */
	BmpInfoHeader& infoHeader() { return m_infoHeader; }

private:
	BmpFileHeader m_fileHeader{};             ///< BMP�ļ�ͷ
	BmpInfoHeader m_infoHeader{};             ///< BMP��Ϣͷ
	std::vector<unsigned char> m_extraHeader;  ///< ����ͷ��Ϣ���ɫ������
	std::vector<unsigned char> m_pixelData;    ///< ͼ����������
};

#endif // BMP_IMAGE_H
