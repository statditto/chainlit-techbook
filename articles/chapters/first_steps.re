= Chainlitはじめの一歩

//lead{
本章では、Chainlitを動かすまでの環境構築を丁寧に解説します。パッケージ管理ツールとして近年急速に普及している@<b>{uv}を使い、Chainlitをインストールするところから、ブラウザでチャット画面が開くまでを一緒に進めていきましょう。
//}

//pagebreak

== uvとは

@<b>{uv}は、Rustで書かれた高速なPythonパッケージ・プロジェクト管理ツールです@<fn>{uv}。従来の@<code>{pip}や@<code>{venv}、@<code>{pyenv}を組み合わせて行っていた作業を、uvひとつでまかなえるのが特徴です。インストールが速く、仮想環境の作成からパッケージの追加まで一貫したコマンドで扱えるため、本書では環境構築にuvを採用します。

//footnote[uv][https://docs.astral.sh/uv/]

=== uvのインストール

まだuvをインストールしていない場合は、以下のコマンドを実行してください。

//cmd{
# macOS / Linux
$ curl -LsSf https://astral.sh/uv/install.sh | sh

# Windows (PowerShell)
$ powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
//}

インストール後、シェルを再起動するかパスを再読み込みし、バージョンが表示されることを確認します。2024年6月時点の最新バージョンは@<b>{0.10.10}です。

//cmd{
$ uv --version
uv 0.10.10 (8c730aaad 2026-03-13)
//}

== プロジェクトの作成

uvでPythonプロジェクトを新規作成します。@<code>{--python}オプションでPythonバージョンを指定すると、@<code>{pyproject.toml}の@<code>{requires-python}が自動的に設定されます。

//cmd{
$ uv init chainlit-first --python 3.13.4
$ cd chainlit-first
//}

=== Pythonバージョンの固定

続いて@<code>{uv python pin}を実行します。プロジェクトルートに@<code>{.python-version}ファイルが作成され、このディレクトリ内では常に指定したバージョンが使われるようになります。

//cmd{
$ uv python pin 3.13.4
Pinned `.python-version` to `3.13.4`
//}

//note[Pythonバージョンが手元にない場合]{
指定したバージョンがまだインストールされていなければ、@<code>{uv python install 3.13.4}で自動的にダウンロード・インストールできます。uvがPythonのインストールまで一括管理してくれるのも便利なポイントです。
//}

=== Chainlitのインストール

次に、Chainlitをバージョン@<b>{2.9.6}に固定して追加します。バージョンを固定しておくことで、将来的なアップデートによる予期しない動作変更を防ぐことができます。

//cmd{
$ uv add "chainlit==2.9.6"
//}

//note[バージョン固定の重要性]{
Chainlitは活発に開発されており、マイナーバージョンアップでもAPIの挙動が変わることがあります。本書のサンプルコードはChainlit 2.9.6で動作確認を行っているため、再現性を確保するために@<code>{==2.9.6}でピン留めすることをおすすめします。
//}

これで@<code>{pyproject.toml}にChainlitの依存関係が追記され、仮想環境（@<code>{.venv}）へのインストールも自動的に完了します。

== Chainlitを動かしてみる

環境が整ったら、Chainlitに用意されている@<code>{chainlit hello}コマンドを実行してみましょう。このコマンドはChainlit組み込みのデモアプリをそのまま起動してくれます。設定ファイルの初期生成も兼ねているため、まず最初に一度実行しておくとよいでしょう。

//cmd{
$ uv run chainlit hello
//}

コマンドを実行すると、ターミナルに以下のようなメッセージが表示され、自動的にブラウザが開きます。

//cmd{
2026-xx-xx xx:xx:xx - Your app is available at http://localhost:8000
//}

ブラウザでチャット画面が表示されれば成功です🎉 画面上のチャット入力欄にメッセージを送ると、ボットから返信が届くはずです。Ctrl+Cでサーバーを停止できます。

== 生成されるディレクトリ構造

@<code>{uv init}と@<code>{uv add}、@<code>{chainlit hello}を実行した後、プロジェクト内は以下のような構造になっています。

