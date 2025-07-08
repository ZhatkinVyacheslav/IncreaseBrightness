#define NOMINMAX
#include <Windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <algorithm>
#include <string>
#include <iostream>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

// Функция для увелечения яркости на заданное значение
void IncreaseBrightness(Gdiplus::Bitmap* img, int brightnessValue) {

    // Цикл по всем пикселям  
    for (int y = 0; y < img->GetHeight(); y++) {
        for (int x = 0; x < img->GetWidth(); x++) {
            Gdiplus::Color pixel;
            img->GetPixel(x, y, &pixel);

            // Увеличение каждого канала RGB на определённое значение 
            int newR = std::min(255, static_cast<int>(pixel.GetR()) + brightnessValue);
            int newG = std::min(255, static_cast<int>(pixel.GetG()) + brightnessValue);
            int newB = std::min(255, static_cast<int>(pixel.GetB()) + brightnessValue);

            // Запись нового цвета в пиксель
            img->SetPixel(x, y, Gdiplus::Color(
                pixel.GetA(),
                static_cast<BYTE>(newR),
                static_cast<BYTE>(newG),
                static_cast<BYTE>(newB)
            ));
        }
    }
}

// Функция для поиска кодека, с помощью которого будет сохранение изображения
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);

    // Проверка наличия кодеков
    if (size == 0) {
        std::cerr << "No image encoders found." << std::endl;
        return -1;
    }

    // Проверрка выделения памяти
    Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) {
        std::cerr << "Failed to allocate memory for encoder info." << std::endl;
        return -1;
    }

    // Поиск нужного кодека 
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    // Вывод ошибки при ненахождении кодека
    std::cerr << "Encoder not found" << std::endl;
    free(pImageCodecInfo);
    return -1;
}

int main() {
    // Инициализация сеанса GDI
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Загрузка изображении
    Gdiplus::Bitmap* img = new Gdiplus::Bitmap(L"data/input.jpg");
    if (!img || img->GetLastStatus() != Gdiplus::Ok) {
        std::cerr << "Failed to load image" << std::endl;
        return -1;
    }

    // Вызов функции для увелечения яркости на 40
    IncreaseBrightness(img, 40);

    // Получаение пути где будет сохранён результат 
    WCHAR buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    PathRemoveFileSpecW(buffer);
    std::wstring outputPath = std::wstring(buffer) + L"\\result\\output.jpg";

    // Сохранение результата
    CLSID encoderClsid;
    if (GetEncoderClsid(L"image/jpeg", &encoderClsid) != -1) {
        img->Save(outputPath.c_str(), &encoderClsid, NULL);
        std::cout << "New image saved" << std::endl;
    } else {
        std::cerr << "Failed to save" << std::endl;
    }

    delete img;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}
