# Stm32_fsoft
## Kết nối phần cứng
Thiết bị STM32F401CCU6

DS1307 SDA	PB7

DS1307 SCL	PB6

LCD I2C SDA	PB7

LCD I2C SCL	PB6

GND tất cả	GND

VCC tất cả	3.3V (có thể dùng 5V nếu LCD hỗ trợ)
## Lưu ý: DS1307 và LCD I2C dùng chung bus I2C (nối song song PB6/PB7).
## Cấu hình STM32CubeIDE
1. Mở STM32CubeIDE, chọn chip STM32F401CCUx.

2. Bật I2C1 (Mode: I2C).

3. Cấu hình chân:

PB6: I2C1_SCL

PB7: I2C1_SDA

![image](https://github.com/user-attachments/assets/d3bef7f7-b95f-4083-9b15-80aa2f386091)

