/**
 * @file BmpImage.cpp
 * @brief BMPͼ���ļ��ļ����뱣��ʵ��
 */

#include "BmpImage.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>

using namespace std;

/**
 * @brief ���ļ�����BMPͼ��
 * @param filename Ҫ���ص�BMP�ļ�·��
 * @return ���سɹ�����true��ʧ�ܷ���false
 */
bool BmpImage::load(const std::string& filename)
{
	ifstream fin(filename, ios::binary);
	if (!fin.is_open()) {
		cerr << "[����] �޷����ļ�: " << filename << endl;
		return false;
	}

	// 1) ��ȡ�ļ�ͷ
	fin.read(reinterpret_cast<char*>(&m_fileHeader), sizeof(BmpFileHeader));
	if (!fin) { // ����ȡ�ļ�ͷ�Ƿ�ɹ�
		cerr << "[����] ��ȡBMP�ļ�ͷʧ��: " << filename << endl;
		fin.close(); return false;
	}
	// ���BMP��ʶ 'BM'
	if (m_fileHeader.bfType != 0x4D42) {
		cerr << "[����] ��Ч��BMP�ļ�(bfType != 'BM'): " << filename << endl;
		fin.close();
		return false;
	}

	// 2) ��ȡ��Ϣͷ (��׼40�ֽ�)
	fin.read(reinterpret_cast<char*>(&m_infoHeader), sizeof(BmpInfoHeader));
	if (!fin) { // ����ȡ��Ϣͷ�Ƿ�ɹ�
		cerr << "[����] ��ȡBMP��Ϣͷʧ��: " << filename << endl;
		fin.close(); return false;
	}

	// ������ܵĶ���ͷ/��ɫ��
	size_t actualDataOffset = m_fileHeader.bfOffBits; // ��������ʵ�ʿ�ʼλ��
	streamoff currentPos = fin.tellg();
	streamoff expectedHeaderSize = sizeof(BmpFileHeader) + m_infoHeader.biSize; // ������ͷ�Ĵ�С��

	// ��� bfOffBits ����ڵ�ǰ��ȡλ�ú�����ͷ��С�ĺ�����
	if (actualDataOffset < currentPos) {
		// bfOffBits ָ���Ѷ�����ͷ�ڲ���֮ǰ���ļ��ṹ���ܴ���
		cerr << "[����] bfOffBits (" << actualDataOffset << ") ָ���Ѷ�ȡ��ͷ���ڲ���֮ǰ (��ǰλ��: " << currentPos << ") �ļ�: " << filename << "�����Ի��ˡ�" << endl;
		fin.seekg(actualDataOffset, ios::beg);
		if (!fin) {
			cerr << "[����] �޷����˵�ͷ��ָ����bfOffBitsλ�á��޷������������ݡ�" << endl;
			fin.close(); return false;
		}
		currentPos = fin.tellg(); // ���µ�ǰλ��
		// ���seek����Ȼ���ԣ������ȡextra header���߼�Ҳ���ܳ���
	}

	if (actualDataOffset > (size_t)currentPos) {
		size_t extraSizeToRead = actualDataOffset - (size_t)currentPos;
		m_extraHeader.resize(extraSizeToRead);
		// �ӵ�ǰλ�ö�ȡֱ���������ݿ�ʼǰ����������
		// cout << "[����] ���ڶ�ȡ " << extraSizeToRead << " �ֽڵĶ���ͷ��/��ɫ�����ݡ�" << endl;
		fin.read(reinterpret_cast<char*>(m_extraHeader.data()), extraSizeToRead);
		if (!fin || (size_t)fin.gcount() != extraSizeToRead) {
			cerr << "[����] ��ȡ����ͷ��/��ɫ������ʧ�� (Ԥ�� " << extraSizeToRead << " �ֽ�) �ļ�: " << filename << "��ʵ�ʶ�ȡ " << fin.gcount() << " �ֽڡ�" << endl;
			fin.close(); return false;
		}
	}
	// ��� actualDataOffset == currentPos����û�ж���������Ҫ��ȡ

	// 3) ��ȡ��������
	size_t pixelDataSize = 0;
	if (m_fileHeader.bfSize > actualDataOffset) { // ʹ��ʵ��ƫ����
		pixelDataSize = m_fileHeader.bfSize - actualDataOffset;
	}

	if (pixelDataSize == 0 && m_infoHeader.biSizeImage > 0) {
		pixelDataSize = m_infoHeader.biSizeImage;
		cerr << "[����] ��bfSize������������ݴ�СΪ��/��ֵ������biSizeImage (" << pixelDataSize << " �ֽ�) �ļ�: " << filename << endl;
	}
	else if (m_fileHeader.bfSize <= actualDataOffset && m_infoHeader.biSizeImage == 0) {
		cerr << "[����] BMPͷ��ָʾ��Ч���������ݴ�С (bfSize <= bfOffBits �� biSizeImage Ϊ 0) �ļ�: " << filename << endl;
		fin.close();
		return false; // �޷�ȷ�����ݴ�С
	}

	// ��λ���������ݿ�ʼ�� (�ٴ�ȷ��)
	fin.seekg(actualDataOffset, ios::beg);
	if (!fin) {
		cerr << "[����] �޷���λ����������ƫ��λ�� (bfOffBits = " << actualDataOffset << ") �ļ�: " << filename << endl;
		fin.close();
		return false;
	}


	m_pixelData.resize(pixelDataSize);
	if (pixelDataSize > 0) {
		// cout << "[����] ���Զ�ȡ " << pixelDataSize << " �ֽڵ��������ݡ�" << endl; // ���Debug��Ϣ
		fin.read(reinterpret_cast<char*>(m_pixelData.data()), pixelDataSize);
		// ����ȡ�����Ƿ�ɹ��������Ƿ��ȡ���������ֽ���
		if (!fin || (size_t)fin.gcount() != pixelDataSize) {
			cerr << "[����] ��ȡԤ�ڵ���������ʧ�� (" << pixelDataSize << " �ֽ�) �ļ�: " << filename << "��ʵ�ʶ�ȡ " << fin.gcount() << " �ֽڡ�" << endl;
			fin.close();
			m_pixelData.clear(); // ��ȡ����������������Ч��
			return false;
		}
	}
	else {
		cerr << "[����] �������ݴ�СΪ�㣬�ļ�: " << filename << "��" << endl;
	}


	fin.close();
	return true;
}

