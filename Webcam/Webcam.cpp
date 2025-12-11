#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

#define SAFE_RELEASE(x) if (x) { (x)->Release(); (x) = nullptr; }

#define CHECK_HR(hr_, msg)                                    \
    do {                                                      \
        if (FAILED(hr_)) {                                    \
            wprintf(L"!!! ERROR: %s failed: 0x%08X\n -> Read README.md",        \
                    L##msg, hr_);                             \
            goto done;                                        \
        }                                                     \
    } while (0)

int CaptureWebcamImage()
{
    HRESULT hr = S_OK;

    // --- Khai báo biến ---
    IMFAttributes* pAttributes       = nullptr;
    IMFActivate** ppDevices         = nullptr;
    UINT32           deviceCount       = 0;
    IMFMediaSource* pSource           = nullptr;
    IMFSourceReader* pReader           = nullptr;
    IMFSinkWriter* pWriter           = nullptr;

    IMFMediaType* pType             = nullptr;
    IMFMediaType* pNativeType       = nullptr;
    IMFMediaType* pOutType          = nullptr;
    IMFMediaType* pInType           = nullptr;

    DWORD            streamIndex       = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    DWORD            writerStreamIndex = 0;

    UINT32           width             = 0;
    UINT32           height            = 0;
    UINT32           frameRateNum      = 0;
    UINT32           frameRateDen      = 1;

    // Quay 10 giây (đơn vị 100-nanosecond)
    const LONGLONG   DURATION_10_SECONDS = 10LL * 10000000LL;
    LONGLONG         firstTimestamp    = -1;

    // 1. Khởi tạo COM và Media Foundation
    hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    CHECK_HR(hr, "CoInitializeEx");

    hr = MFStartup(MF_VERSION);
    CHECK_HR(hr, "MFStartup");

    // 2. Tìm Webcam
    hr = MFCreateAttributes(&pAttributes, 1);
    CHECK_HR(hr, "MFCreateAttributes");

    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    CHECK_HR(hr, "SetGUID");

    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &deviceCount);
    CHECK_HR(hr, "MFEnumDeviceSources");

    if (deviceCount == 0) {
        wprintf(L"Khong tim thay webcam nao.\n");
        goto done;
    }

    wprintf(L"Dang su dung webcam so 0...\n");
    hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pSource));
    CHECK_HR(hr, "ActivateObject");

    // 3. Tạo Source Reader
    hr = MFCreateSourceReaderFromMediaSource(pSource, nullptr, &pReader);
    CHECK_HR(hr, "MFCreateSourceReaderFromMediaSource");

    // --- Cấu hình Reader: Ép kiểu về NV12 (Định dạng chuẩn cho Encoder H264) ---
    hr = MFCreateMediaType(&pType);
    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
    
    hr = pReader->SetCurrentMediaType(streamIndex, nullptr, pType);
    CHECK_HR(hr, "SetCurrentMediaType (NV12)");
    SAFE_RELEASE(pType);

    // Lấy thông số thực tế của Webcam (Width, Height, FPS)
    hr = pReader->GetCurrentMediaType(streamIndex, &pNativeType);
    CHECK_HR(hr, "GetCurrentMediaType");

    hr = MFGetAttributeSize(pNativeType, MF_MT_FRAME_SIZE, &width, &height);
    CHECK_HR(hr, "Get Width/Height");

    hr = MFGetAttributeRatio(pNativeType, MF_MT_FRAME_RATE, &frameRateNum, &frameRateDen);
    CHECK_HR(hr, "Get FPS");

    wprintf(L"Format webcam: %ux%u @ %u/%u fps\n", width, height, frameRateNum, frameRateDen);

    // 4. Tạo Sink Writer (Output ra file MP4)
    // Xóa file cũ để tránh lỗi Access Denied
    DeleteFileW(L"webcam.mp4");
    hr = MFCreateSinkWriterFromURL(L"webcam.mp4", nullptr, nullptr, &pWriter);
    CHECK_HR(hr, "MFCreateSinkWriterFromURL");

    // --- Cấu hình Output Writer (H.264) ---
    hr = MFCreateMediaType(&pOutType);
    hr = pOutType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    hr = pOutType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    hr = pOutType->SetUINT32(MF_MT_AVG_BITRATE, 4000000); // 4 Mbps
    hr = pOutType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    hr = MFSetAttributeSize(pOutType, MF_MT_FRAME_SIZE, width, height);
    hr = MFSetAttributeRatio(pOutType, MF_MT_FRAME_RATE, frameRateNum, frameRateDen);
    hr = MFSetAttributeRatio(pOutType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

    hr = pWriter->AddStream(pOutType, &writerStreamIndex);
    CHECK_HR(hr, "AddStream");
    SAFE_RELEASE(pOutType);

    // --- Cấu hình Input Writer (Phải khớp với Reader: NV12) ---
    hr = MFCreateMediaType(&pInType);
    hr = pInType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    hr = pInType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
    hr = MFSetAttributeSize(pInType, MF_MT_FRAME_SIZE, width, height);
    hr = MFSetAttributeRatio(pInType, MF_MT_FRAME_RATE, frameRateNum, frameRateDen);
    hr = MFSetAttributeRatio(pInType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

    hr = pWriter->SetInputMediaType(writerStreamIndex, pInType, nullptr);
    CHECK_HR(hr, "SetInputMediaType");
    SAFE_RELEASE(pInType);

    // 5. Bắt đầu ghi
    hr = pWriter->BeginWriting();
    CHECK_HR(hr, "BeginWriting");

    firstTimestamp = -1;
    wprintf(L"-> Bat dau ghi hinh 10 giay...\n");

    while (true)
    {
        DWORD    actualStreamIndex = 0;
        DWORD    flags             = 0;
        LONGLONG timestamp         = 0;
        IMFSample* pSample         = nullptr;

        hr = pReader->ReadSample(
            streamIndex,
            0,
            &actualStreamIndex,
            &flags,
            &timestamp,
            &pSample
        );

        if (FAILED(hr)) {
            wprintf(L"ReadSample Error.\n");
            break;
        }

        // Kiểm tra cờ kết thúc hoặc lỗi stream
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
        if (flags & MF_SOURCE_READERF_STREAMTICK) {
             SAFE_RELEASE(pSample);
             continue;
        }

        if (!pSample) continue;

        // --- [QUAN TRỌNG] Xử lý lại Timestamp ---
        // Nếu không có đoạn này, file MP4 sẽ bị màn hình đen
        if (firstTimestamp < 0) {
            firstTimestamp = timestamp;
        }

        LONGLONG rebasedTime = timestamp - firstTimestamp;

        // Gán thời gian mới (bắt đầu từ 0) cho sample
        pSample->SetSampleTime(rebasedTime);
        
        // Tính duration cho mỗi frame (để player biết frame kéo dài bao lâu)
        // 10^7 / (Num/Den)
        LONGLONG frameDuration = (10000000LL * frameRateDen) / frameRateNum;
        pSample->SetSampleDuration(frameDuration);

        // Kiểm tra đã đủ 10 giây chưa
        if (rebasedTime >= DURATION_10_SECONDS) {
            SAFE_RELEASE(pSample);
            break;
        }

        // Ghi vào file
        hr = pWriter->WriteSample(writerStreamIndex, pSample);
        SAFE_RELEASE(pSample);

        if (FAILED(hr)) {
            wprintf(L"WriteSample failed: 0x%08X\n", hr);
            break;
        }
        
        // In dấu chấm để biết chương trình đang chạy
        // wprintf(L"."); 
    }

    // 6. Kết thúc và lưu file
    hr = pWriter->Finalize();
    CHECK_HR(hr, "Finalize");

    wprintf(L"\n-> XONG! File output: webcam.mp4\n");

done:
    // Cleanup
    if (ppDevices) {
        for (UINT32 i = 0; i < deviceCount; ++i) SAFE_RELEASE(ppDevices[i]);
        CoTaskMemFree(ppDevices);
    }
    SAFE_RELEASE(pAttributes);
    SAFE_RELEASE(pSource);
    SAFE_RELEASE(pReader);
    SAFE_RELEASE(pWriter);
    SAFE_RELEASE(pNativeType);

    MFShutdown();
    CoUninitialize();

    return FAILED(hr) ? -1 : 0;
}
