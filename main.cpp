/**
 * @file main.cpp
 * @brief BMPͼ����д������
 *
 * �ó���ʵ������BMPͼ�������غ���ȡ���ݵĹ��ܣ�֧�ֶ�����дģʽ�Ͳ������á�
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <locale>

#include "BmpImage.h"
#include "StegoCore.h"

using namespace std;

/**
 * @brief ��ӡ��ǰ����д����
 * @param ctx ��д�����Ķ��󣬰�����ǰ���ò���
 */
static void printStegoSettings(const StegoContext& ctx) {
	cout << "\n===== ��ǰ��д���� =====" << endl;
	cout << "ģʽ: ";
	switch (ctx.mode) {
	case LSB_SEQUENTIAL: cout << "˳�� LSB (1λ)"; break;
	case LSB_RANDOM:     cout << "��� LSB (1λ)";     break;
	case LSB_ENHANCED:   cout << "��ǿ LSB (˳�� 2λ)"; break;
	default:             cout << "δ֪ģʽ"; break;
	}
	cout << endl;

	cout << "ͨ�� (BGR): 0x" << hex << ctx.channelMask << dec << " (";
	if (ctx.channelMask & 0x01) cout << "�� "; // Blue
	if (ctx.channelMask & 0x02) cout << "�� "; // Green
	if (ctx.channelMask & 0x04) cout << "�� "; // Red
	cout << ")" << endl;

	if (!ctx.password.empty()) {
		cout << "����: [������]" << endl;
	}
	else {
		cout << "����: (δ����)" << endl;
	}
	cout << "�Զ���� (����ȡ): " << (ctx.autoDetect ? "������" : "�ѽ���") << endl;
	cout << "==========================================" << endl;
}

/**
 * @brief ������д����
 * @param ctx ��д�����Ķ��󣬽����޸�
 * @param isExtract �Ƿ�Ϊ��ȡ�������ã�Ӱ���Ƿ���ʾ�Զ����ѡ��
 */
