#include <windows.h>
#include <fstream>
#include <vector>

bool SaveBMP(const char* filename, HBITMAP hBitmap, HDC hdc, int width, int height) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;   // ảnh không bị lật ngược
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    int rowSize = ((width * 3 + 3) & ~3);
    int dataSize = rowSize * height;
    std::vector<BYTE> pixels(dataSize);

    GetDIBits(hdc, hBitmap, 0, height, pixels.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    BITMAPFILEHEADER bf;
    bf.bfType = 0x4D42; // 'BM'
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    file.write((char*)&bf, sizeof(bf));
    file.write((char*)&bi, sizeof(bi));
    file.write((char*)pixels.data(), dataSize);

    return true;
}

void TakeScreenshot() {
    int width  = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = GetDesktopWindow();
    HDC hdcScreen = GetDC(hwnd);

    HDC memDC = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(memDC, hBitmap);

    BitBlt(memDC, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

    SaveBMP("screenshot.bmp", hBitmap, hdcScreen, width, height);

    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hdcScreen);
}
