#include "BmpImage.h"
#include <fstream>
#include <iostream>
#include <cstring>

/**
 * @file BmpImage.cpp
 * @brief BMPͼ���ļ������뱣��ʵ��
 * @copyright Copyright 2025, ��Ϣ���ص�6��
 *
 * ���ļ�ʵ����BmpImage������й��ܣ�����BMP�ļ��ļ��ؽ����ͱ��������
 * �ϸ���ѭWindows BMP�ļ���ʽ�淶��������ֱ߽������ʹ��������
 */

using namespace std;

/**
 * @brief ���ļ�����BMPͼ��
 * @param[in] filename Ҫ���ص�BMP�ļ�·��
 * @return ���سɹ�����true��ʧ�ܷ���false
 *
 * �÷���ִ�����²�����
 * 1. ���ļ�����֤������Ч��
 * 2. ��ȡ��У���ļ�ͷ����Ϣͷ
 * 3. ������ܵ���չͷ���ɫ������
 * 4. ��ȡ�������ݲ���֤������
 *
 * @note �ļ���ʧ�ܻ��ʽ����������ϸ������Ϣ��cerr
 * @warning ����ʧ�ܻ��������ͼ������
 */
bool BmpImage::load(const std::string& filename)
{
	// �Զ�����ģʽ���ļ�
	ifstream fin(filename, ios::binary);
	if (!fin.is_open()) {
		cerr << "[����] �޷����ļ�: " << filename << endl;
		return false;
	}

	/* �ļ�ͷ��ȡ����֤ */
	fin.read(reinterpret_cast<char*>(&m_fileHeader), sizeof(m_fileHeader));
	if (!fin || m_fileHeader.bfType != 0x4D42) {
		cerr << "[����] ��Ч��BMP�ļ�: " << filename
			<< " (�ļ����ͱ�ʶӦΪ0x4D42)" << endl;
		fin.close();
		return false;
	}

	/* ��Ϣͷ��ȡ����֤ */
	fin.read(reinterpret_cast<char*>(&m_infoHeader), sizeof(m_infoHeader));
	if (!fin) {
		cerr << "[����] ��ȡ��Ϣͷʧ��: " << filename << endl;
		fin.close();
		return false;
	}

	/* ������չͷ/��ɫ������ */
	size_t dataOffset = m_fileHeader.bfOffBits;
	streamoff curPos = fin.tellg();

	// ���ƫ�����Ƿ������λ��ȡλ��
	if (dataOffset < static_cast<size_t>(curPos)) {
		// ���˵�ָ��ƫ�����������쳣�����
		fin.seekg(dataOffset, ios::beg);
	}
	else if (dataOffset > static_cast<size_t>(curPos)) {
		// ��ȡ��չͷ����
		size_t extraSize = dataOffset - static_cast<size_t>(curPos);
		m_extraHeader.resize(extraSize);
		fin.read(reinterpret_cast<char*>(m_extraHeader.data()), extraSize);
		if (!fin || static_cast<size_t>(fin.gcount()) != extraSize) {
			cerr << "[����] ��ȡ��չͷʧ��: " << filename
				<< " (������С: " << extraSize
				<< ", ʵ�ʶ�ȡ: " << fin.gcount() << ")" << endl;
			fin.close();
			return false;
		}
	}

	/* �������ݶ�ȡ */
	size_t pixelSize = 0;
	// ����ʹ���ļ�ͷ�е��ܴ�С����
	if (m_fileHeader.bfSize > dataOffset) {
		pixelSize = m_fileHeader.bfSize - dataOffset;
	}
	// ��ѡ��Ϣͷ�е��������ݴ�С
	if (pixelSize == 0 && m_infoHeader.biSizeImage > 0) {
		pixelSize = m_infoHeader.biSizeImage;
	}

	m_pixelData.resize(pixelSize);
	fin.seekg(dataOffset, ios::beg);

	if (pixelSize > 0) {
		fin.read(reinterpret_cast<char*>(m_pixelData.data()), pixelSize);
		if (!fin || static_cast<size_t>(fin.gcount()) != pixelSize) {
			cerr << "[����] ��ȡ��������ʧ��: " << filename
				<< " (������С: " << pixelSize
				<< ", ʵ�ʶ�ȡ: " << fin.gcount() << ")" << endl;
			fin.close();
			m_pixelData.clear();
			return false;
		}
	}

	fin.close();
	return true;
}

