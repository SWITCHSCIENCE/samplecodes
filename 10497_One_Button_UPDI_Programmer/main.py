#!/usr/bin/env python3
"""
main.py: Raspberry Pi Zero 2 W で ATtiny1616 に UPDI 経由で書き込むプログラム
"""

import os
import time
import subprocess
import glob
import RPi.GPIO as GPIO
import smbus
import sys
import shutil

# イベント検出用フラグ
file_select_event = False
file_select_prev_event = False

def on_file_select(channel):
    """ボタン2割り込みコールバック（次のファイルへ）"""
    global file_select_event
    # 既に処理中の場合は無視（二重実行防止）
    if not file_select_event:
        file_select_event = True
        print("SW2割り込み検出")

def on_file_select_prev(channel):
    """ボタン3割り込みコールバック（前のファイルへ）"""
    global file_select_prev_event
    # 既に処理中の場合は無視（二重実行防止）
    if not file_select_prev_event:
        file_select_prev_event = True
        print("SW3割り込み検出")

# SW1(書き込みボタン)の割り込みフラグ
write_button_pressed = False

def on_write_button(channel):
    """ボタン1割り込みコールバック"""
    global write_button_pressed
    # 既に処理中の場合は無視（二重実行防止）
    if not write_button_pressed:
        write_button_pressed = True
        print("SW1割り込み検出")

class LED:
    """GPIO ピン制御用の LED クラス（LOW で点灯、HIGH で消灯）"""

    def __init__(self, pin):
        """
        Args:
            pin (int): BCM モードの GPIO ピン番号
        """
        self.pin = pin
        GPIO.setup(self.pin, GPIO.OUT)
        GPIO.output(self.pin, GPIO.HIGH)  # 初期状態は消灯

    def on(self):
        """LED を点灯する（LOW 出力）"""
        GPIO.output(self.pin, GPIO.LOW)

    def off(self):
        """LED を消灯する（HIGH 出力）"""
        GPIO.output(self.pin, GPIO.HIGH)


class Buzzer:
    """GPIO ピン制御用のブザー クラス"""

    def __init__(self, pin):
        """
        Args:
            pin (int): BCM モードの GPIO ピン番号
        """
        self.pin = pin
        GPIO.setup(self.pin, GPIO.OUT)
        GPIO.output(self.pin, GPIO.LOW)

    def play_tone(self, frequency, duration_sec):
        """
        指定した周波数で指定時間ブザーを鳴らす
        Args:
            frequency (float): 周波数（Hz）
            duration_sec (float): 鳴動時間（秒）
        """
        pwm = GPIO.PWM(self.pin, frequency)
        pwm.start(50)  # デューティ比50%で駆動
        time.sleep(duration_sec)
        pwm.stop()
        GPIO.output(self.pin, GPIO.LOW)
    
    def play_success(self):
        """成功時のメロディー: 楽しい上昇音階（約1.5秒）"""
        # ド(C4): 261.626Hz
        # ミ(E4): 329.628Hz
        # ソ(G4): 391.995Hz
        # ド(C5): 523.251Hz
        self.play_tone(261.626, 0.15)  # ド
        time.sleep(0.03)
        self.play_tone(329.628, 0.15)  # ミ
        time.sleep(0.03)
        self.play_tone(391.995, 0.15)  # ソ
        time.sleep(0.03)
        self.play_tone(523.251, 0.2)   # ド（高）
        time.sleep(0.06)
        self.play_tone(391.995, 0.15)  # ソ
        time.sleep(0.03)
        self.play_tone(523.251, 0.35)  # ド（高）長め
    
    def play_error(self):
        """失敗時のメロディー: 悲しい下降音階（約1.5秒）"""
        # ソ(G4): 391.995Hz
        # ファ(F4): 349.228Hz
        # ミ(E4): 329.628Hz
        # レ(D4): 293.665Hz
        # ド(C4): 261.626Hz
        self.play_tone(391.995, 0.2)   # ソ
        time.sleep(0.03)
        self.play_tone(349.228, 0.2)   # ファ
        time.sleep(0.03)
        self.play_tone(329.628, 0.2)   # ミ
        time.sleep(0.03)
        self.play_tone(293.665, 0.2)   # レ
        time.sleep(0.03)
        self.play_tone(261.626, 0.4)   # ド（長く悲しく）


