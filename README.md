# STM32_FSoft

Một thư viện và demo tích hợp đầy đủ để sử dụng vi điều khiển STM32 (STM32F401/…/F4 series) với nền tảng FSoft — bao gồm cấu hình phần cứng (GPIO, UART, SPI, I²C, ADC…), ứng dụng mẫu (đọc cảm biến, LCD, RTC…), và hướng dẫn sử dụng.

## Mục lục

- [Giới thiệu](#giới-thiệu)
- [Tính năng](#tính-năng)
- [Yêu cầu](#yêu-cầu)
- [Cài đặt](#cài-đặt)
- [Cấu trúc thư mục](#cấu-trúc-thư-mục)
- [Hướng dẫn sử dụng](#hướng-dẫn-sử-dụng)
- [Tài liệu tham khảo](#tài-liệu-tham-khảo)
- [Đóng góp](#đóng-góp)
- [Giấy phép](#giấy-phép)

---

## Giới thiệu

STM32_FSoft là bộ thư viện hỗ trợ nhanh chóng cho các bạn sử dụng vi điều khiển STM32F4 (ví dụ STM32F401CC), sử dụng ngôn ngữ C/C++, dễ dàng tích hợp các module như cảm biến đất, đồng hồ thời gian thực, và giao diện LCD. Đây là cơ sở tốt để phát triển ứng dụng nhúng nhanh chóng.

---

## Tính năng

- Cấu hình khởi tạo MCU: SystemClock, NVIC, SysTick
- Thư viện điều khiển:
  - **UART**: giao tiếp máy tính
  - **SPI / I²C**: kết nối cảm biến, RTC DS3231
  - **ADC**: đọc cảm biến độ ẩm đất
  - **GPIO**: LED, nút nhấn, relay
- Mẫu ứng dụng:
  - Đọc cảm biến độ ẩm đất và xuất kết quả qua LCD 16×2
  - Đồng bộ thời gian qua DS3231
  - Kết nối với LabVIEW để điều khiển và giám sát dữ liệu

---

## Yêu cầu

- Vi điều khiển: STM32F4-series (ví dụ STM32F401CC)
- Toolchain: STM32CubeIDE hoặc Keil MDK
- Thư viện HAL (STM32CubeMX)
- Phụ kiện: cảm biến độ ẩm, RTC DS3231, module LCD 16×2

---

## Cài đặt

1. Clone dự án:
   ```bash
   git clone https://github.com/AnhHuy23/Stm32_fsoft.git
   cd Stm32_fsoft
