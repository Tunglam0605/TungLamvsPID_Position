# MotorPID_Position (by Nguyễn Khắc Tùng Lâm)

Thư viện điều khiển vị trí động cơ DC sử dụng encoder và PID, hỗ trợ nhiều động cơ đồng thời, dành cho Arduino Due, Mega, Uno...

**Tác giả:** Nguyễn Khắc Tùng Lâm  
**Facebook:** Lâm Tùng  
**TikTok:** @Tunglam0605  
**SĐT:** 0325270213  

## Sử dụng nhanh:
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