/**
 * @brief ��BMPͼ�񱣴浽�ļ�
 * @param filename Ҫ������ļ�·��
 * @return ����ɹ�����true��ʧ�ܷ���false
 */
bool BmpImage::save(const std::string& filename) const
{
	ofstream fout(filename, ios::binary | ios::trunc); // ��� trunc ȷ�����Ǿ��ļ�
	if (!fout.is_open()) {
		cerr << "[����] �޷������ļ����б���: " << filename << endl;
		return false;
	}

	// --- ׼��Ҫд���ͷ��Ϣ ---
	BmpFileHeader fileHeader = m_fileHeader;
	BmpInfoHeader infoHeader = m_infoHeader;

	// 1. ȷ������ƫ���� (bfOffBits) - ͨ��ʹ�ü���ʱ��ֵ
	uint32_t actualOffBits = m_fileHeader.bfOffBits; // ��������ʱ��ƫ����

	// (��ѡ) У��bfOffBits�Ƿ���ͷ��С+����ͷ��Сƥ��
	uint32_t calculatedOffBits = sizeof(BmpFileHeader) + infoHeader.biSize + static_cast<uint32_t>(m_extraHeader.size());
	if (actualOffBits != calculatedOffBits) {
		cerr << "[����] ͷ���е�bfOffBits (" << actualOffBits << ") �����biSize+extraHeader�����ֵ (" << calculatedOffBits << ") ��ƥ�䡣ʹ��ͷ��ֵ��" << endl;
	}
	fileHeader.bfOffBits = actualOffBits; // ȷ��д��������ֵ

	// 2. �����ļ��ܴ�С (bfSize)
	uint32_t actualPixelDataSize = static_cast<uint32_t>(m_pixelData.size());
	fileHeader.bfSize = actualOffBits + actualPixelDataSize;

	// 3. (��ѡ) ���� biSizeImage
	if (infoHeader.biSizeImage != 0 && infoHeader.biSizeImage != actualPixelDataSize) {
		cerr << "[����] ����biSizeImage���� " << infoHeader.biSizeImage << " �� " << actualPixelDataSize << "��" << endl;
		infoHeader.biSizeImage = actualPixelDataSize;
	}
	else if (infoHeader.biSizeImage == 0 && infoHeader.biCompression == 0) {
		// ����Ϊδѹ��ͼ����� biSizeImage
		infoHeader.biSizeImage = actualPixelDataSize;
	}

	// --- [����] ��ӡ��Ҫд�����Ϣ ---
	// cout << "[����] ����BMP:" << endl;
	// cout << "  bfOffBits: " << fileHeader.bfOffBits << endl;
	// cout << "  PixelDataSize: " << actualPixelDataSize << endl;
	// cout << "  Calculated bfSize: " << fileHeader.bfSize << endl;
	// cout << "  biSizeImage: " << infoHeader.biSizeImage << endl;
	// --- ������Ϣ���� ---

	// 1) д���ļ�ͷ
	fout.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BmpFileHeader));
	if (!fout) { cerr << "[����] д���ļ�ͷʧ��: " << filename << endl; fout.close(); return false; }

	// 2) д����Ϣͷ (д��biSizeָ�����ֽ�����������ֻ�б�׼������infoHeader)
	if (infoHeader.biSize >= sizeof(BmpInfoHeader)) {
		fout.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BmpInfoHeader));
		if (!fout) { cerr << "[����] д���׼��Ϣͷʧ��: " << filename << endl; fout.close(); return false; }

		// ��� biSize > 40������ʣ�ಿ���� m_extraHeader �Ŀ�ͷ
		if (infoHeader.biSize > sizeof(BmpInfoHeader)) {
			size_t remainingHeaderSize = infoHeader.biSize - sizeof(BmpInfoHeader);
			if (remainingHeaderSize <= m_extraHeader.size()) {
				// cout << "[����] ���ڴ�m_extraHeaderд�� " << remainingHeaderSize << " �ֽڵ���չ��Ϣͷ��" << endl;
				fout.write(reinterpret_cast<const char*>(m_extraHeader.data()), remainingHeaderSize);
				if (!fout) { cerr << "[����] д����չ��Ϣͷʧ��: " << filename << endl; fout.close(); return false; }
			}
			else {
				cerr << "[����] biSizeָʾ�����ͷ������m_extraHeader̫С��" << endl;
				fout.close(); return false; // ��һ�£��޷���ȫ����
			}
		}
	}
	else {
		cerr << "[����] ��Ϣͷ�е�biSize��Ч (" << infoHeader.biSize << ")��" << endl;
		fout.close(); return false;
	}

	// 3) д��ʣ��Ķ���ͷ/��ɫ������ (��� biSize > 40����Ӧ���� m_extraHeader ����չͷ֮��Ĳ���)
	size_t extraHeaderOffset = (infoHeader.biSize > sizeof(BmpInfoHeader)) ? (infoHeader.biSize - sizeof(BmpInfoHeader)) : 0;
	if (extraHeaderOffset < m_extraHeader.size()) {
		size_t paletteSize = m_extraHeader.size() - extraHeaderOffset;
		// cout << "[����] ���ڴ�m_extraHeaderд�� " << paletteSize << " �ֽڵĵ�ɫ�����ݡ�" << endl;
		fout.write(reinterpret_cast<const char*>(m_extraHeader.data() + extraHeaderOffset), paletteSize);
		if (!fout) { cerr << "[����] д���ɫ������ʧ��: " << filename << endl; fout.close(); return false; }
	}

	// ���д��ָ���Ƿ���Ԥ�ڵ� bfOffBits
	streampos currentWritePos = fout.tellp();
	if ((uint32_t)currentWritePos != fileHeader.bfOffBits) {
		cerr << "[����] �ļ�ָ�� (" << currentWritePos
			<< ") ����д������ǰ��Ԥ������ƫ��λ�� (bfOffBits=" << fileHeader.bfOffBits
			<< ")��ͷ����С���ܴ����д��ʧ�ܡ�" << endl;
		// ǿ�ƶ�λ�����ڸ����⣬�����Գ���
		// fout.seekp(fileHeader.bfOffBits, ios::beg);
	}


	// 4) д����������
	if (!m_pixelData.empty()) {
		// cout << "[����] ����д�� " << m_pixelData.size() << " �ֽڵ��������ݡ�" << endl; // ����
		fout.write(reinterpret_cast<const char*>(m_pixelData.data()), m_pixelData.size());
		// --- [����] ���д����������״̬ ---
		if (!fout) {
			cerr << "[����] д���������ݺ���������������: " << filename << endl;
			// ���Գ��Ի�ȡ����ϸ�Ĵ�����Ϣ�����������ھ���ʵ��
			fout.close();
			return false;
		}
	}

	// --- [����] ����ǿ��ˢ�»����� ---
	// cout << "[����] ˢ�������..." << endl;
	fout.flush();
	if (!fout) {
		cerr << "[����] ˢ�º���������: " << filename << endl;
		fout.close();
		return false;
	}

	// �ر��ļ�
	// cout << "[����] �ر������..." << endl;
	fout.close();

	// ���رպ������״̬
	if (!fout) {
		cerr << "[����] �رպ󱨸��ļ�������: " << filename << "���ļ����ܲ�������" << endl;
		// ��ʹ�رպ󱨸����Ҳ����ֵ�����ϲ������δ���
	}

	// --- [����] ���´��ļ������ʵ�ʴ�С ---
