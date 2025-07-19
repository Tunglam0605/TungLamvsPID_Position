# MotorPID_Position (by Nguyễn Khắc Tùng Lâm)

Thư viện điều khiển vị trí động cơ DC sử dụng encoder và PID, hỗ trợ nhiều động cơ đồng thời, dành cho Arduino Due, Mega, Uno...

**Tác giả:** Nguyễn Khắc Tùng Lâm  
**Facebook:** Lâm Tùng  
**TikTok:** @Tunglam0605  
**SĐT:** 0325270213  

---

## 🚗 TÍNH NĂNG NỔI BẬT

- **Quản lý nhiều động cơ cùng lúc:** mỗi động cơ là một object độc lập, auto nhận diện ngắt – không cần attachInterrupt ngoài sketch.
- **Giữ vị trí chính xác bằng PID:** tự động scale hệ số PID khi sai số lớn để chạy nhanh, giữ chặt khi về đích – không rung.
- **moveNRound(n, autoCorrection):**  
  Quay đúng n vòng _thực tế_ so với góc hiện tại.
  - **Bù xung** (`autoCorrection = true`): chống cộng dồn sai số encoder/mất xung/trượt cơ khí.
  - **Không bù xung**: tăng góc lý thuyết, sai số sẽ cộng dồn (chỉ dùng để test).
- **Bật/tắt PID:** Motor tự do hoặc giữ vị trí bất kỳ ngay khi bật lại.
- **Đổi hệ số PID/giới hạn tích phân/xung-vòng mọi lúc.**
- **API trực quan, dễ dùng – style thương mại, code cực gọn!**

---

## ⚡️ CÀI ĐẶT

1. **Copy 2 file** `MotorPID_Position.h` và `MotorPID_Position.cpp` vào thư mục `libraries/MotorPID_Position` của Arduino IDE.
2. **Hoặc nén cả thư mục** thành `.zip` rồi cài từ IDE (Sketch > Include Library > Add .ZIP Library…).
3. **Tạo mới file .ino**, dùng như ví dụ phía dưới.

---

## 🎮 CÁCH SỬ DỤNG CƠ BẢN

