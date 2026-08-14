# Telemetry System

C11 と POSIX Socket API を用いた、装置テレメトリ監視システムです。Device Simulator が Telemetry Server へデータを送信し、将来的には解析・SQLite への保存・CLI による監視を行います。

詳細な仕様は [SPEC.md](SPEC.md) を参照してください。

## Build

Linux または WSL2 で、gcc（または C11 をサポートする C コンパイラ）と GNU Make を用意してください。

```bash
make
```

ビルド成果物は `build/` に作成されます。

```text
build/telemetry_server
build/device_simulator
```

## Clean

```bash
make clean
```

## Commit Message Rules

コミットメッセージは次の形式にします。

```text
<type>: <summary>
```

- `type` は以下から選びます。
  - `feat`: 機能追加
  - `fix`: バグ修正
  - `docs`: ドキュメント変更
  - `test`: テスト追加・修正
  - `refactor`: 振る舞いを変えないコード整理
  - `build`: Makefile、ビルド設定、依存関係の変更
  - `ci`: GitHub Actions など CI 設定の変更
  - `chore`: 上記以外の保守作業
- `summary` は短く、命令形の英語で記述します。
- 末尾にピリオドは付けません。
- 必要な補足は、タイトルの後を空行で区切って本文に記述します。

例:

```text
feat: add TCP server socket initialization
fix: close client socket on disconnect
test: add protocol parser invalid input tests
build: add SQLite link options
docs: add local development instructions
```