class Button:
    """GPIO ピン制御用のボタン クラス"""

    def __init__(self, pin, bounce_time=0.01, pull_up=False):
        """
        Args:
            pin (int): BCM モードの GPIO ピン番号
            bounce_time (float): デバウンス時間（秒）
            pull_up (bool): プルアップ(True)またはプルダウン(False)設定
        """
        self.pin = pin
        self.bounce_time = bounce_time
        self.pull_up = pull_up
        pud = GPIO.PUD_UP if pull_up else GPIO.PUD_DOWN
        GPIO.setup(self.pin, GPIO.IN, pull_up_down=pud)
        self.active_level = GPIO.LOW if pull_up else GPIO.HIGH

    def is_active(self):
        """
        Returns:
            bool: ボタンがアクティブ状態（押されている）かどうか
        """
        return GPIO.input(self.pin) == self.active_level
        

class LCD:
    """I2C接続のLCDディスプレイクラス"""
    LCD_CONTROL_REGISTER = 0x00
    LCD_DATA_REGISTER = 0x40
    CMD_FUNCTIONSET = 0x38
    CMD_BIAS_OSC = 0x14
    CMD_CONTRAST_SET = 0x70
    CMD_POWER_ICON_CTRL = 0x5C
    CMD_FOLLOWER_CTRL = 0x6C
    CMD_DISPLAY_ON = 0x0C
    CMD_CLEAR = 0x01
    CMD_ENTRY_MODE = 0x06

    def __init__(self, address=0x3e, backlight_pin=None, busnum=1):
        """
        Args:
            address (int): I2Cアドレス
            backlight_pin (int): バックライト制御GPIOピン番号
            busnum (int): I2Cバス番号
        """
        self.address = address
        self.bus = smbus.SMBus(busnum)
        self.backlight_pin = backlight_pin
        if backlight_pin is not None:
            GPIO.setup(backlight_pin, GPIO.OUT)
            GPIO.output(backlight_pin, GPIO.HIGH)
        time.sleep(0.05)
        # 初期化シーケンス (AQM0802A / ST7032) - 拡張命令セットモード
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x39])
        time.sleep(0.005)
        # 内部発振周波数設定
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x14])
        time.sleep(0.005)
        # コントラスト設定下位4bit - 適切な値に設定 (0x04)
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x70 | 0x04])
        time.sleep(0.005)
        # コントラスト設定上位2bit + ブースタON - 中間値に設定 (0x01)
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x5C | 0x01])
        time.sleep(0.005)
        # フォロワー制御 - 適切な電圧値に設定
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x6C])
        time.sleep(0.2)
        # 標準命令セットに戻す
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x38])
        time.sleep(0.005)
        # 表示オン, カーソルオフ
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x0C])
        time.sleep(0.005)
        # 表示クリア
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x01])
        time.sleep(0.002)
        # エントリモード設定
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [0x06])
        time.sleep(0.005)

    def clear(self):
        """LCDの表示をクリアします."""
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [self.CMD_CLEAR])
        time.sleep(0.002)

    def display(self, text, line=0):
        """指定行にテキストを表示します."""
        addr = 0x80 + (0x40 * line)
        # DDRAMアドレス設定
        self.bus.write_i2c_block_data(self.address, self.LCD_CONTROL_REGISTER, [addr])
        # データ書込み
        for c in text.ljust(8)[:8]:
            self.bus.write_i2c_block_data(self.address, self.LCD_DATA_REGISTER, [ord(c)])


