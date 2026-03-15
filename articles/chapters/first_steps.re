= Chainlitはじめの一歩

//lead{
本章では、Chainlitを動かすまでの環境構築を丁寧に解説します。パッケージ管理ツールとして近年急速に普及しているuvを使い、Chainlitのインストールから、最初のアプリケーションを起動するまで、一つずつ一緒に進めていきましょう！
//}

//pagebreak

== uvとは

@<b>{uv}は、Rustで書かれた高速なPythonパッケージ・プロジェクト管理ツールです@<fn>{uv}。従来の@<code>{pip}や@<code>{venv}、@<code>{pyenv}を組み合わせて行っていた作業を、uvひとつでまかなえるのが特徴です。インストールが速く、仮想環境の作成からパッケージの追加まで一貫したコマンドで扱えるため、本書では環境構築にuvを採用します。

//footnote[uv][https://docs.astral.sh/uv/]

=== uvのインストール

uvをインストールしていない場合は、公式のガイド（Installing uv (@<href>{https://docs.astral.sh/uv/getting-started/installation/})）に従ってインストールを進めましょう。本書の執筆時点では、次のコマンドを実行してインストールできます。

//cmd{
# macOS / Linux
$ curl -LsSf https://astral.sh/uv/install.sh | sh

# Windows (PowerShell)
$ powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
//}

インストール後、シェルを再起動するかパスを再読み込みし、バージョンが表示されることを確認します。2026年3月時点の最新バージョンは@<b>{0.10.10}です。

//cmd{
$ uv --version
uv 0.10.10 (8c730aaad 2026-03-13)
//}

== プロジェクトの作成

uvでPythonプロジェクトを新規作成します。@<code>{--python}オプションでPythonバージョンを指定すると、@<code>{pyproject.toml}の@<code>{requires-python}と@<code>{.python-version}ファイルが自動的に生成されます。

//cmd{
$ uv init chainlit-first --python 3.13.4
$ cd chainlit-first
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

コマンドを実行すると、ターミナルに次のようなメッセージが表示され、自動的にブラウザが開きます。

//cmd{
2026-xx-xx xx:xx:xx - Your app is available at http://localhost:8000
//}

@<img>{ditto-first}のように、ブラウザでチャット画面が表示されれば成功です！画面上のチャット入力欄にメッセージを送ると、ボットから返信が届くはずです。

//image[ditto-first][デモアプリのチャット画面][scale=1.0]{
//}

== 生成されるディレクトリ構造

@<code>{uv init}と@<code>{uv add}、@<code>{chainlit hello}を実行すると、プロジェクト内は次のような構造になります。

//cmd{
chainlit-first/
├── .chainlit/
│   ├── config.toml
│   └── translations/
│       ├── en-US.json
│       ├── ja.json
│       └── （その他の言語ファイル）
├── .python-version      ← uv init --pythonで生成
├── .venv/               ← uv addで生成（仮想環境）
├── chainlit.md          ← chainlit helloで生成
├── main.py              ← uv initで生成
├── pyproject.toml       ← uv initで生成
├── README.md            ← uv initで生成
└── uv.lock              ← uv addで生成（依存関係のロックファイル）
//}

それぞれの役割を順に見ていきましょう。

=== main.py

@<code>{uv init}によって生成されるエントリポイントです。初期状態では単純なHello Worldのコードが入っています。Chainlitアプリは別途@<code>{app.py}として作成するため、このファイルはそのまま残しておいて問題ありません。

=== chainlit.md

チャット画面のウェルカムページとして表示されるMarkdownファイルです。プロジェクトの説明や使い方をここに記載しておくと、ユーザーへの案内として機能します。

=== .chainlit/config.toml

Chainlitアプリの動作を細かく制御する設定ファイルです。主なセクションと設定項目を次に示します。

//table[config][config.tomlの主な設定]{
セクション	設定項目	説明
--------------------------
[project]	session_timeout	接続切断後にセッションを保持する秒数
[project]	cache	LangChainなどのサードパーティキャッシュを有効にするか
[features]	unsafe_allow_html	メッセージ内のHTMLレンダリングを許可するか
[features]	latex	数式（LaTeX）のレンダリングを有効にするか
[UI]	name	チャット画面に表示されるアシスタントの名前
[UI]	cot	思考過程（Chain of Thought）の表示モード
//}

最初のうちはデフォルト設定のまま進めて問題ありません。カスタマイズしたくなったときに、このファイルを編集するようにしましょう。詳しい設定項目は、公式ドキュメント(@<href>{https://docs.chainlit.io/backend/config/overview})を参照してください。

=== .chainlit/translations/

Chainlit UIの多言語対応用JSONファイルが格納されています。@<code>{ja.json}を編集することで、UI上のテキストを日本語にカスタマイズできます。@<code>{config.toml}の@<code>{[UI]}セクションで@<code>{language = "ja"}を設定すると、デフォルトで日本語表示に変更できます。

== 自分のアプリを動かしてみる

@<code>{chainlit hello}とは別に、少しだけ自分でもコードを書いてみましょう！@<code>{uv init}で生成された@<code>{main.py}はChainlit用のコードではないため、新たに@<code>{app.py}を作成します。Chainlitの公式ドキュメントでも@<code>{app.py}が慣例として使われており、本書でもこの名前に統一します。

//listnum[app][app.py][python]{
import chainlit as cl

@cl.on_message
async def main(message: cl.Message):
    await cl.Message(
        content=f"「{message.content}」と言いましたね！",
    ).send()
//}

たったの7行です。それぞれの役割を見てみましょう。

 * @<code>{@cl.on_message}：ユーザーがメッセージを送るたびに、直下の関数を呼び出すよう登録するデコレータです。「メッセージが来たらここを動かす」という宣言だと思えばOKです。
 * @<code>{message: cl.Message}：ユーザーが送ったメッセージオブジェクトです。@<code>{message.content}でテキスト本文を取り出せます。
 * @<code>{cl.Message(...).send()}：Chainlitにレスポンスを送り返す命令です。@<code>{content}に文字列を渡すだけで、チャット画面にメッセージが表示されます。

Webフレームワークでゼロからチャット画面を作ろうとすると、WebSocketの接続管理やHTMLのテンプレート、フロントエンドのコードなど、多くの要素が必要になります。Chainlitはそうした複雑さをすべて内部で引き受けてくれるため、開発者はロジックだけに集中できます。基本的には、「ユーザーからメッセージが来たら何を返すか」だけ書けば、あっという間にチャットアプリが完成します。

さっそく次のコマンドで起動してみましょう。

//cmd{
$ uv run chainlit run app.py -w
//}

@<code>{-w}オプションを付けると、ファイルを変更したときに自動でリロードされるウォッチモードで起動します。開発中はこのオプションを活用すると便利です。

//note[uvを使ったコマンド実行]{
@<code>{uv run}を先頭に付けることで、プロジェクトの仮想環境に入らずともその環境のPythonやコマンドを使って実行できます。@<code>{source .venv/bin/activate}などの仮想環境アクティベーション操作が不要になるため、必要に応じて使い分けるといいでしょう。
//}

== まとめ

本章では、uvを使ったプロジェクト作成からChainlit 2.9.6のインストール、@<code>{chainlit hello}による動作確認、そして生成されるファイル群の役割までを解説しました。

 * @<code>{uv init} でプロジェクトを作成し、@<code>{uv add "chainlit==2.9.6"} でバージョンを固定してインストール
 * @<code>{chainlit hello} でデモを起動し、設定ファイルを初期生成
 * @<code>{.chainlit/config.toml} でアプリの動作を設定
 * @<code>{chainlit.md} でウェルカム画面をカスタマイズ

なお、本章ではChainlitの起動確認までを扱っており、OpenAIやAnthropicなどのLLM APIとの接続（APIキーの設定など）については取り上げていません。各LLMへの接続方法は次章以降のサンプルコードを参考にしてください。

次章からは、実際にChainlitを使ったさまざまなアプリの実装例を見ていきます。ぜひ各章のコードをcloneして、実際に動かしながら読み進めてください！