static void configureStego(StegoContext& ctx, bool isExtract = false) {
	printStegoSettings(ctx);

	cout << "\n[ѯ��] �Ƿ��޸�����? (y/n): ";
	char c;
	cin >> c;
	// ������뻺�������Է����� getline ����
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	if (c == 'y' || c == 'Y') {
		if (isExtract) {
			cout << "\n[��ʾ] �Զ���⽫����ȡʱ���Զ���ģʽ��ͨ����" << endl;
			cout << "[����] �Ƿ������Զ����? (y/n): ";
			char adChoice;
			cin >> adChoice;
			ctx.autoDetect = (adChoice == 'y' || adChoice == 'Y');
			// ������뻺����
			cin.ignore(numeric_limits<streamsize>::max(), '\n');

			if (ctx.autoDetect) {
				cout << "[��ʾ] �������Զ���⡣��ȡʱ�������·���ģʽ��ͨ�����á�" << endl;
			}
		}

		cout << "\n----- ѡ����дģʽ -----" << endl;
		cout << "1. ˳�� LSB (1λ)" << endl;
		cout << "2. ��� LSB (1λ, ��Ҫ����)" << endl;
		cout << "3. ��ǿ LSB (˳�� 2λ)" << endl;
		cout << "[����] ѡ��ģʽ (1-3): ";
		int m;
		// ���������֤
		while (!(cin >> m) || m < 1 || m > 3) {
			cout << "[����] ������Ч�������� 1, 2, �� 3: ";
			cin.clear(); // ��������־
			cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ����������
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ������з�

		switch (m) {
		case 1: ctx.mode = LSB_SEQUENTIAL; break;
		case 2: ctx.mode = LSB_RANDOM;     break;
		case 3: ctx.mode = LSB_ENHANCED;   break;
		}

		cout << "\n----- ѡ����ɫͨ�� (BGR) -----" << endl;
		cout << "1. �� ��   (0x01)" << endl;
		cout << "2. �� ��   (0x02)" << endl;
		cout << "3. �� ��   (0x04)" << endl;
		cout << "4. �� + �� (0x03)" << endl;
		cout << "5. �� + �� (0x05)" << endl;
		cout << "6. �� + �� (0x06)" << endl;
		cout << "7. ���� (��+��+��) (0x07)" << endl;
		cout << "[����] ѡ��ͨ�� (1-7): ";
		int maskSel;
		while (!(cin >> maskSel) || maskSel < 1 || maskSel > 7) {
			cout << "[����] ������Ч�������� 1 �� 7: ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		switch (maskSel) {
		case 1: ctx.channelMask = 0x01; break;
		case 2: ctx.channelMask = 0x02; break;
		case 3: ctx.channelMask = 0x04; break;
		case 4: ctx.channelMask = 0x03; break;
		case 5: ctx.channelMask = 0x05; break;
		case 6: ctx.channelMask = 0x06; break;
		case 7: ctx.channelMask = 0x07; break;
		}

		cout << "\n----- �������� (���� XOR ��/�� ��� LSB) -----" << endl;
		cout << "[����] ���������� (������ʹ������): ";
		string pw;
		getline(cin, pw);
		ctx.password = pw;

		cout << "\n[��ʾ] �����Ѹ��¡�" << endl;
		printStegoSettings(ctx);
	}
	else {
		cout << "\n[��ʾ] ʹ�õ�ǰ���á�" << endl;
	}
}

/**
 * @brief ���������������
 * @return �����˳�״̬��
 *
 * ʵ����һ������ʽ�˵�ϵͳ�������û�ѡ���������ݡ���ȡ���ݻ�������д������
 */
int main() {
	// �������ñ��ػ�����ҪΪ����֧�ֵĻ�������ȷ��ʾ���ַ�
	try {
		setlocale(LC_ALL, "");
	}
	catch (const std::exception& e) {
		cerr << "[����] ���ñ��ػ�ʧ��: " << e.what() << endl;
	}

	cout << "======================================================" << endl;
	cout << "       ����ͼ�����Ϣ���� - ��BMPͼ������������       " << endl;
	cout << "======================================================" << endl;

	StegoContext ctx;      // ������д�����Ķ���
	StegoCore stegoCore; // ������д���Ĵ������

	while (true) {
		cout << "\n========== ���˵� ==========" << endl;
		cout << "1. �������ݵ� BMP" << endl;
		cout << "2. �� BMP ��ȡ����" << endl;
		cout << "3. ��������" << endl;
		cout << "4. �˳�" << endl;
		cout << "===========================" << endl;
		cout << "[����] ������ѡ�� (1-4): ";
		int choice;

		// ������֤
		while (!(cin >> choice) || choice < 1 || choice > 4) {
			cout << "[����] ������Ч�������� 1, 2, 3, �� 4: ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ���Ļ��з�

		if (choice == 4) {
			cout << "\n[��ʾ] �����˳�����" << endl;
			break; // �˳�ѭ��
		}
		else if (choice == 3) {
			configureStego(ctx, false); // ����ͨ������
			continue; // �������˵�
		}
		else if (choice == 1) { // Hide data
			cout << "\n--- �������� ---" << endl;
			configureStego(ctx, false); // ���/������������

			cout << "\n[����] ������ԭʼ BMP ͼ���ļ�·��: ";
			string bmpFile;
			getline(cin, bmpFile);

			BmpImage bmp;
			if (!bmp.load(bmpFile)) {
				cerr << "[����] ���� BMP �ļ�ʧ�ܡ�" << endl;
				continue; // �������˵�
			}

			// ���ͼ���Ƿ���Ч(���磬�Ƿ�Ϊ24/32λ)
			if (bmp.getBitCount() != 24 && bmp.getBitCount() != 32) {
				cerr << "[����] ��֧�� 24λ �� 32λ BMP ͼ���������ء�" << endl;
				continue;
			}
			if (bmp.getPixelDataSize() == 0) {
				cerr << "[����] BMP ͼ��û���������ݡ�" << endl;
				continue;
			}

			cout << "[����] ������Ҫ���ص��ļ�·��: ";
			string hideFile;
			getline(cin, hideFile);

			ifstream fin(hideFile, ios::binary | ios::ate); // ate: �򿪲���λ��ĩβ�Ի�ȡ��С
			if (!fin.is_open()) {
				cerr << "\n[����] �޷���Ҫ���ص��ļ�: " << hideFile << endl;
				continue;
			}

			streamsize fsize = fin.tellg(); // ��ȡ�ļ���С
			if (fsize <= 0) {
				cerr << "\n[����] Ҫ���ص��ļ�Ϊ�ջ��С��Ч: " << hideFile << endl;
				fin.close();
				continue;
			}
			if (fsize > numeric_limits<uint32_t>::max()) {
				cerr << "\n[����] Ҫ���ص��ļ����� (�����ڲ�����)��" << endl;
				fin.close();
				continue;
			}

			fin.seekg(0, ios::beg); // �ص��ļ���ͷ
			vector<char> buffer; // ʹ�� vector ��������
			try {
				buffer.resize(fsize);
				fin.read(buffer.data(), fsize);
				if (!fin) { // ����ȡ�����Ƿ�ɹ�
					cerr << "\n[����] δ��������ȡ�ļ�����: " << hideFile << endl;
					fin.close();
					continue;
				}
			}
			catch (const std::bad_alloc& e) {
				cerr << "\n[����] �����ڴ��ȡ�ļ�ʧ��: " << fsize << " �ֽڡ� " << e.what() << endl;
				fin.close();
				continue;
			}
			catch (const std::exception& e) {
				cerr << "\n[����] ��ȡ�ļ�ʱ�����������: " << e.what() << endl;
				fin.close();
				continue;
			}
			fin.close();

			cout << "\n[��ʾ] �������� " << buffer.size() << " �ֽ�..." << endl;
			if (stegoCore.hideData(bmp, buffer.data(), buffer.size(), ctx)) {
				cout << "\n[����] ������Ҫ�������� BMP �ļ�·�� (������������): ";
				string outBmp;
				getline(cin, outBmp);

				if (bmp.save(outBmp)) {
					cout << "\n[�ɹ�] �������سɹ�������ѱ��浽: " << outBmp << endl;
				}
				else {
					cerr << "\n[����] ������� BMP �ļ�ʧ�ܡ�" << endl;
				}
			}
			else {
				cerr << "\n[����] ��������ʧ�ܡ��������������á�" << endl;
			}
		}
		else if (choice == 2) {
			cout << "\n--- ��ȡ���� ---" << endl;
			configureStego(ctx, true);

			cout << "\n[����] ����������������ݵ� BMP ͼ���ļ�·��: ";
			string bmpFile;
			getline(cin, bmpFile);

			BmpImage bmp;
			if (!bmp.load(bmpFile)) {
				cerr << "[����] ���� BMP �ļ�ʧ�ܡ�" << endl;
				continue;
			}
			if (bmp.getPixelDataSize() == 0) {
				cerr << "[����] BMP ͼ��û�пɹ���ȡ���������ݡ�" << endl;
				continue;
			}

			char* outData = nullptr;
			size_t outLen = 0;
			cout << "\n[��ʾ] ������ȡ����..." << endl;
			if (!stegoCore.extractData(bmp, outData, outLen, ctx)) {
				cerr << "[��ʾ] ��ȡ����ʧ�ܣ���ʹ�õ�ǰ����/����δ�ҵ��������ݡ�" << endl;
				delete[] outData;
			}
			else {
				// ��ȡ�ɹ�
				cout << "\n[�ɹ�] �ɹ���ȡ " << outLen << " �ֽڵ��������ݡ�" << endl;
				cout << "  ��⵽��ģʽ: ";
				switch (ctx.mode) {
				case LSB_SEQUENTIAL: cout << "˳�� LSB (1λ)"; break;
				case LSB_RANDOM:     cout << "��� LSB (1λ)";     break;
				case LSB_ENHANCED:   cout << "��ǿ LSB (˳�� 2λ)"; break;
				default:             cout << "δ֪"; break;
				}
				cout << "\n  ��⵽��ͨ��: 0x" << hex << ctx.channelMask << dec << endl;

				cout << "[����] ������Ҫ������ȡ���ݵ��ļ�·��: ";
				string saveFile;
				getline(cin, saveFile);

				ofstream fout(saveFile, ios::binary);
				if (!fout.is_open()) {
					cerr << "\n[����] �޷���������ļ�: " << saveFile << endl;
				}
				else {
					fout.write(outData, outLen);
					fout.close();
					if (!fout) {
						cerr << "\n[����] δ�ܽ�������ȡ������д���ļ�: " << saveFile << endl;
					}
					else {
						cout << "\n[�ɹ�] ��ȡ�������ѳɹ����浽: " << saveFile << endl;
					}
				}
				delete[] outData;
				outData = nullptr;
			}
		}
	}

	return EXIT_SUCCESS;
}