//cmd{
chainlit-first/
├── .chainlit/
│   ├── config.toml
│   └── translations/
│       ├── en-US.json
│       ├── ja.json
│       └── （その他の言語ファイル）
├── .files/              ← chainlit helloで生成（アップロードファイル保存用）
├── .python-version      ← uv python pinで生成
├── .venv/               ← uv addで生成（仮想環境）
├── chainlit.md          ← chainlit helloで生成
├── main.py              ← uv initで生成
├── pyproject.toml       ← uv initで生成
├── README.md            ← uv initで生成
└── uv.lock              ← uv addで生成（依存関係のロックファイル）
//}

それぞれの役割を順に見ていきましょう。

=== main.py

@<code>{uv init}によって生成されるエントリポイントです。初期状態では単純なHello Worldのコードが入っていますが、これを書き換えてChainlitアプリを作っていくか、別途@<code>{app.py}などを作成して使います。

=== chainlit.md

アプリ起動時にチャット画面のウェルカムページとして表示されるMarkdownファイルです。プロジェクトの説明や使い方をここに記載しておくと、ユーザーへの案内として機能します。

=== .chainlit/config.toml

Chainlitアプリの動作を細かく制御する設定ファイルです。主なセクションと設定項目を以下に示します。

//table[config][config.tomlの主な設定]{
セクション	設定項目	説明
--------------------------
[project]	session_timeout	接続切断後にセッションを保持する秒数（デフォルト: 3600）
[project]	cache	LangChainなどのサードパーティキャッシュを有効にするか
[features]	unsafe_allow_html	メッセージ内のHTMLレンダリングを許可するか（セキュリティ注意）
[features]	latex	数式（LaTeX）のレンダリングを有効にするか
[UI]	name	チャット画面に表示されるアシスタントの名前
[UI]	cot	思考過程（Chain of Thought）の表示モード
//}

最初のうちはデフォルト設定のまま進めて問題ありません。カスタマイズしたくなったときに、このファイルを編集するようにしましょう。

=== .chainlit/translations/

Chainlit UIの多言語対応用JSONファイルが格納されています。@<code>{ja.json}を編集することで、UI上のテキストを日本語にカスタマイズできます。@<code>{config.toml}の@<code>{[UI]}セクションで@<code>{language = "ja"}を設定すると、日本語表示がデフォルトになります。

== 自分のアプリを動かしてみる

@<code>{chainlit hello}は組み込みのデモですが、自分でコードを書いて動かしてみましょう。@<code>{app.py}という名前で以下の内容を作成します。

//list[app][app.py]{
import chainlit as cl

@cl.on_message
async def main(message: cl.Message):
    await cl.Message(
        content=f"「{message.content}」と言いましたね！",
    ).send()
//}

以下のコマンドで起動します。

//cmd{
$ uv run chainlit run app.py -w
//}

@<code>{-w}オプションを付けると、ファイルを変更したときに自動でリロードされるウォッチモードで起動します。開発中はこのオプションを活用すると便利です。

//note[uvを使ったコマンド実行]{
@<code>{uv run}を先頭に付けることで、プロジェクトの仮想環境に入らずともその環境のPythonやコマンドを使って実行できます。@<code>{source .venv/bin/activate}などの仮想環境アクティベーション操作が不要になるため、本書では@<code>{uv run}を使って実行する形式に統一します。
//}

== まとめ

本章では、uvを使ったプロジェクト作成からChainlit 2.9.6のインストール、@<code>{chainlit hello}による動作確認、そして生成されるファイル群の役割までを解説しました。

 * @<code>{uv init} でプロジェクトを作成し、@<code>{uv add "chainlit==2.9.6"} でバージョンを固定してインストール
 * @<code>{chainlit hello} でデモを起動し、設定ファイルを初期生成
 * @<code>{.chainlit/config.toml} でアプリの動作を設定
 * @<code>{chainlit.md} でウェルカム画面をカスタマイズ

なお、本章ではChainlitの起動確認までを扱っており、OpenAIやAnthropicなどのLLM APIとの接続（APIキーの設定など）については取り上げていません。各LLMへの接続方法は次章以降のサンプルコードを参考にしてください。

次章からは、実際にChainlitを使ったさまざまなアプリの実装例を見ていきます。ぜひ各章のコードをcloneして、実際に動かしながら読み進めてください！
