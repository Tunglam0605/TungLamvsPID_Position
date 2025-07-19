#include "MotorPID_Position_V2.h"
#define PPR 3600  // Số xung trên 1 vòng encoder, chỉnh theo động cơ của bạn

// Khai báo 2 động cơ, tuỳ chân ENCA/ENCB/PWM thuận/nghịch & hệ số PID thực tế
MotorPID_Position motor1(37, 36, 2, 3, 5.65, 0.065, 0.1, PPR);
MotorPID_Position motor2(35, 34, 4, 5, 5.65, 0.065, 0.1, PPR);

void HomeAllMotors() {
    Serial.println("=== Bắt đầu homing tất cả động cơ ===");
    bool home1Done = false;
    bool home2Done = false;

    // Đảm bảo PID đang TẮT để điều khiển tay
    motor1.setEnable(false);
    motor2.setEnable(false);

    // Quay đến khi về đúng vị trí home
    while (!(home1Done && home2Done)) {
        // Đọc cảm biến home A1, A2
        int homeA1 = analogRead(A1);
        int homeA2 = analogRead(A2);

        // Động cơ 1
        if (!home1Done) {
            if (homeA1 > 127) {
                analogWrite(3,200); // Quay thuận, chỉnh tốc độ tùy ý
            } else {
                analogWrite(2,0);   // Dừng lại
                home1Done = true;
            }
        }

        // Động cơ 2
        if (!home2Done) {
            if (homeA2 > 127) {
                analogWrite(4,200); // Quay thuận
            } else {
                analogWrite(4,0);
                home2Done = true;
            }
        }
        delay(5); // tránh loop quá nhanh, tránh nhiễu
    }

    // Đã về home xong => Reset encoder, bật lại PID giữ vị trí hiện tại (home)
    motor1.Home();  // reset encoder về 0
    motor2.Home();
    motor1.setEnable(true);
    motor2.setEnable(true);

    Serial.println("=== Đã homing xong, tất cả động cơ về 0 ===");
}


void setup() {
    Serial.begin(115200);
    motor1.Init();
    motor2.Init();
    HomeAllMotors();   // Chạy duy nhất 1 lần
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

    uint8_t sethome1 = analogRead(A1);
    uint8_t sethome2 = analogRead(A2);

    // In trạng thái động cơ mỗi 300ms
    static unsigned long tPrint = 0;
    if (millis() - tPrint > 300) {
       Serial.print("[M1] Home: "); Serial.print(sethome1, 1);
        Serial.print("[M2] Home: "); Serial.print(sethome2, 1);
        Serial.print("[M1] Angle: "); Serial.print(motor1.getCurrentAngle(), 1);
        Serial.print(" | Pulse: "); Serial.print(motor1.getCurrentPulse());
        Serial.print(" | Target: "); Serial.print(motor1.getTargetAngle(), 1);
        Serial.print(" || [M2] Angle: "); Serial.print(motor2.getCurrentAngle(), 1);
        Serial.print(" | Pulse: "); Serial.print(motor2.getCurrentPulse());
        Serial.print(" | Target: "); Serial.println(motor2.getTargetAngle(), 1);
        tPrint = millis();
    }
}