#ifndef MOTORPID_POSITION_H
#define MOTORPID_POSITION_H

#include <Arduino.h>

/*
 * MotorPID_Position.h - Multi-motor version (dynamic mapping)
 * Tác giả: Nguyễn Khắc Tùng Lâm
 * Facebook: Lâm Tùng | TikTok: @Tunglam0605 | SĐT: 0325270213
 *
 * Thư viện điều khiển vị trí động cơ DC sử dụng encoder & PID cho nhiều động cơ cùng lúc.
 * Tương thích Arduino Due, Mega, Uno... (ưu tiên Due/Mega vì nhiều interrupt hardware).
 *
 * Sử dụng:
 *   - Tạo object MotorPID_Position cho mỗi động cơ với các chân ENCA, ENCB, PWM thuận/nghịch, hệ số PID, pulses_per_rev.
 *   - Gọi .Init() trong setup() để đăng ký ngắt encoder, mapping động cơ tự động.
 *   - Trong loop, chỉ cần gọi .Position(góc) để giữ vị trí; muốn reset encoder gọi .Home();
 *   - Có thể đổi số xung/vòng, hệ số PID, giới hạn tích phân bất kỳ lúc nào.
 * 
 * Liên hệ - Kết nối:
 *   - Facebook: Lâm Tùng
 *   - TikTok: @Tunglam0605
 *   - SĐT: 0325270213
 */

#define MAX_PID_MOTORS 8    // Đổi tùy nhu cầu

class MotorPID_Position {
public:
    // ------ Khởi tạo 1 motor ------
    MotorPID_Position(uint8_t enca, uint8_t encb, uint8_t pwmF, uint8_t pwmB,
                      float kp, float ki, float kd,
                      int pulses_per_rev, float dt = 0.01, float iSat = 500.0f);

    void Init();                    // Đăng ký ISR encoder
    void Position();                // Gọi trong loop: giữ target hiện tại
    void Position(float angle);     // Đặt góc đích mới (độ)
    void moveNRound(float n);  // Quay n vòng (từ vị trí hiện tại)
    void setEnable(bool en);        // Bật/tắt PID (enable = true để giữ vị trí)
    void Stop();                    // Stop motor ngay
    void Home();                    // Reset encoder, PID, target về 0
    void ResetPID();                // Reset PID về 0 (không đổi vị trí)

    // ----- PID & params -----
    void setPulsePerRev(int ppr);
    void setPID(float kp, float ki, float kd);
    void setFastPIDOffset(float kp_off, float kd_off);
    void setISat(float iSat);

    // ----- Lấy trạng thái -----
    float getCurrentAngle();        // Lấy góc hiện tại (độ)
    long  getCurrentPulse();        // Lấy số xung encoder hiện tại
    float getTargetAngle();         // Lấy góc đích
    bool  isEnabled();              // Đang bật PID không

    // ------ ISR toàn cục ------
    static void globalEncoderISR_A();
    static void globalEncoderISR_B();

private:
    uint8_t enca, encb, pwmF, pwmB;
    volatile long posi;
    int pulses_per_rev;
    float kp, ki, kd;
    float dt;
    float iSat;
    float kp_offset = 5.0f, kd_offset = 0.5f;
    float kp_fast, ki_fast, kd_fast;
    float eprev, eintegral;
    float target_angle, snapshot_angle;
    volatile int lastEncoded;
    unsigned long lastCompute;
    bool enabled, autoCorrection;

    static MotorPID_Position* motorList[MAX_PID_MOTORS];
    static uint8_t motorCount;

    void handleEncoderInterrupt();      // ISR nội bộ: KHÔNG THAM SỐ!
    void computePID();
    void setMotor(int dir, int pwm);
};

#endif