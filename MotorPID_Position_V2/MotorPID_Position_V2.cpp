/*
 * MotorPID_Position_V2.cpp - Multi-motor PID Position Library
 * Tác giả: Nguyễn Khắc Tùng Lâm
 * Facebook: Lâm Tùng | TikTok: @Tunglam0605 | SĐT: 0325270213
 *
 * Điều khiển vị trí động cơ DC dùng encoder + PID, đa động cơ (tối đa MAX_PID_MOTORS).
 * Chỉ target_angle đổi khi có lệnh mới, angle/pulse chỉ đổi khi motor quay thật.
 */

#include "MotorPID_Position_V2.h"

// --------- Static variables cho đa động cơ ----------
MotorPID_Position* MotorPID_Position::motorList[MAX_PID_MOTORS] = {nullptr};
uint8_t MotorPID_Position::motorCount = 0;

// --------- Constructor ----------
MotorPID_Position::MotorPID_Position(uint8_t enca, uint8_t encb, uint8_t pwmF, uint8_t pwmB,
                                     float kp, float ki, float kd,
                                     int pulses_per_rev, float dt, float iSat)
    : enca(enca), encb(encb), pwmF(pwmF), pwmB(pwmB),
      kp(kp), ki(ki), kd(kd), pulses_per_rev(pulses_per_rev),
      dt(dt), iSat(iSat)
{
    posi = 0;
    target_angle = 0;
    snapshot_angle = 0;
    eprev = 0;
    eintegral = 0;
    lastEncoded = 0;
    lastCompute = 0;
    enabled = true;
    autoCorrection = true;
    // Bộ PID nhanh (auto scale)
    kp_offset = 5.0f;    // Có thể tùy chỉnh
    kd_offset = 0.5f;
    kp_fast = kp + kp_offset;
    ki_fast = ki;
    kd_fast = kd + kd_offset;
}

// ---------- Khởi tạo & Đăng ký ngắt ----------
void MotorPID_Position::Init() {
    pinMode(enca, INPUT_PULLUP);
    pinMode(encb, INPUT_PULLUP);
    pinMode(pwmF, OUTPUT);
    pinMode(pwmB, OUTPUT);

    if (motorCount < MAX_PID_MOTORS) {
        motorList[motorCount++] = this;
        attachInterrupt(digitalPinToInterrupt(enca), globalEncoderISR_A, CHANGE);
        attachInterrupt(digitalPinToInterrupt(encb), globalEncoderISR_B, CHANGE);
    }
}

// ---------- ISR Encoder toàn cục ----------
void MotorPID_Position::globalEncoderISR_A() {
    for (int i = 0; i < motorCount; ++i)
        if (motorList[i]) motorList[i]->handleEncoderInterrupt();
}
void MotorPID_Position::globalEncoderISR_B() {
    for (int i = 0; i < motorCount; ++i)
        if (motorList[i]) motorList[i]->handleEncoderInterrupt();
}

// ---------- Xử lý xung encoder (Quadrature x4) ----------
void MotorPID_Position::handleEncoderInterrupt() {
    int MSB = digitalRead(enca);
    int LSB = digitalRead(encb);
    int encoded = (MSB << 1) | LSB;
    int sum = (lastEncoded << 2) | encoded;
    // Quadrature giải mã x4
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) posi++;
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) posi--;
    lastEncoded = encoded;
}

// ================= API CHÍNH ===================

// --- Gọi trong loop: chỉ PID về target_angle đã set ---
void MotorPID_Position::Position() {
    if (!enabled) return;
    unsigned long now = millis();
    if (now - lastCompute >= dt * 1000) {
        computePID();
        lastCompute = now;
    }
}

// --- Đặt góc đích mới, snapshot tại thời điểm lệnh ---
void MotorPID_Position::Position(float angle) {
    snapshot_angle = getCurrentAngle();    // Lấy vị trí gốc
    target_angle = angle;
    ResetPID();
}

