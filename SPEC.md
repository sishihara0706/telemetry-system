# C言語 装置テレメトリ監視システム 仕様書 v0.1

## 1. 目的

Linux / WSL上で動作する、C言語製の簡易的な装置監視システムを構築する。

単一プログラムではなく複数プロセスから構成し、以下を実践する。

* Cによる実用的なアプリケーション開発
* Makefileによるビルド
* ソケット通信
* プロセス間の責務分離
* ログ処理
* 設定ファイル
* 単体テスト / 結合テスト
* Dockerによる実行環境の固定
* Docker Composeによる複数サービス管理
* GitHub Actionsによる自動ビルド / テスト

---

# 2. システム概要

仮想的な「産業装置」が複数存在し、各装置から状態データが送信される。

中央のTelemetry Serverがデータを受信し、保存・監視する。

```text
Device Simulator A ─┐
                    │
Device Simulator B ─┼── TCP ──> Telemetry Server
                    │                 │
Device Simulator C ─┘                 │
                                      ├── ログ
                                      │
                                      └── SQLite
                                             │
                                             ↓
                                      Monitor Client
```

構成するプログラムは3種類とする。

### device_simulator

産業装置を模擬するプログラム。

一定周期で状態情報をTelemetry Serverへ送信する。

### telemetry_server

複数のDevice SimulatorからTCP接続を受け付ける。

受信データを解析し、SQLiteデータベースへ保存する。

### monitor_client

Telemetry ServerまたはDBの状態を確認するCLIアプリケーション。

---

# 3. Device Simulator

## 3.1 起動例

```bash
./device_simulator \
    --device-id DEV001 \
    --host 127.0.0.1 \
    --port 9000
```

## 3.2 生成データ

以下のデータを1秒周期で生成する。

* device_id
* timestamp
* temperature
* pressure
* motor_speed
* status

例：

```text
DEV001,1723500000,42.3,101.2,2300,NORMAL
```

## 3.3 状態

statusは以下のいずれかとする。

```text
NORMAL
WARNING
ERROR
```

温度などの値によって自動的に決定する。

例：

```text
temperature < 70
→ NORMAL

70 <= temperature < 85
→ WARNING

temperature >= 85
→ ERROR
```

---

# 4. Telemetry Server

## 4.1 起動

```bash
./telemetry_server --port 9000
```

## 4.2 主な機能

Telemetry Serverは以下を担当する。

1. TCP Listen
2. クライアント接続受付
3. データ受信
4. プロトコル解析
5. データ検証
6. SQLiteへの保存
7. ログ出力

複数Deviceから同時接続できること。

---

# 5. 通信プロトコル

初期版では独自のテキストプロトコルを使用する。

形式：

```text
device_id,timestamp,temperature,pressure,motor_speed,status\n
```

例：

```text
DEV001,1723500000,42.3,101.2,2300,NORMAL
```

1行を1メッセージとする。

TCPでは1回の`recv()`と1メッセージが一致するとは限らないため、受信バッファ処理を実装する。

これを重要な実装課題の1つとする。

---

# 6. データベース

SQLiteを使用する。

テーブル：

```sql
CREATE TABLE telemetry (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT NOT NULL,
    timestamp INTEGER NOT NULL,
    temperature REAL NOT NULL,
    pressure REAL NOT NULL,
    motor_speed INTEGER NOT NULL,
    status TEXT NOT NULL
);
```

---

# 7. Monitor Client

CLIベースとする。

初期バージョンではGUIを使用しない。

起動例：

```bash
./monitor_client --db telemetry.db
```

表示例：

```text
DEVICE     TEMP     PRESSURE     RPM      STATUS
-------------------------------------------------
DEV001     42.3     101.2        2300     NORMAL
DEV002     76.1     101.0        2100     WARNING
DEV003     88.4     100.8        1800     ERROR
```

オプション：

```bash
./monitor_client --device DEV001
```

とすると特定Deviceのみ表示する。

---

# 8. ログ

Telemetry Serverは以下のログレベルを持つ。

```text
DEBUG
INFO
WARN
ERROR
```

例：

```text
2026-08-13 13:20:01 INFO  server started port=9000
2026-08-13 13:20:05 INFO  device connected id=DEV001
2026-08-13 13:21:15 WARN  high temperature device=DEV001 temp=76.2
2026-08-13 13:23:01 ERROR invalid packet
```

ログは

```text
stdout
```

へ出力する。

これは後にDocker化した際、

```bash
docker logs
```

で扱いやすくするためである。

---

# 9. 設定

初期版ではコマンドライン引数を使用する。

後の改造で設定ファイルを導入できる構造にしておく。

例：

```text
server.conf
```

```ini
port=9000
database=/data/telemetry.db
log_level=INFO
```

---

# 10. ディレクトリ構成

