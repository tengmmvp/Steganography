/**
 * @file main.cpp
 * @brief BMP图像隐写主程序
 *
 * 该程序实现了在BMP图像中隐藏和提取数据的功能，支持多种隐写模式和参数配置。
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
 * @brief 打印当前的隐写配置
 * @param ctx 隐写上下文对象，包含当前配置参数
 */
static void printStegoSettings(const StegoContext& ctx) {
	cout << "\n===== 当前隐写设置 =====" << endl;
	cout << "模式: ";
	switch (ctx.mode) {
	case LSB_SEQUENTIAL: cout << "顺序 LSB (1位)"; break;
	case LSB_RANDOM:     cout << "随机 LSB (1位)";     break;
	case LSB_ENHANCED:   cout << "增强 LSB (顺序 2位)"; break;
	default:             cout << "未知模式"; break;
	}
	cout << endl;

	cout << "通道 (BGR): 0x" << hex << ctx.channelMask << dec << " (";
	if (ctx.channelMask & 0x01) cout << "蓝 "; // Blue
	if (ctx.channelMask & 0x02) cout << "绿 "; // Green
	if (ctx.channelMask & 0x04) cout << "红 "; // Red
	cout << ")" << endl;

	if (!ctx.password.empty()) {
		cout << "密码: [已设置]" << endl;
	}
	else {
		cout << "密码: (未设置)" << endl;
	}
	cout << "自动检测 (仅提取): " << (ctx.autoDetect ? "已启用" : "已禁用") << endl;
	cout << "==========================================" << endl;
}

/**
 * @brief 配置隐写参数
 * @param ctx 隐写上下文对象，将被修改
 * @param isExtract 是否为提取操作配置，影响是否显示自动检测选项
 */