#ifdef _DEBUG // ֻ��Debugģʽ��ִ�м�飬����Ӱ��Release����
// ����ֱ���� #if 1 ǿ�ƿ���
	{
		ifstream checker(filename, ios::binary | ios::ate);
		if (checker.is_open()) {
			streampos realFileSize = checker.tellg();
			checker.close();
			cout << "[����] ������ļ���С��֤:" << endl;
			cout << "  ͷ��������bfSize: " << fileHeader.bfSize << endl;
			cout << "  �����ϵ�ʵ���ļ���С: " << (uint32_t)realFileSize << endl;
			if ((uint32_t)realFileSize != fileHeader.bfSize) {
				cerr << "[���ش���] �����������bfSize��ʵ���ļ���С��ƥ�䣡Ԥ�� "
					<< fileHeader.bfSize << "��ʵ�� " << (uint32_t)realFileSize << endl;
				// ������Կ��Ƿ��� false �����׳��쳣����Ϊ�ļ��϶���������
				return false; // ����ʧ�ܣ���Ϊ�ļ���һ��
			}
			else {
				cout << "[����] �ļ���С��֤�ɹ���" << endl;
			}
		}
		else {
			cerr << "[����] �޷����´��ѱ�����ļ� ('" << filename << "') ����֤��С��" << endl;
			// �޷���֤�������ܶ϶�����ʧ�ܣ����Ի����ȷ���true
		}
	}
#endif
	// --- ���Լ����� ---

	return true; // ����رճɹ������������debug��鷵��false��
}