// --- Quay n vòng từ vị trí hiện tại ---
void MotorPID_Position::moveNRound(float n) {
    snapshot_angle = getCurrentAngle();    // Lấy vị trí gốc
    float oldTarget = target_angle;
    target_angle = snapshot_angle + 360.0f * n;
    ResetPID();
}

// --- Tắt/bật PID ---
void MotorPID_Position::setEnable(bool en) {
    enabled = en;
    if (!en) {}// setMotor(0, 0);
    else {
        snapshot_angle = getCurrentAngle();
        target_angle = snapshot_angle;
        ResetPID();
    }
}

// --- Stop motor ---
void MotorPID_Position::Stop() { setMotor(0, 0); }

// --- Reset encoder, PID, target ---
void MotorPID_Position::Home() {
    setMotor(0, 0);
    posi = 0;
    snapshot_angle = 0;
    target_angle = 0;
    ResetPID();
}

// --- Reset bộ PID ---
void MotorPID_Position::ResetPID() { eprev = 0; eintegral = 0; }

// --- Đổi các hệ số/tùy chỉnh ---
void MotorPID_Position::setPulsePerRev(int ppr) { pulses_per_rev = ppr; }
void MotorPID_Position::setPID(float kp_, float ki_, float kd_) {
    kp = kp_; ki = ki_; kd = kd_;
    kp_fast = kp + kp_offset;
    ki_fast = ki;
    kd_fast = kd + kd_offset;
}
void MotorPID_Position::setFastPIDOffset(float kp_off, float kd_off) {
    kp_offset = kp_off;
    kd_offset = kd_off;
    kp_fast = kp + kp_offset;
    kd_fast = kd + kd_offset;
}
void MotorPID_Position::setISat(float iSat_) { iSat = iSat_; }
float MotorPID_Position::getCurrentAngle() { return (float)posi * 360.0f / (float)pulses_per_rev; }
long  MotorPID_Position::getCurrentPulse() { return posi; }
float MotorPID_Position::getTargetAngle() { return target_angle; }
bool  MotorPID_Position::isEnabled() { return enabled; }

// ================ PID CORE ================
void MotorPID_Position::computePID() {
    long target_pulse = (long)(target_angle * pulses_per_rev / 360.0f);
    long curr_pulse = posi;
    long error = target_pulse - curr_pulse;
    float error_deg = abs(error) * 360.0f / pulses_per_rev;

    // Tự động scale bộ PID
    float useKp = kp, useKi = ki, useKd = kd;
    if (error_deg > 360) { // Sai số lớn (1 vòng)
        useKp = kp_fast; useKi = ki_fast; useKd = kd_fast;
    } else if (error_deg > 90) {
        useKp = (kp + kp_fast) / 2.0f;
        useKi = (ki + ki_fast) / 2.0f;
        useKd = (kd + kd_fast) / 2.0f;
    }

    float dedt = (error - eprev) / dt;
    eintegral += error * dt;
    eintegral = constrain(eintegral, -iSat, iSat);

    float u = useKp * error + useKi * eintegral + useKd * dedt;
    eprev = error;

    int dir = (u > 0) ? 1 : (u < 0 ? -1 : 0);
    int pwr = constrain(abs((int)u), 0, 255);

    // Nếu gần target thì dừng hẳn motor
    if (abs(error) < 3) setMotor(0, 0);
    else setMotor(dir, pwr);
}

// ========== Điều khiển động cơ ==========
void MotorPID_Position::setMotor(int dir, int pwm) {
    pwm = constrain(pwm, 0, 255);
    if (dir == 1) {
        analogWrite(pwmF, pwm);
        analogWrite(pwmB, 0);
    } else if (dir == -1) {
        analogWrite(pwmF, 0);
        analogWrite(pwmB, pwm);
    } else {
        analogWrite(pwmF, 0);
        analogWrite(pwmB, 0);
    }
}