### 1. **Khai báo động cơ**
```cpp
#include <MotorPID_Position.h>
#define PPR 900   // Số xung mỗi vòng (tuỳ encoder)

MotorPID_Position motor1(2, 3, 4, 5, 15, 0.1, 0.5, PPR);
MotorPID_Position motor2(18, 19, 6, 7, 18, 0.12, 0.6, PPR); // Thêm động cơ tuỳ thích
```
### 2. **Khởi tạo trong setup()**
```cpp
void setup() {
    Serial.begin(115200);
    motor1.Init();
    motor2.Init();
    motor1.Home();
    motor2.Home();
}
```
### 3. **Điều khiển vị trí/Quay n vòng trong loop()**
```cpp
// Giữ vị trí hiện tại:
motor1.Position(motor1.getCurrentAngle());
// Quay n vòng thực tế, có/không bù xung:
motor1.moveNRound(2);    // Quay +2 vòng
motor1.moveNRound(-1);  // Quay -1 vòng
```
## 🛠️ BẢNG HÀM API CHÍNH
```
Hàm / Method	--------------------- Chức năng chính
Position(angle)--------------------	Giữ vị trí góc (độ)
moveNRound(n)----------------------	Quay n vòng
setEnable(bool)--------------------	Tắt/bật PID (motor tự do/giữ vị trí mới)
Home()	--------------------------- Reset encoder về 0, dừng motor
Stop()	--------------------------- Ngắt motor ngay (PWM=0)
setPID(kp,ki,kd) ------------------	Đổi hệ số PID chính
setFastPIDOffset(kp_off, kd_off) -- Offset hệ số PID khi sai số lớn (auto scale)
setPulsePerRev(ppr)	--------------- Đổi số xung/vòng
setISat(val)	--------------------- Đổi giới hạn tích phân
getCurrentAngle()	----------------- Đọc góc hiện tại (độ)
getCurrentPulse()	----------------- Đọc số xung encoder hiện tại
```
## 🚩 VÍ DỤ ĐẦY ĐỦ – ĐIỀU KHIỂN 2 ĐỘNG CƠ
```cpp
#include "MotorPID_Position_V2.h"
#define PPR 3600  // Số xung trên 1 vòng encoder, chỉnh theo động cơ của bạn

// Khai báo 2 động cơ, tuỳ chân ENCA/ENCB/PWM thuận/nghịch & hệ số PID thực tế
MotorPID_Position motor1(37, 36, 2, 3, 5.65, 0.065, 0.1, PPR);
MotorPID_Position motor2(35, 34, 4, 5, 5.65, 0.065, 0.1, PPR);

void setup() {
    Serial.begin(115200);
    motor1.Init();
    motor2.Init();
    motor1.setEnable(false);
    motor2.setEnable(false);
    motor1.Home();
    motor2.Home();
    Serial.println(F("Gửi: ROUND 2 | ROUND2 -1 | HOME1 | HOME2 | OFF1 | ON1 | SET1 180 | SET2 -90"));
}
void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim(); // Bỏ ký tự trắng đầu/cuối
        Serial.print("Chuỗi nhận được: |"); Serial.print(cmd); Serial.println("|"); // Debug!

        // ROUND2 n
        if (cmd.startsWith("ROUND2")) {
            int idx = cmd.indexOf(' ');
            if (idx > 0) {
                String numStr = cmd.substring(idx + 1); numStr.trim();
                int n = numStr.toFloat();
                Serial.print("Motor2 quay "); Serial.print(n); Serial.println(" vòng.");
                motor2.moveNRound(n);
            }
        }
        // ROUND n
        else if (cmd.startsWith("ROUND")) {
            int idx = cmd.indexOf(' ');
            if (idx > 0) {
                String numStr = cmd.substring(idx + 1); numStr.trim();
                int n = numStr.toFloat();
                Serial.print("Motor1 quay "); Serial.print(n); Serial.println(" vòng.");
                motor1.moveNRound(n);
            }
        }
        // SET2 góc
        else if (cmd.startsWith("SET2")) {
            int idx = cmd.indexOf(' ');
            if (idx > 0) {
                String numStr = cmd.substring(idx + 1); numStr.trim();
                float goc = numStr.toFloat();
                Serial.print("Motor2 giữ góc: "); Serial.println(goc);
                motor2.Position(goc);
            }
        }
        // SET1 góc
        else if (cmd.startsWith("SET1")) {
            int idx = cmd.indexOf(' ');
            if (idx > 0) {
                String numStr = cmd.substring(idx + 1); numStr.trim();
                float goc = numStr.toFloat();
                Serial.print("Motor1 giữ góc: "); Serial.println(goc);
                motor1.Position(goc);
            }
        }
        // HOME1
        else if (cmd == "HOME1") {
            Serial.println("Motor1 về home!");
            motor1.Home();
        }
        // HOME2
        else if (cmd == "HOME2") {
            Serial.println("Motor2 về home!");
            motor2.Home();
        }
        // OFF1
        else if (cmd == "OFF1") {
            Serial.println("Motor1: PID OFF!");
            motor1.setEnable(false);
        }
        // ON1
        else if (cmd == "ON1") {
            Serial.println("Motor1: PID ON!");
            motor1.setEnable(true);
        }
        // OFF2
        else if (cmd == "OFF2") {
            Serial.println("Motor2: PID OFF!");
            motor2.setEnable(false);
        }
        // ON2
        else if (cmd == "ON2") {
            Serial.println("Motor2: PID ON!");
            motor2.setEnable(true);
        }
        // STOP1
        else if (cmd == "STOP1") {
            Serial.println("Motor1 stop!");
            motor1.Stop();
        }
        // STOP2
        else if (cmd == "STOP2") {
            Serial.println("Motor2 stop!");
            motor2.Stop();
        }
        // Unknown command
        else {
            Serial.println("Lệnh không hợp lệ hoặc sai cú pháp!");
        }
    }

    // Gọi PID cho từng motor (KHÔNG truyền tham số)
    motor1.Position();
    motor2.Position();

    // In trạng thái động cơ mỗi 300ms
    static unsigned long tPrint = 0;
    if (millis() - tPrint > 300) {
        Serial.print("[M1] Angle: "); Serial.print(motor1.getCurrentAngle(), 1);
        Serial.print(" | Pulse: "); Serial.print(motor1.getCurrentPulse());
        Serial.print(" | Target: "); Serial.print(motor1.getTargetAngle(), 1);
        Serial.print(" || [M2] Angle: "); Serial.print(motor2.getCurrentAngle(), 1);
        Serial.print(" | Pulse: "); Serial.print(motor2.getCurrentPulse());
        Serial.print(" | Target: "); Serial.println(motor2.getTargetAngle(), 1);
        tPrint = millis();
    }
}
```
## 🧑‍🔬 HƯỚNG DẪN DÒ PID – AUTO TỐI ƯU HIỆU QUẢ
Tăng dần kp đến khi motor giữ vị trí không bị lệch nhiều, bắt đầu hơi rung nhẹ (nếu quá lớn sẽ dao động mạnh).

Tăng kd để giảm rung, động cơ về đích “êm”, không lắc mạnh khi dừng.

Thêm nhẹ ki nếu còn sai số tĩnh (nhưng ki lớn quá sẽ gây tích phân bão hoà/motor dễ vượt quá).

Thử quay nhiều vòng với cả bù xung (true) và không bù xung (false) để kiểm tra độ lặp lại chính xác.

Lưu ý: Nếu dùng nhiều loại motor/encoder, thử scale offset nhanh setFastPIDOffset(kp_off, kd_off) để tăng lực khi chạy xa, giữ vị trí tốt khi về đích.

## 📱 HỖ TRỢ – GÓP Ý – NÂNG CẤP
Tác giả: Nguyễn Khắc Tùng Lâm
Facebook: Lâm Tùng
TikTok: @Tunglam0605
SDT: 0325270213

Có ý tưởng nâng cấp, thắc mắc, feedback? Cứ inbox, mình support tới bến!

## 🚀 Làm chủ mọi động cơ với MotorPID_Position – Best choice cho dân kỹ thuật điều khiển Gen Z!
