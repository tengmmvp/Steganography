#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <limits>
#include <locale>
#include <iomanip>

#include "BmpImage.h"
#include "StegoCore.h"

/**
 * @file main.cpp
 * @brief 基于 BMP 图像的隐写程序主入口
 */

using namespace std;

// 控制台颜色定义
namespace ConsoleColor {
	const string Reset = "\033[0m";
	const string Red = "\033[31m";
	const string Green = "\033[32m";
	const string Yellow = "\033[33m";
	const string Blue = "\033[34m";
	const string Magenta = "\033[35m";
	const string Cyan = "\033[36m";
	const string White = "\033[37m";
	const string Bold = "\033[1m";
}

/**
 * @brief 打印程序标题
 */
static void printTitle() {
	cout << ConsoleColor::Cyan << ConsoleColor::Bold;
	cout << "\n═════════ BMP 图像隐写数据隐藏与提取工具 ═════════\n"
		<< ConsoleColor::Reset;
}

/**
 * @brief 打印主菜单
 */
static void printMainMenu() {
	cout << "\n───────────────────── 主菜单 ─────────────────────\n";
	cout << ConsoleColor::Yellow;
	cout << "1. 隐藏数据到 BMP 图像\n";
	cout << "2. 从 BMP 图像提取数据\n";
	cout << "3. 配置隐写参数\n";
	cout << "4. 退出程序\n";
	cout << ConsoleColor::Reset;
	cout << "──────────────────────────────────────────────────\n";
	cout << "请输入选项 (1-4): ";
}

/**
 * @brief 打印当前隐写设置
 * @param ctx 隐写上下文
 */
static void printStegoSettings(const StegoContext& ctx) {
	cout << "\n────────────────── 当前隐写设置 ──────────────────\n";

	// 显示模式
	cout << "模式: ";
	cout << ConsoleColor::Yellow;
	switch (ctx.mode) {
	case LSB_SEQUENTIAL: cout << "顺序 LSB (1 bit)"; break;
	case LSB_RANDOM:     cout << "随机 LSB (1 bit)"; break;
	case LSB_ENHANCED:   cout << "增强 LSB (2 bit)"; break;
	default:             cout << "未知";             break;
	}
	cout << ConsoleColor::Reset << "\n";

	// 显示通道
	cout << "通道 (BGR): 0x" << hex << ctx.channelMask << dec << " (";
	if (ctx.channelMask & 0x01) cout << ConsoleColor::Blue << "蓝 " << ConsoleColor::Reset;
	if (ctx.channelMask & 0x02) cout << ConsoleColor::Green << "绿 " << ConsoleColor::Reset;
	if (ctx.channelMask & 0x04) cout << ConsoleColor::Red << "红 " << ConsoleColor::Reset;
	cout << ")\n";

	// 显示密码状态
	cout << "密码: " << (ctx.password.empty() ?
		ConsoleColor::Yellow + "(未设置)" + ConsoleColor::Reset :
		ConsoleColor::Green + "[已设置]" + ConsoleColor::Reset) << "\n";

	// 显示自动检测状态
	cout << "自动检测(仅提取): " << (ctx.autoDetect ?
		ConsoleColor::Green + "已启用" + ConsoleColor::Reset :
		ConsoleColor::Yellow + "已禁用" + ConsoleColor::Reset) << "\n";

	cout << "──────────────────────────────────────────────────\n";
}

/**
 * @brief 配置隐写参数
 * @param ctx 隐写上下文
 * @param isExtract 是否为提取流程
 */