```text
telemetry-system/
│
├── Makefile
├── README.md
│
├── include/
│   ├── protocol.h
│   ├── logger.h
│   └── telemetry.h
│
├── src/
│   ├── common/
│   │   ├── protocol.c
│   │   └── logger.c
│   │
│   ├── server/
│   │   ├── main.c
│   │   ├── server.c
│   │   └── database.c
│   │
│   ├── simulator/
│   │   ├── main.c
│   │   └── simulator.c
│   │
│   └── monitor/
│       ├── main.c
│       └── monitor.c
│
├── tests/
│   ├── test_protocol.c
│   ├── test_logger.c
│   └── test_database.c
│
├── scripts/
│
└── build/
```

---

# 11. Makefile

以下をサポートする。

```bash
make
```

全プログラムをビルド。

```bash
make server
```

Telemetry Serverのみビルド。

```bash
make simulator
```

Device Simulatorのみビルド。

```bash
make monitor
```

Monitor Clientのみビルド。

```bash
make test
```

テスト実行。

```bash
make clean
```

生成物削除。

ビルドには以下を使用する。

```text
gcc
-Wall
-Wextra
-Werror
```

可能であれば、

```text
-pedantic
```

も使用する。

---

# 12. テスト

テストは3段階に分ける。

## Unit Test

対象：

* パケット解析
* データ検証
* status判定
* ログ関連
* DB操作

例：

```text
"DEV001,1000,42.3,101.2,2300,NORMAL"
```

を解析して正しい構造体になること。

---

## Integration Test

実際に

```text
Simulator
    ↓ TCP
Server
    ↓
SQLite
```

を動作させる。

Simulatorから送ったデータがDBへ保存されることを確認する。

---

## Error Test

以下も確認する。

* 不正パケット
* TCP切断
* Server停止
* DB書き込み失敗
* 長すぎるメッセージ
* 不正な数値

---

# 13. Docker化

最初の実装・テスト完了後にDocker化する。

以下のDocker Imageを作る。

```text
telemetry-server
device-simulator
monitor-client
```

Serverコンテナ：

```text
telemetry-server
```

Simulatorコンテナ：

```text
device-simulator
```

---

# 14. Docker Compose

Composeでは以下を起動する。

```text
telemetry-server
device-1
device-2
device-3
```

構成：

```text
device-1 ─┐
device-2 ─┼── Docker Network ── telemetry-server
device-3 ─┘
```

これにより、

```bash
docker compose up
```

だけで複数装置のシミュレーション環境を構築可能にする。

これは今回Dockerを使う大きなメリットとなる。

---

# 15. Docker Volume

SQLite DBをVolumeへ保存する。

```text
telemetry-data
```

これによりServerコンテナを削除してもデータを保持する。

---

# 16. GitHub Actions

Pull Request / Push時に以下を実行する。

```text
Checkout
   ↓
Build
   ↓
Unit Test
   ↓
Integration Test
```

さらに可能であれば、

```text
gcc warnings
clang-format
cppcheck
AddressSanitizer
UndefinedBehaviorSanitizer
```

も追加する。

CIのイメージ：

```text
GitHub Push
     │
     ↓
GitHub Actions
     │
     ├── Build
     ├── Unit Test
     ├── Integration Test
     ├── Static Analysis
     └── Sanitizer
```

---

# 17. 初期バージョンの完成条件

以下が成立したらVersion 1完成とする。

* [ ] Makefileで全プログラムをビルドできる
* [ ] Telemetry ServerがTCP Listenできる
* [ ] SimulatorがServerへ接続できる
* [ ] Simulatorが1秒周期でデータ送信できる
* [ ] Serverが複数Simulatorを処理できる
* [ ] データをSQLiteへ保存できる
* [ ] Monitorから最新状態を確認できる
* [ ] Unit Testが動作する
* [ ] Integration Testが動作する
* [ ] Docker Imageを作成できる
* [ ] Docker Composeで全サービスを起動できる
* [ ] GitHub ActionsでBuild/Testできる

---

# 18. Version 2 改造仕様

Version 1完成後に、仕様変更を入れる。

テーマ：

**Device SimulatorからServerへのHeartbeat機能追加**

各Deviceは通常のTelemetryとは別にHeartbeatを送信する。

```text
HEARTBEAT,DEV001,1723500000
```

ServerはDeviceごとの最終Heartbeat時刻を管理する。

一定時間Heartbeatが届かない場合、

```text
DEVICE LOST
```

と判定する。

例：

```text
2026-08-13 14:00:01 ERROR device timeout device=DEV001
```

Monitorにも、

```text
DEVICE     TEMP     STATUS      CONNECTION
------------------------------------------------
DEV001     42.3     NORMAL      ONLINE
DEV002     55.2     NORMAL      OFFLINE
```

を追加する。

この改造では、

* プロトコル変更
* Server変更
* DB変更
* Monitor変更
* Unit Test変更
* Integration Test変更
* CIによるリグレッション確認

を経験する。

つまり、「既存システムへ仕様変更を入れる」という実務に近い開発を行う。