class Programmer:
    """pymcuprog で ATtiny1616 を UPDI 経由で書き込むクラス"""

    def write_fuses(self):
        """
        fuseビットを書き込む
        Returns:
            bool: 成功時 True、失敗時 False
        """
        # pymcuprog コマンドを検出
        script = shutil.which("pymcuprog")
        if not script:
            script = os.path.join(os.path.dirname(sys.executable), "pymcuprog")
        # 存在・実行権限チェック
        if not os.path.isfile(script) or not os.access(script, os.X_OK):
            print("Error: pymcuprog command not found. Please install and ensure it's in PATH.")
            return False
        # fuseビット書き込みコマンド構築
        cmd = [
            script,
            "-d", "attiny1616",
            "-t", "uart",
            "-u", "/dev/ttyAMA0",
            "write",
            "-m", "fuses",
            "-o", "0x00",
            "-l", "0x00", "0x00", "0x01", "0xFF", "0x00", "0xF6", "0x04", "0x00", "0x00"
        ]
        # デバッグ用: 実行コマンドを表示
        print(f"Fuseビット書き込み: {' '.join(cmd)}")
        # コマンド実行 (出力をキャプチャ)
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        # 実行ログを常に出力
        if result.stdout:
            print("STDOUT:", result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr)
        if result.returncode != 0:
            print("Fuseビット書き込みエラー")
            return False
        print("Fuseビット書き込み成功")
        return True

    def write_hex(self, file_path):
        """
        Args:
            file_path (str): 書き込む .hex ファイルのパス
        Returns:
            bool: 成功時 True、失敗時 False
        """
        # まずfuseビットを書き込む
        if not self.write_fuses():
            return False
        
        # pymcuprog コマンドを検出
        script = shutil.which("pymcuprog")
        if not script:
            script = os.path.join(os.path.dirname(sys.executable), "pymcuprog")
        # 存在・実行権限チェック
        if not os.path.isfile(script) or not os.access(script, os.X_OK):
            print("Error: pymcuprog command not found. Please install and ensure it's in PATH.")
            return False
        # コマンド構築
        cmd = [script, "write", 
            "-t", "uart",
            "-u", "/dev/ttyAMA0",
            "-d", "attiny1616",
            "-f", file_path,
            "--erase",
            "--verify",
        ]
        # デバッグ用: 実行コマンドを表示
        print(f"hexファイル書き込み: {' '.join(cmd)}")
        # コマンド実行 (出力をキャプチャ)
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        # 実行ログを常に出力
        if result.stdout:
            print("STDOUT:", result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr)
        if result.returncode != 0:
            print("hexファイル書き込みエラー")
            return False
        print("hexファイル書き込み成功")
        return True


