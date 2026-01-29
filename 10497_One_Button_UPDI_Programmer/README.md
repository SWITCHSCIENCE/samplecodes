# One Button UPDI Programmer

Raspberry Piを使用して、ボタンひとつでAVRマイコン（tinyAVR 0/1/2シリーズ等）へのUPDI書き込みを行うためのサンプルコードおよびセットアップ手順です。

動作確認にはRaspberry Pi Zero 2 W(Raspberry Pi OS 64-bit)を使用し、サンプルプログラム(One_Button_UPDI_Programmer_Demo.hex)ではATtiny1616をターゲットマイコンとしています。

One_Button_UPDI_Programmer_Demo.hexはPIN_PA4に繋がったNeopixelを点灯するプログラムとなっています。

---

## 書き込み用サンプルプログラム(main.py)について

書き込みにはMICROCHIP社のpymcuprogを使用しています。
pymcuprogの詳細は下記リポジトリをご参照ください。
- <p><a href="https://github.com/microchip-pic-avr-tools/pymcuprog" target="_blank">pymcuprog (github)</a></p>

書き込みに使用するファイルはhexディレクトリ内に配置してください。
複数ファイル配置可能です。

ATtiny1616以外のマイコンへ書き込む場合は203行目のclass Programmer:内コマンド構築2か所を適宜書き換える必要があります。

LEDの色は以下の状態を表しています。
- 緑：書き込み待機中、書き込み完了
- 黄：書き込み中
- 赤：書き込みエラー

合わせて、書き込み完了時と書き込みエラー時にはそれぞれの状態を表すビープ音が鳴ります。

またmain.pyで指定しているATtiny1616のヒューズビットは以下の設定になっています。

## ATtiny1616 Fuse Settings (0x001280 - 0x001288)

| Address | Register | Value | Description |
|:---|:---|:---:|:---|
| 0x00 | **WDTCFG** | `00` | WDT Disabled |
| 0x01 | **BODCFG** | `00` | BOD Disabled |
| 0x02 | **OSCCFG** | `02` | **20MHz** Internal Osc |
| 0x03 | (Reserved) | `FF` | - |
| 0x04 | **TCD0CFG** | `00` | Default |
| 0x05 | **SYSCFG0** | **`F6`** | **UPDI Pin Enabled** / EEPROM Retained |
| 0x06 | **SYSCFG1** | `04` | SUT: 4ms |
| 0x07 | **APPEND** | `00` | No Append Section |
| 0x08 | **BOOTEND** | `00` | No Boot Section |

---

### ⚠️ SYSCFG0 Critical Note
`SYSCFG0` の値 **`F6`** は、PA0ピンを **UPDIモード** として動作させます。
もし `RSTPINCFG` を変更して **GPIO/RESETモード** にすると、本プログラマーでは書き込み不能（要12Vプログラマ）になるため注意してください。

---

## セットアップ手順

### 1. システムの更新とインターフェース設定
OSを最新の状態にし、`raspi-config`からI2Cとシリアルポートを有効化します。

```bash
sudo apt update
sudo apt upgrade
sudo raspi-config
```

#### **raspi-config内での操作手順:**
1. **3 Interface Options** を選択
2. **I5 I2C** -> **Yes** (有効化)
3. **I6 Serial Port** を選択
   - `Would you like a login shell to be accessible over serial?` -> **No**
   - `Would you like the serial port hardware to be enabled?` -> **Yes**
4. **Finish** を選択し、再起動します。

---

### 2. UARTおよびI2Cの詳細設定
GPIOでUART0を使うための設定と、I2Cの通信速度（ボーレート）設定を行います。

> [!IMPORTANT]
> Raspberry Pi OS（Bookworm以降）では、設定ファイルの場所が `/boot/firmware/config.txt` になっています。

```bash
sudo nano /boot/firmware/config.txt
```

ファイルの末尾に以下の内容を追記してください：

