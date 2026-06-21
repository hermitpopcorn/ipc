# メッセージキュー IPC デモ

メッセージキュー (`mq_open`, `mq_send`, `mq_receive`) は、UNIX 系 OS における IPC 方式の一種。
UDS (Unix Domain Socket) とは異なり、MQ は非同期通信になっており、送信時に受信側が生きていなくてもメッセージが届く。

このデモは POSIX 版。SystemV のもあるが、性能的に POSIX のが推奨されているらしい。
違いについては別の機会で。

## 動作原理

### メッセージキューの特徴

POSIX メッセージキューは、以下の特性を持つ:

1. リアルタイム通信と違って、メッセージが格納されるキューを使用
2. 受信側が起動していなくても、メッセージはキューに保持される
3. 各メッセージに優先度を設定可能
4. 仲介者（ハブ）が不要

### API 概要

- `mq_open()` - メッセージキューをオープン（存在しなければ作成）
- `mq_send()` - メッセージをキューに送信
- `mq_receive()` - キューからメッセージを受信（通常はブロッキング）
- `mq_close()` - メッセージキューをクローズ
- `mq_unlink()` - メッセージキューを削除
- `mq_getattr()` / `mq_setattr()` - キューの属性を取得/設定

## このデモについて

構成は以下になっている:

- **producer.cpp**
  - メッセージキューを開く
  - メッセージをキューに送信
  - 優先度を指定可能
  - 実行後に終了（メッセージはキューに残る）

- **consumer.cpp**
  - メッセージキューを開く
  - キューからメッセージを受信
  - 優先度が高いものから順に受信
  - Ctrl+C で終了

## 使用方法

### 基本的な使用方法

```bash
# ビルド
make

# ターミナル1: コンシューマーを起動（メッセージ受信待機）
./consumer

# ターミナル2: プロデューサーでメッセージ送信
./producer 1 "Hello from Producer 1"
./producer 2 "Hello from Producer 2"
```

### メッセージ永続性のデモ

```bash
# ステップ1: コンシューマーを起動していない状態でメッセージを送信
./producer 1 "Message sent before consumer started"
./producer 2 "Another message before consumer started"

# ステップ2: 後からコンシューマーを起動
./consumer
```

## リソースクリーンアップ

POSIX メッセージキューはシステムリソースとして保持されるため、手動削除が必要な場合がある:

```bash
rm /dev/mqueue/ipc_demo_queue
```