def main():
    global file_select_event
    global file_select_prev_event
    global write_button_pressed
    # GPIO 初期化
    GPIO.setmode(GPIO.BCM)

    # ハードウェアオブジェクト生成
    # RGB LED対応: 赤=GPIO22, 緑=GPIO17, 青=GPIO27
    red_led = LED(22)
    green_led = LED(17)
    blue_led = LED(27)
    buzzer = Buzzer(18)
    button = Button(23, bounce_time=0.3, pull_up=True)
    programmer = Programmer()
    # SW2ボタンのチャタリング対策を強化するため、デバウンス時間を延長
    button2 = Button(20, bounce_time=0.3, pull_up=True)  # 0.01秒→0.3秒に変更
    # SW3ボタン（前のファイルへ）
    button3 = Button(21, bounce_time=0.3, pull_up=True)
    lcd = LCD(address=0x3e, backlight_pin=6)

    # 割り込み設定: SW2押下時にフラグ設定（次のファイルへ）
    # チャタリング対策としてbouncetimeを300msに設定
    GPIO.add_event_detect(button2.pin,
                         GPIO.FALLING if button2.pull_up else GPIO.RISING,
                         callback=on_file_select,
                         bouncetime=300)  # 直接300msを指定

    # 割り込み設定: SW3押下時にフラグ設定（前のファイルへ）
    GPIO.add_event_detect(button3.pin,
                         GPIO.FALLING if button3.pull_up else GPIO.RISING,
                         callback=on_file_select_prev,
                         bouncetime=300)  # 直接300msを指定

    # 割り込み設定: SW1(書き込みボタン)押下時にフラグ設定
    # bouncetimeを長めに設定して二重検出を防止
    GPIO.add_event_detect(button.pin,
                         GPIO.RISING if not button.pull_up else GPIO.FALLING,
                         callback=on_write_button,
                         bouncetime=300)  # 300ms

    # 起動時状態: 緑 LED 点灯
    green_led.on()
    red_led.off()
    blue_led.off()

    # 初期hex選択
    base_dir = os.path.dirname(os.path.abspath(__file__))
    hex_dir = os.path.join(base_dir, "hex")
    os.makedirs(hex_dir, exist_ok=True)
    hex_files = sorted(glob.glob(os.path.join(hex_dir, "*.hex")))
    selected_idx = 0
    if hex_files:
        names = [os.path.basename(f) for f in hex_files]
        cur = names[selected_idx]
        nxt = names[(selected_idx+1) % len(names)]
        # 二行表示: 1行目に選択ファイル, 2行目に次のファイル
        lcd.display(cur[:8].ljust(8), line=0)
        lcd.display(nxt[:8].ljust(8), line=1)
    else:
        lcd.display("No HEX files", line=0)

    try:
        while True:
            # SW2とSW3の同時押しチェック（プログラム終了）
            if button2.is_active() and button3.is_active():
                print("SW2とSW3同時押し検出 - プログラムを終了します")
                lcd.clear()
                lcd.display("Exiting...", line=0)
                time.sleep(1)
                break
            
            # hexファイル選択: SW2割り込みフラグで次を表示
            if file_select_event:
                file_select_event = False
                hex_files = sorted(glob.glob(os.path.join(hex_dir, "*.hex")))
                if hex_files:
                    names = [os.path.basename(f) for f in hex_files]
                    # 次のファイル選択
                    selected_idx = (selected_idx + 1) % len(names)
                    cur = names[selected_idx]
                    nxt = names[(selected_idx + 1) % len(names)]
                    # 表示更新
                    lcd.display(cur[:8].ljust(8), line=0)
                    lcd.display(nxt[:8].ljust(8), line=1)
                    # 長いファイル名はスクロール (イベント検出で中断)
                    # ボタン操作がない限り繰り返しスクロールする
                    if len(cur) > 8:
                        # 繰り返しスクロール用の無限ループ
                        while not file_select_event and not file_select_prev_event and not write_button_pressed and not (button2.is_active() and button3.is_active()):
                            scroll_str = cur + " " * 8
                            scroll_len = len(scroll_str)
                            for i in range(scroll_len):
                                lcd.display(scroll_str[i:i+8], line=0)
                                lcd.display(nxt[:8].ljust(8), line=1)
                                # 短い間隔で割り込みチェック
                                for _ in range(6):  # 0.3秒を0.05秒×6回に分割
                                    time.sleep(0.05)
                                    if file_select_event or file_select_prev_event or write_button_pressed or (button2.is_active() and button3.is_active()):
                                        break
                                if file_select_event or file_select_prev_event or write_button_pressed or (button2.is_active() and button3.is_active()):
                                    break
                            
                            # スクロール間の待機時間を最小限にし、ほぼ連続的にスクロールする
                            # 待機時間なしで次のスクロールを開始
                            if not file_select_event and not file_select_prev_event and not write_button_pressed and not (button2.is_active() and button3.is_active()):
                                # スクロール間の待機は必要最小限にする
                                # 割り込みチェックのために最小限の待機を入れる
                                time.sleep(0.01)
                                if file_select_event or file_select_prev_event or write_button_pressed or (button2.is_active() and button3.is_active()):
                                    break
                        
                        # スクロール中にファイル選択ボタンが押された場合、フラグを再設定して次のループで確実にファイル切り替えが行われるようにする
                        if file_select_event or file_select_prev_event:
                            continue
            
            # hexファイル選択: SW3割り込みフラグで前を表示
            if file_select_prev_event:
                file_select_prev_event = False
                hex_files = sorted(glob.glob(os.path.join(hex_dir, "*.hex")))
                if hex_files:
                    names = [os.path.basename(f) for f in hex_files]
                    # 前のファイル選択
                    selected_idx = (selected_idx - 1) % len(names)
                    cur = names[selected_idx]
                    nxt = names[(selected_idx + 1) % len(names)]
                    # 表示更新
                    lcd.display(cur[:8].ljust(8), line=0)
                    lcd.display(nxt[:8].ljust(8), line=1)
                    # 長いファイル名はスクロール (イベント検出で中断)
                    # ボタン操作がない限り繰り返しスクロールする
                    if len(cur) > 8:
                        # 繰り返しスクロール用の無限ループ
                        while not file_select_event and not file_select_prev_event and not write_button_pressed and not (button2.is_active() and button3.is_active()):
                            scroll_str = cur + " " * 8
                            scroll_len = len(scroll_str)
                            for i in range(scroll_len):
                                lcd.display(scroll_str[i:i+8], line=0)
                                lcd.display(nxt[:8].ljust(8), line=1)
                                # 短い間隔で割り込みチェック
                                for _ in range(6):  # 0.3秒を0.05秒×6回に分割
                                    time.sleep(0.05)
                                    if file_select_event or file_select_prev_event or write_button_pressed or (button2.is_active() and button3.is_active()):
                                        break
                                if file_select_event or file_select_prev_event or write_button_pressed or (button2.is_active() and button3.is_active()):
                                    break
                            
                            # スクロール間の待機時間を最小限にし、ほぼ連続的にスクロールする
                            # 待機時間なしで次のスクロールを開始
                            if not file_select_event and not file_select_prev_event and not write_button_pressed and not (button2.is_active() and button3.is_active()):
                                # スクロール間の待機は必要最小限にする
                                # 割り込みチェックのために最小限の待機を入れる
                                time.sleep(0.01)
                                if file_select_event or file_select_prev_event or write_button_pressed or (button2.is_active() and button3.is_active()):
                                    break
                        
                        # スクロール中にファイル選択ボタンが押された場合、フラグを再設定して次のループで確実にファイル切り替えが行われるようにする
                        if file_select_event or file_select_prev_event:
                            continue
            
            # 短い間隔で割り込みチェック
            time.sleep(0.05)

            if write_button_pressed:
                write_button_pressed = False
                # 書込み開始
                green_led.off()
                # 黄色LED表示（赤と緑を同時点灯）
                red_led.on()
                green_led.on()
                lcd.display("Writing", line=1)

                # 選択中のhexファイルを書込
                if not hex_files:
                    lcd.display("No HEX", line=1)
                else:
                    target = hex_files[selected_idx]
                    # 書込み実行と結果判定
                    success = False
                    try:
                        success = programmer.write_hex(target)
                    except Exception as e:
                        print("プログラミング例外:", e)
                    if success:
                        buzzer.play_success()
                        lcd.display("Finish!!", line=1)
                        red_led.off()
                        green_led.on()
                        # 短い間隔で割り込みチェック
                        for _ in range(20):  # 1秒を0.05秒×20回に分割
                            time.sleep(0.05)
                            if file_select_event:
                                file_select_event = False
                                break
                        # Finish後に元のファイル名表示に戻す
                        lcd.display(cur[:8].ljust(8), line=0)
                        lcd.display(nxt[:8].ljust(8), line=1)
                    else:
                        buzzer.play_error()
                        lcd.display("Error!!", line=1)
                        red_led.on()
                        # エラー時はスイッチ押下まで待機
                        while True:
                            # SW2とSW3同時押しでプログラム終了
                            if button2.is_active() and button3.is_active():
                                print("SW2とSW3同時押し検出（エラー中） - プログラムを終了します")
                                lcd.clear()
                                lcd.display("Exiting...", line=0)
                                time.sleep(1)
                                raise KeyboardInterrupt
                            if write_button_pressed or file_select_event or file_select_prev_event:
                                red_led.off()
                                if file_select_event:
                                    file_select_event = False
                                if file_select_prev_event:
                                    file_select_prev_event = False
                                if write_button_pressed:
                                    write_button_pressed = False
                                # ボタンが離されるのを待つ（短い間隔でチェック）
                                while button.is_active():
                                    time.sleep(0.05)
                                break
                            time.sleep(0.05)
                # 書込み後処理
                # 黄色LED消灯（赤と緑を消灯）
                red_led.off()
                green_led.off()
                # 短い間隔で割り込みチェック
                for _ in range(20):  # 1秒を0.05秒×20回に分割
                    time.sleep(0.05)
                    if file_select_event:
                        break
    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()


if __name__ == "__main__":
    main()
