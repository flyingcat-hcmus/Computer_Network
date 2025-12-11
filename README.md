# CP

COMPILE = g++ -std=c++17 -I./ -I./asio/include server.cpp -o server.exe -lmswsock  -lws2_32 -lgdi32 -luser32

Nếu webcam lỗi !!! ERROR: S failed: 0xC00D5212 thì kiếm code sau:
hr = pInType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12); (hoặc ko có NV12 thì là YUY2)
đổi tên NV12 <-> YUY2 nếu có xảy ra lỗi