```text
# Use miniUART to connect to Bluetooth module
dtoverlay=miniuart-bt
core_freq=250

# Change I2C baudrate
dtparam=i2c_arm_baudrate=50000
```

保存して再起動します。
```bash
sudo reboot
```

---

### 3. Python環境の構築 (`uv` の使用)
高速なパッケージマネージャ `uv` を使用して環境を構築します。

#### uvのインストールとパス設定
```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env
```

#### リポジトリの準備
```bash

git clone --filter=blob:none --sparse https://github.com/SWITCHSCIENCE/samplecodes.git
cd samplecodes
git sparse-checkout set 10497_One_Button_UPDI_Programmer
cd 10497_One_Button_UPDI_Programmer
```

#### システムPythonを利用したプロジェクト初期化
```bash
uv init --package uv-sys-python --no-managed-python
nano pyproject.toml
```

`pyproject.toml` に以下を追記します：
```toml
[tool.uv]
python-preference = "only-system"
```

#### 仮想環境の作成とライブラリのインストール
```bash
uv venv --system-site-packages
source .venv/bin/activate
uv pip install pymcuprog
```

---

#### HEXファイルの配置仕様
書き込みを行うHEXファイルは、事前に`hex`ディレクトリ内へ格納してください。

複数のHEXファイルを配置して実行することが可能ですが、書き込みプログラムの仕様上、ヒューズビット設定は一括で適用されます。したがって、**配置されたすべてのファイルに対して一律のヒューズビットが書き込まれます。**

## 使い方

仮想環境が有効な状態で、以下のコマンドを実行することでプログラムが起動します。

```bash
python main.py
```

LCD左側のスイッチで書き込みたいhexファイルを選択し、その後右側にある緑のスイッチを押すことで書き込みが開始します。

---

## 自動起動設定 (systemd)

Raspberry Piの起動時に自動的にプログラムを実行するように設定すると便利です。

### 1. サービスファイルの作成
```bash
sudo nano /etc/systemd/system/one_button_updi_programmer.service
```

### 2. サービスファイルの内容
以下の内容を貼り付けて保存します。
※ユーザー名が `pi` 以外の場合は、適宜書き換えてください。

```one_button_updi_programmer.service
[Unit]
Description=One Button UPDI Programmer Service
After=network.target

[Service]
# プログラムがあるディレクトリを作業ディレクトリに指定
WorkingDirectory=/home/pi/samplecodes/10497_One_Button_UPDI_Programmer
# 仮想環境内のPythonパスを直接指定して実行
ExecStart=/home/pi/samplecodes/10497_One_Button_UPDI_Programmer/.venv/bin/python main.py
# 実行ユーザーを指定
User=pi
# クラッシュした時に自動再起動する設定
Restart=always

[Install]
WantedBy=multi-user.target
```

### 3. サービスの有効化と起動
```bash
# 設定の反映
sudo systemctl daemon-reload

# 自動起動の設定
sudo systemctl enable one_button_updi_programmer.service

# 今すぐ起動（テスト）
sudo systemctl start one_button_updi_programmer.service
```

### 電源操作の設定

### 1. GPIOボタンによるシャットダウン設定
GPIO 26番ピンに接続された物理ボタンを、3秒長押しでシャットダウンするように設定します。

```bash
sudo nano /boot/firmware/config.txt
```

ファイルの最終行に以下を追記します：
```text
dtoverlay=gpio-shutdown,gpio_pin=26,active_low=1,gpio_pull=up,debounce=3000
```

### 2. UIポップアップの無効化（1回で電源オフにする）
OS標準の電源メニュー表示（gtk-nop）を無効化し、1回の操作でシャットダウンできるようにします。

```bash
sudo mv /etc/xdg/autostart/pwrkey.desktop /etc/xdg/autostart/pwrkey.desktop.disabled
sudo reboot
```

### 状態確認とログ
```bash
# 動作ステータスの確認
sudo systemctl status one_button_updi_programmer.service

# プログラムの出力（ログ）をリアルタイムで確認
journalctl -u one_button_updi_programmer.service -f
```

---