static void configureStego(StegoContext& ctx, bool isExtract = false) {
	printStegoSettings(ctx);

	cout << ConsoleColor::Cyan << "[询问] " << ConsoleColor::Reset
		<< "是否修改设置? (y/n): ";
	char c; cin >> c;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	if (c != 'y' && c != 'Y') {
		cout << ConsoleColor::Green << "[提示] " << ConsoleColor::Reset << "使用当前设置\n";
		return;
	}

	bool skipModeChannelConfig = false;
	if (isExtract) {
		cout << ConsoleColor::Cyan << "[配置] " << ConsoleColor::Reset
			<< "是否启用自动检测? (y/n): ";
		char ad; cin >> ad;
		ctx.autoDetect = (ad == 'y' || ad == 'Y');
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		if (ctx.autoDetect) {
			cout << ConsoleColor::Green << "[提示] " << ConsoleColor::Reset
				<< "自动检测已启用，跳过模式/通道配置\n";
			skipModeChannelConfig = true;
		}
	}

	// 如果不跳过模式/通道配置，则配置这些参数
	if (!skipModeChannelConfig) {
		// 选择模式
		cout << "\n────────────────── 选择隐写模式 ──────────────────\n";
		cout << ConsoleColor::Yellow;
		cout << "1. 顺序 LSB (1 bit)\n";
		cout << "2. 随机 LSB (1 bit)\n";
		cout << "3. 增强 LSB (2 bit)\n";
		cout << ConsoleColor::Reset;
		cout << "──────────────────────────────────────────────────\n";
		cout << "请选择 (1-3): ";

		int m;
		while (!(cin >> m) || m < 1 || m > 3) {
			cout << ConsoleColor::Red << "[错误] " << ConsoleColor::Reset
				<< "输入无效，请输入 1-3: ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
		ctx.mode = static_cast<SteganoMode>(m - 1);
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		// 选择通道
		cout << "\n────────────────── 选择颜色通道 ──────────────────\n";
		cout << ConsoleColor::Yellow;
		cout << "1. " << ConsoleColor::Blue << "蓝" << ConsoleColor::Yellow << "(0x01)\n";
		cout << "2. " << ConsoleColor::Green << "绿" << ConsoleColor::Yellow << "(0x02)\n";
		cout << "3. " << ConsoleColor::Red << "红" << ConsoleColor::Yellow << "(0x04)\n";
		cout << "4. " << ConsoleColor::Blue << "蓝" << ConsoleColor::Yellow << "+" << ConsoleColor::Green << "绿" << ConsoleColor::Yellow << "(0x03)\n";
		cout << "5. " << ConsoleColor::Blue << "蓝" << ConsoleColor::Yellow << "+" << ConsoleColor::Red << "红" << ConsoleColor::Yellow << "(0x05)\n";
		cout << "6. " << ConsoleColor::Green << "绿" << ConsoleColor::Yellow << "+" << ConsoleColor::Red << "红" << ConsoleColor::Yellow << "(0x06)\n";
		cout << "7. " << ConsoleColor::Blue << "蓝" << ConsoleColor::Yellow << "+" << ConsoleColor::Green << "绿" << ConsoleColor::Yellow << "+" << ConsoleColor::Red << "红" << ConsoleColor::Yellow << "(0x07)\n";
		cout << ConsoleColor::Reset;
		cout << "──────────────────────────────────────────────────\n";
		cout << "请选择 (1-7): ";

		int ms;
		while (!(cin >> ms) || ms < 1 || ms > 7) {
			cout << ConsoleColor::Red << "[错误] " << ConsoleColor::Reset
				<< "输入无效，请输入 1-7: ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}

		switch (ms) {
		case 1: ctx.channelMask = 0x01; break;
		case 2: ctx.channelMask = 0x02; break;
		case 3: ctx.channelMask = 0x04; break;
		case 4: ctx.channelMask = 0x03; break;
		case 5: ctx.channelMask = 0x05; break;
		case 6: ctx.channelMask = 0x06; break;
		case 7: ctx.channelMask = 0x07; break;
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}

	// 设置密码
	cout << ConsoleColor::Cyan << "\n[配置] " << ConsoleColor::Reset
		<< "输入新密码 (留空则不使用): ";
	string pw; getline(cin, pw);
	ctx.password = pw;

	cout << ConsoleColor::Green << "[提示] " << ConsoleColor::Reset << "设置已更新\n";
	printStegoSettings(ctx);
}

/**
 * @brief 获取文件路径输入
 * @param prompt 提示信息
 * @return 用户输入的文件路径
 */
static string getFilePath(const string& prompt) {
	cout << ConsoleColor::Cyan << prompt << ConsoleColor::Reset;
	string path;
	getline(cin, path);
	return path;
}

/**
 * @brief 显示操作进度
 * @param message 进度消息
 */
static void showProgress(const string& message) {
	cout << ConsoleColor::Yellow << "[进行中] " << ConsoleColor::Reset
		<< message << "..." << endl;
}

/**
 * @brief 显示成功消息
 * @param message 成功消息
 */
static void showSuccess(const string& message) {
	cout << ConsoleColor::Green << "[成功] " << ConsoleColor::Reset
		<< message << endl;
}

/**
 * @brief 显示错误消息
 * @param message 错误消息
 */
static void showError(const string& message) {
	cerr << ConsoleColor::Red << "[错误] " << ConsoleColor::Reset
		<< message << endl;
}

/**
 * @brief 显示信息消息
 * @param message 信息消息
 */
static void showInfo(const string& message) {
	cout << ConsoleColor::Blue << "[信息] " << ConsoleColor::Reset
		<< message << endl;
}

/**
 * @brief 显示流程标题
 * @param title 流程标题
 */
static void showProcessTitle(const string& title) {
	cout << ConsoleColor::Magenta;
	cout << "\n===== " << title << " =====\n" << ConsoleColor::Reset;
}

int main() {
	// 设置本地化以支持中文
	try { setlocale(LC_ALL, ""); }
	catch (...) {}

	printTitle();

	StegoContext ctx;
	StegoCore    core;

	while (true) {
		printMainMenu();

		int choice;
		while (!(cin >> choice) || choice < 1 || choice > 4) {
			cout << ConsoleColor::Red << "[错误] " << ConsoleColor::Reset
				<< "输入无效，请输入 1-4: ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		if (choice == 4) {
			cout << ConsoleColor::Green << "\n[提示] " << ConsoleColor::Reset
				<< "感谢使用，程序已安全退出\n";
			break;
		}

		if (choice == 3) {
			showProcessTitle("隐写参数配置");
			configureStego(ctx, false);
			continue;
		}

		if (choice == 1) {
			// 隐藏流程
			showProcessTitle("数据隐藏流程");
			configureStego(ctx, false);

			string bmpPath = getFilePath("请输入原始 BMP 文件路径: ");
			BmpImage bmp;

			showProgress("加载 BMP 文件");
			if (!bmp.load(bmpPath)) {
				showError("加载A BMP 文件失败");
				continue;
			}
			showInfo("BMP 图像尺寸: " + to_string(bmp.getWidth()) + "x" + to_string(bmp.getHeight()));

			string inPath = getFilePath("请输入要隐藏的文件路径: ");
			ifstream fin(inPath, ios::binary | ios::ate);
			if (!fin.is_open()) {
				showError("打开文件失败: " + inPath);
				continue;
			}

			auto sz = fin.tellg();
			if (sz <= 0 || sz > numeric_limits<uint32_t>::max()) {
				showError("文件大小不合法: " + to_string(sz) + " 字节");
				fin.close();
				continue;
			}

			showInfo("待隐藏文件大小: " + to_string(sz) + " 字节");
			fin.seekg(0, ios::beg);
			vector<char> buffer(sz);
			fin.read(buffer.data(), sz);
			fin.close();

			showProgress("正在执行数据隐藏");
			if (core.hideData(bmp, buffer.data(), buffer.size(), ctx)) {
				string outBmp = getFilePath("请输入输出 BMP 文件路径: ");

				showProgress("保存隐写后的 BMP 文件");
				if (bmp.save(outBmp))
					showSuccess("隐写完成，输出文件: " + outBmp);
				else
					showError("保存 BMP 文件失败");
			}
			else {
				showError("数据隐藏失败，可能是图像容量不足或参数设置不当");
			}
		}
		else if (choice == 2) {
			// 提取流程
			showProcessTitle("数据提取流程");
			configureStego(ctx, true);

			string bmpPath = getFilePath("请输入含隐藏数据的 BMP 文件路径: ");
			BmpImage bmp;

			showProgress("加载 BMP 文件");
			if (!bmp.load(bmpPath)) {
				showError("加载 BMP 文件失败");
				continue;
			}
			showInfo("BMP 图像尺寸: " + to_string(bmp.getWidth()) + "x" + to_string(bmp.getHeight()));

			showProgress("正在提取隐藏数据");
			char* outBuf = nullptr;
			size_t outLen = 0;

			if (!core.extractData(bmp, outBuf, outLen, ctx)) {
				showInfo("提取失败或未检测到隐藏数据");
				delete[] outBuf;
			}
			else {
				showSuccess("提取到 " + to_string(outLen) + " 字节隐藏数据");

				cout << "检测到隐写模式: " << ConsoleColor::Yellow;
				switch (ctx.mode) {
				case LSB_SEQUENTIAL: cout << "顺序 LSB"; break;
				case LSB_RANDOM:     cout << "随机 LSB"; break;
				case LSB_ENHANCED:   cout << "增强 LSB"; break;
				}
				cout << ConsoleColor::Reset << "\n";

				cout << "检测到通道掩码: 0x" << hex << ctx.channelMask << dec << "\n";

				string savePath = getFilePath("请输入提取数据的保存路径: ");

				showProgress("保存提取的数据");
				ofstream fout(savePath, ios::binary);
				if (fout.is_open()) {
					fout.write(outBuf, outLen);
					fout.close();
					showSuccess("隐藏数据已保存到: " + savePath);
				}
				else {
					showError("无法创建输出文件: " + savePath);
				}
				delete[] outBuf;
			}
		}

		cout << "\n按 Enter 键继续...";
		cin.get();
	}

	return EXIT_SUCCESS;
}