/**
 * @brief ��ͼ�񱣴�ΪBMP�ļ�
 * @param[in] filename ����ļ�·��
 * @return ����ɹ�����true��ʧ�ܷ���false
 *
 * �÷���ִ�����²�����
 * 1. ׼������֤ͷ��Ϣ
 * 2. д���ļ�ͷ����Ϣͷ
 * 3. д����չͷ���ɫ������
 * 4. д����������
 *
 * @note ����ʧ�ܻ������ϸ������Ϣ��cerr
 * @warning �Ḳ���Ѵ��ڵ�ͬ���ļ�
 */
bool BmpImage::save(const std::string& filename) const
{
	// �Զ�����ģʽ����/�ض��ļ�
	ofstream fout(filename, ios::binary | ios::trunc);
	if (!fout.is_open()) {
		cerr << "[����] �޷������ļ�: " << filename << endl;
		return false;
	}

	/* ׼��ͷ��Ϣ */
	BmpFileHeader fh = m_fileHeader;
	BmpInfoHeader ih = m_infoHeader;
	uint32_t offBits = fh.bfOffBits;
	uint32_t pixelSize = static_cast<uint32_t>(m_pixelData.size());

	// �����ļ��ܴ�С
	fh.bfSize = offBits + pixelSize;

	/* д���ļ�ͷ */
	fout.write(reinterpret_cast<const char*>(&fh), sizeof(fh));
	if (!fout) {
		cerr << "[����] д���ļ�ͷʧ��" << endl;
		fout.close();
		return false;
	}

	/* д����Ϣͷ����չ���� */
	if (ih.biSize >= sizeof(BmpInfoHeader)) {
		fout.write(reinterpret_cast<const char*>(&ih), sizeof(BmpInfoHeader));
		if (!fout) {
			cerr << "[����] д����Ϣͷʧ��" << endl;
			fout.close();
			return false;
		}

		// ������ܵ���չͷ
		if (ih.biSize > sizeof(BmpInfoHeader)) {
			size_t ext = ih.biSize - sizeof(BmpInfoHeader);
			if (ext <= m_extraHeader.size()) {
				fout.write(reinterpret_cast<const char*>(m_extraHeader.data()), ext);
				if (!fout) {
					cerr << "[����] д����չͷʧ��" << endl;
					fout.close();
					return false;
				}
			}
			else {
				cerr << "[����] ��չͷ��С��ʵ�ʲ��� (��Ҫ: " << ext
					<< ", ʵ��: " << m_extraHeader.size() << ")" << endl;
				fout.close();
				return false;
			}
		}
	}
	else {
		cerr << "[����] ��Ϣͷ��С��Ч: " << ih.biSize
			<< " (��СӦΪ: " << sizeof(BmpInfoHeader) << ")" << endl;
		fout.close();
		return false;
	}

	/* д��ʣ���ɫ������ */
	size_t paletteOff = (ih.biSize > sizeof(BmpInfoHeader))
		? ih.biSize - sizeof(BmpInfoHeader) : 0;
	if (paletteOff < m_extraHeader.size()) {
		fout.write(reinterpret_cast<const char*>(m_extraHeader.data() + paletteOff),
			m_extraHeader.size() - paletteOff);
		if (!fout) {
			cerr << "[����] д���ɫ������ʧ��" << endl;
			fout.close();
			return false;
		}
	}

	/* д���������� */
	fout.seekp(offBits, ios::beg);
	if (!m_pixelData.empty()) {
		fout.write(reinterpret_cast<const char*>(m_pixelData.data()), m_pixelData.size());
		if (!fout) {
			cerr << "[����] д����������ʧ�� (��С: "
				<< m_pixelData.size() << " �ֽ�)" << endl;
			fout.close();
			return false;
		}
	}

	fout.close();
	return true;
}