static void configureStego(StegoContext& ctx, bool isExtract = false) {
	printStegoSettings(ctx);

	cout << "\n[询问] 是否修改设置? (y/n): ";
	char c;
	cin >> c;
	// 清除输入缓冲区，以防后续 getline 问题
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	if (c == 'y' || c == 'Y') {
		if (isExtract) {
			cout << "\n[提示] 自动检测将在提取时尝试多种模式和通道。" << endl;
			cout << "[配置] 是否启用自动检测? (y/n): ";
			char adChoice;
			cin >> adChoice;
			ctx.autoDetect = (adChoice == 'y' || adChoice == 'Y');
			// 清除输入缓冲区
			cin.ignore(numeric_limits<streamsize>::max(), '\n');

			if (ctx.autoDetect) {
				cout << "[提示] 已启用自动检测。提取时将忽略下方的模式和通道设置。" << endl;
			}
		}

		cout << "\n----- 选择隐写模式 -----" << endl;
		cout << "1. 顺序 LSB (1位)" << endl;
		cout << "2. 随机 LSB (1位, 需要密码)" << endl;
		cout << "3. 增强 LSB (顺序 2位)" << endl;
		cout << "[配置] 选择模式 (1-3): ";
		int m;
		// 添加输入验证
		while (!(cin >> m) || m < 1 || m > 3) {
			cout << "[错误] 输入无效。请输入 1, 2, 或 3: ";
			cin.clear(); // 清除错误标志
			cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 丢弃坏输入
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除换行符

		switch (m) {
		case 1: ctx.mode = LSB_SEQUENTIAL; break;
		case 2: ctx.mode = LSB_RANDOM;     break;
		case 3: ctx.mode = LSB_ENHANCED;   break;
		}

		cout << "\n----- 选择颜色通道 (BGR) -----" << endl;
		cout << "1. 仅 蓝   (0x01)" << endl;
		cout << "2. 仅 绿   (0x02)" << endl;
		cout << "3. 仅 红   (0x04)" << endl;
		cout << "4. 蓝 + 绿 (0x03)" << endl;
		cout << "5. 蓝 + 红 (0x05)" << endl;
		cout << "6. 绿 + 红 (0x06)" << endl;
		cout << "7. 所有 (蓝+绿+红) (0x07)" << endl;
		cout << "[配置] 选择通道 (1-7): ";
		int maskSel;
		while (!(cin >> maskSel) || maskSel < 1 || maskSel > 7) {
			cout << "[错误] 输入无效。请输入 1 到 7: ";
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

		cout << "\n----- 设置密码 (用于 XOR 和/或 随机 LSB) -----" << endl;
		cout << "[配置] 输入新密码 (留空则不使用密码): ";
		string pw;
		getline(cin, pw);
		ctx.password = pw;

		cout << "\n[提示] 设置已更新。" << endl;
		printStegoSettings(ctx);
	}
	else {
		cout << "\n[提示] 使用当前设置。" << endl;
	}
}

/**
 * @brief 主函数，程序入口
 * @return 程序退出状态码
 *
 * 实现了一个交互式菜单系统，允许用户选择隐藏数据、提取数据或配置隐写参数。
 */
int main() {
	// 尝试设置本地化，主要为了在支持的环境下正确显示宽字符
	try {
		setlocale(LC_ALL, "");
	}
	catch (const std::exception& e) {
		cerr << "[警告] 设置本地化失败: " << e.what() << endl;
	}

	cout << "======================================================" << endl;
	cout << "       基于图像的信息隐藏 - 在BMP图像中隐藏数据       " << endl;
	cout << "======================================================" << endl;

	StegoContext ctx;      // 创建隐写上下文对象
	StegoCore stegoCore; // 创建隐写核心处理对象

	while (true) {
		cout << "\n========== 主菜单 ==========" << endl;
		cout << "1. 隐藏数据到 BMP" << endl;
		cout << "2. 从 BMP 提取数据" << endl;
		cout << "3. 配置设置" << endl;
		cout << "4. 退出" << endl;
		cout << "===========================" << endl;
		cout << "[输入] 请输入选项 (1-4): ";
		int choice;

		// 输入验证
		while (!(cin >> choice) || choice < 1 || choice > 4) {
			cout << "[错误] 输入无效。请输入 1, 2, 3, 或 4: ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 消耗换行符

		if (choice == 4) {
			cout << "\n[提示] 正在退出程序。" << endl;
			break; // 退出循环
		}
		else if (choice == 3) {
			configureStego(ctx, false); // 配置通用设置
			continue; // 返回主菜单
		}
		else if (choice == 1) { // Hide data
			cout << "\n--- 隐藏数据 ---" << endl;
			configureStego(ctx, false); // 检查/配置隐藏设置

			cout << "\n[输入] 请输入原始 BMP 图像文件路径: ";
			string bmpFile;
			getline(cin, bmpFile);

			BmpImage bmp;
			if (!bmp.load(bmpFile)) {
				cerr << "[错误] 加载 BMP 文件失败。" << endl;
				continue; // 返回主菜单
			}

			// 检查图像是否有效(例如，是否为24/32位)
			if (bmp.getBitCount() != 24 && bmp.getBitCount() != 32) {
				cerr << "[错误] 仅支持 24位 或 32位 BMP 图像用于隐藏。" << endl;
				continue;
			}
			if (bmp.getPixelDataSize() == 0) {
				cerr << "[错误] BMP 图像没有像素数据。" << endl;
				continue;
			}

			cout << "[输入] 请输入要隐藏的文件路径: ";
			string hideFile;
			getline(cin, hideFile);

			ifstream fin(hideFile, ios::binary | ios::ate); // ate: 打开并定位到末尾以获取大小
			if (!fin.is_open()) {
				cerr << "\n[错误] 无法打开要隐藏的文件: " << hideFile << endl;
				continue;
			}

			streamsize fsize = fin.tellg(); // 获取文件大小
			if (fsize <= 0) {
				cerr << "\n[错误] 要隐藏的文件为空或大小无效: " << hideFile << endl;
				fin.close();
				continue;
			}
			if (fsize > numeric_limits<uint32_t>::max()) {
				cerr << "\n[错误] 要隐藏的文件过大 (超过内部限制)。" << endl;
				fin.close();
				continue;
			}

			fin.seekg(0, ios::beg); // 回到文件开头
			vector<char> buffer; // 使用 vector 管理缓冲区
			try {
				buffer.resize(fsize);
				fin.read(buffer.data(), fsize);
				if (!fin) { // 检查读取操作是否成功
					cerr << "\n[错误] 未能完整读取文件内容: " << hideFile << endl;
					fin.close();
					continue;
				}
			}
			catch (const std::bad_alloc& e) {
				cerr << "\n[错误] 分配内存读取文件失败: " << fsize << " 字节。 " << e.what() << endl;
				fin.close();
				continue;
			}
			catch (const std::exception& e) {
				cerr << "\n[错误] 读取文件时发生意外错误: " << e.what() << endl;
				fin.close();
				continue;
			}
			fin.close();

			cout << "\n[提示] 尝试隐藏 " << buffer.size() << " 字节..." << endl;
			if (stegoCore.hideData(bmp, buffer.data(), buffer.size(), ctx)) {
				cout << "\n[输入] 请输入要保存的输出 BMP 文件路径 (包含隐藏数据): ";
				string outBmp;
				getline(cin, outBmp);

				if (bmp.save(outBmp)) {
					cout << "\n[成功] 数据隐藏成功。输出已保存到: " << outBmp << endl;
				}
				else {
					cerr << "\n[错误] 保存输出 BMP 文件失败。" << endl;
				}
			}
			else {
				cerr << "\n[错误] 隐藏数据失败。请检查容量和设置。" << endl;
			}
		}
		else if (choice == 2) {
			cout << "\n--- 提取数据 ---" << endl;
			configureStego(ctx, true);

			cout << "\n[输入] 请输入包含隐藏数据的 BMP 图像文件路径: ";
			string bmpFile;
			getline(cin, bmpFile);

			BmpImage bmp;
			if (!bmp.load(bmpFile)) {
				cerr << "[错误] 加载 BMP 文件失败。" << endl;
				continue;
			}
			if (bmp.getPixelDataSize() == 0) {
				cerr << "[错误] BMP 图像没有可供提取的像素数据。" << endl;
				continue;
			}

			char* outData = nullptr;
			size_t outLen = 0;
			cout << "\n[提示] 尝试提取数据..." << endl;
			if (!stegoCore.extractData(bmp, outData, outLen, ctx)) {
				cerr << "[提示] 提取数据失败，或使用当前设置/密码未找到隐藏数据。" << endl;
				delete[] outData;
			}
			else {
				// 提取成功
				cout << "\n[成功] 成功提取 " << outLen << " 字节的隐藏数据。" << endl;
				cout << "  检测到的模式: ";
				switch (ctx.mode) {
				case LSB_SEQUENTIAL: cout << "顺序 LSB (1位)"; break;
				case LSB_RANDOM:     cout << "随机 LSB (1位)";     break;
				case LSB_ENHANCED:   cout << "增强 LSB (顺序 2位)"; break;
				default:             cout << "未知"; break;
				}
				cout << "\n  检测到的通道: 0x" << hex << ctx.channelMask << dec << endl;

				cout << "[输入] 请输入要保存提取数据的文件路径: ";
				string saveFile;
				getline(cin, saveFile);

				ofstream fout(saveFile, ios::binary);
				if (!fout.is_open()) {
					cerr << "\n[错误] 无法创建输出文件: " << saveFile << endl;
				}
				else {
					fout.write(outData, outLen);
					fout.close();
					if (!fout) {
						cerr << "\n[错误] 未能将所有提取的数据写入文件: " << saveFile << endl;
					}
					else {
						cout << "\n[成功] 提取的数据已成功保存到: " << saveFile << endl;
					}
				}
				delete[] outData;
				outData = nullptr;
			}
		}
	}

	return EXIT_SUCCESS;
}
