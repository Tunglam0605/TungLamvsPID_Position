# MotorPID_Position (by Nguyễn Khắc Tùng Lâm)

Thư viện điều khiển vị trí động cơ DC sử dụng encoder và PID, hỗ trợ nhiều động cơ đồng thời, dành cho Arduino Due, Mega, Uno...

**Tác giả:** Nguyễn Khắc Tùng Lâm  
**Facebook:** Lâm Tùng  
**TikTok:** @Tunglam0605  
**SĐT:** 0325270213  

## Hướng dẫn sử dụng nhanh

1. **Khai báo object cho từng động cơ:**
```cpp
MotorPID_Position motor1(ENCA1, ENCB1, PWM1A, PWM1B, Kp1, Ki1, Kd1, pulses_per_rev);
------------------------------------------------------------------------------------
- Tạo object MotorPID_Position cho mỗi động cơ.
- Gọi .Init() trong setup().
- Trong loop gọi .Position(góc muốn giữ).
- Reset encoder gọi .Home().

## Ví dụ dùng 2 động cơ:
```cpp
#include <MotorPID_Position.h>

MotorPID_Position motor1(2, 3, 4, 5, 15, 0.1, 0.5, 900);
MotorPID_Position motor2(18, 19, 20, 21, 14, 0.13, 0.6, 900);

void setup() {
  Serial.begin(115200);
  motor1.Init(); motor2.Init();
  motor1.Home(); motor2.Home(); 
}
void loop() {
  motor1.Position(180);
  motor2.Position(90);
}

## Hướng dẫn dò thông số PID (Kp, Ki, Kd) cho động cơ
Cách làm thực tế, đơn giản dễ áp dụng cho mọi loại động cơ:
1 Đặt trước Ki = 0, Kd = 0.
2 Tăng dần Kp cho tới khi động cơ bắt đầu rung (dao động) hoặc về vị trí rất nhanh. Khi rung mạnh, giảm lại khoảng 10–20%.
3 Thêm Kd nhỏ (ví dụ 0.1–0.5), tăng dần đến khi giảm rung động và chuyển động mượt hơn.
4 Thêm Ki rất nhỏ (0.01–0.2), để loại bỏ sai số tĩnh nếu có (nếu motor giữ lâu vẫn lệch vị trí). Nếu thấy motor bị “giật”, vọt quá thì giảm lại.
Chỉ nên điều chỉnh một thông số mỗi lần, test thực tế để cảm nhận.
