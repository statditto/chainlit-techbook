
= send_window_messageで物理パズルゲームを作る

//lead{
Chainlitのカスタムフロントエンド機能と@<code>{send_window_message}を使い、チャットでLLMに指示するだけで物理パズルが自動生成されるWebアプリを作ります。
チャットUI × 物理エンジン（Matter.js）のゲーム画面をリアルタイムに繋ぐ実装を紹介します。
//}

//pagebreak


== 本章の概要

「簡単なパズルを作って」―チャットにそう入力するだけで、LLMが物理パズルのステージを生成し、ブラウザ上でそのまま遊べるアプリを作ります。
物理エンジンにはMatter.js@<fn>{matterjs}を使っています。

//footnote[matterjs][Matter.js：2D物理エンジンのJavaScriptライブラリ。https://brm.io/matter-js/]

//image[hojo-1][完成イメージ：チャットでパズルを生成し、右側のゲーム画面で遊ぶ]{
//}

ゲームの流れは簡単です。

 1. ユーザーがチャットでパズルの要件を入力する（例：「簡単なパズルを作って」など）
 2. バックエンドでLLMがステージのJSONデータを生成する
 3. Chainlitの@<code>{send_window_message}で、JSONをフロントエンドに送信する
 4. フロントエンドがMatter.jsでステージを描画する
 5. ユーザーがオレンジ色の板をドラッグして配置し、スタートボタンでボールを転がす
 6. ボールがゴールに到達すればクリア！

「もっと難しくして」や「月面モードのパズルを作って」と追加で指示すれば、LLMが既存のステージを調整して再生成してくれます。
ステージデザイナーを雇わなくても、チャットで無限に新しいパズルが遊べます。


=== アプリの概要

作成したアプリを紹介します。
主要ファイルはたった3つです。

//table[files][ファイル構成]{
ファイル	役割	行数
-----------------
app.py	Chainlit UIレイヤー	約30行
llm.py	LLM連携・ステージ生成	約180行
public/index.html	ゲームフロントエンド	約300行
//}

設定ファイル（@<code>{.chainlit/config.toml}、@<code>{.env}など）を除けば、たったこれだけで動作します。
app.pyに至っては30行。Pythonの入門書に載っていそうな短さですが、ちゃんとゲームとして動作します。


== アーキテクチャ：Chainlitを「ハブ」にする

//image[hojo-2][データの流れ：チャット → LLM → send_window_message → フロントエンド]{
//}

このアプリの核心は、Chainlitを@<b>{チャットUIとゲーム画面を繋ぐハブ}として使っている点です。

通常のChainlitアプリは「ユーザーがメッセージを送信し、システムがそのメッセージに応答する」だけです。
ところが@<code>{send_window_message}を1行足すだけで、チャットの外側にある独自画面にリアルタイムでデータを送れるようになります。
つまり、Chainlitが「LLMの出力をゲーム画面に中継するハブ」になるのです。

本書の別章（@<chapref>{ditto}）では@<code>{CustomElement}を使って、チャットメッセージの中にUIを埋め込むアプローチを紹介しています。
一方、本章の@<code>{send_window_message}はチャットの@<b>{外に}あるフロントエンドにデータを送ります。
どちらもChainlitの拡張手法ですが、用途が異なります。

//table[comparison][CustomElementとsend_window_messageの比較]{
項目	CustomElement	send_window_message
-----------------
UIの配置	チャットメッセージ内	チャット外の独自画面
送信方向	双方向（props + callAction）	バックエンド→フロントエンド
適した用途	フォーム、お絵かき等	ダッシュボード、ゲーム等
//}


== バックエンド
バックエンドは、ユーザーのチャット入力を受け取り、LLMにステージ生成を依頼し、その結果を@<code>{send_window_message}でフロントエンドに送る役割を担います。
Chainlitの基本的な機能（@<code>{on_chat_start}、@<code>{on_message}、@<code>{cl.Message}など）を使いながら、@<code>{cl.Step}で処理状況を可視化し、@<code>{cl.user_session}でセッション変数を管理し、@<code>{send_window_message}でデータを送る、という流れになります。


===  Chainlit UIレイヤー

Chainlit UIレイヤーのコードはわずか30行です。

//emlist[Chainlit UIレイヤー（app.py）]{
import chainlit as cl
from llm import generate_stage

@cl.on_chat_start
async def on_chat_start() -> None:
    cl.user_session.set("current_stage", None)
    await cl.Message(
        content=(
            "パズルシミュレーターへようこそ！\n"
            "板を並べてゴールを目指しましょう！\n"
            "**例：** `簡単なパズルを作って` / "
            "`難しいパズルを作って` / `重力を強くして`"
        ),
    ).send()

@cl.on_message
async def on_message(message: cl.Message) -> None:
    current_stage = cl.user_session.get("current_stage")
    action = "調整" if current_stage else "生成"
    async with cl.Step(f"ステージ{action}中...", type="tool", show_input=False):
        stage = await generate_stage(message.content.strip(), current_stage)
        cl.user_session.set("current_stage", stage)
    diff = stage.get("difficulty", "normal")
    label = {"easy": "優しい", "normal": "普通", 
             "hard": "難しい"}.get(diff, diff)
    await cl.Message(
        content=f"ステージ{action}完了！ 難易度: {label}"
    ).send()
    await cl.send_window_message({"type": "stage_updated", "stage": stage})
//}

非常に短いコードですが、Chainlitの重要な機能が多く詰まっています。順に見ていきましょう。


==== @cl.on_chat_startと@cl.on_message ― イベントハンドリング

Chainlitでは、デコレータでイベントハンドラを登録します。

@<code>{@cl.on_chat_start}は、ユーザーがチャットセッションを開始したときに1度だけ呼ばれます。
本アプリでは、セッション変数の初期化とウェルカムメッセージの送信に使っています。

@<code>{@cl.on_message}は、ユーザーがメッセージを送信するたびに呼ばれるハンドラです。
引数の@<code>{message: cl.Message}からユーザーの入力テキスト（@<code>{message.content}）を取得できます。

本アプリの@<code>{on_message}では、以下の処理を行っています。

 1. @<code>{cl.user_session}から現在のステージを取得し、新規生成か調整かを判定
 2. @<code>{cl.Step}で処理中表示を出しながら、LLMにステージ生成を依頼
 3. 生成結果を@<code>{cl.user_session}に保存し、完了メッセージを返す
 4. ステージJSONを@<code>{cl.send_window_message}でフロントエンドに送信

たった十数行のハンドラですが、「チャット入力 → LLM → ゲーム画面」というパイプライン全体がここで組み立てられています。
以降のセクションでは、このハンドラの中で使われている各機能について詳しく説明します。


==== cl.user_session ― セッション変数の管理

@<code>{cl.user_session}は、ユーザーセッションごとにデータを保持する辞書的オブジェクトです。

//emlist[セッション変数の操作（@<code>{cl.user_session}）]{
cl.user_session.set("current_stage", None)     # 初期化
current = cl.user_session.get("current_stage") # 取得
//}

本アプリでは「現在のステージ」の情報をここに保持しています。
2回目以降のメッセージではこのステージを「下書き」としてLLMに渡すことで、「新規生成」ではなく「調整」モードで動作させることができます。
データベースは不要で、セッション中だけ有効な軽量なストレージです。
「重力を強くして」「板を増やして」といった対話的な調整を可能にするのに欠かせません。
@<chapref>{ckato}の章では、データの永続化について詳しく解説されていますが、本アプリではセッション変数だけで完結させています。


==== cl.Step ― 処理状況の可視化

@<code>{cl.Step}はコンテキストマネージャで、実行中の処理をチャットUI上に表示することができます。

//emlist[処理状況の可視化（@<code>{cl.Step}）]{
async with cl.Step("ステージ生成中...", type="tool", show_input=False):
    stage = await generate_stage(...)
//}

@<code>{with}ブロックに入ることで現在の処理状況が表示され、ブロックを抜けると完了になります。
LLMのレスポンスには数秒かかるため、ユーザーに処理中であると伝えることはUXとして非常に重要です。
何のフィードバックもなく数秒待たされると、ユーザーはアプリが止まったと勘違いするかもしれません。


==== cl.send_window_message ― フロントエンドへのデータ送信

@<code>{cl.send_window_message}は、Chainlitバックエンドからブラウザの@<code>{window}オブジェクトに任意のデータを送信するAPIです。
本章のアプリを支える最も重要な機能です。

//emlist[バックエンドからフロントエンドへの送信]{
await cl.send_window_message({"type": "stage_updated", "stage": stage})
//}

任意のJSONシリアライズ可能なデータを送れます。
本アプリでは、LLMが生成したステージJSON（物体の位置・サイズ・物理パラメータなど数十項目）をまるごと投げています。

受信側のフロントエンドでは、@<code>{window.addEventListener("message")}でこのデータを受け取ります。

//emlist[フロントエンド側の受信（public/index.html）]{
window.addEventListener("message", e => {
  const d = e.data;
  if (!d || typeof d !== "object") return;
  // Chainlitはwindow_messageでラップして送信する
  const p = (d.type === "window_message" && d.data) ? d.data : d;
  if (p.type === "stage_updated" && p.stage) {
    loadStage(p.stage);
  }
});
//}

ここで1つ注意点があります。
Chainlitは@<code>{send_window_message}で送ったデータを@<code>{{"type": "window_message", "data": ...@}}でラップします。
「あれ、送ったはずのデータが取れない…」と悩んだ末にこのラップに気づく―というのは筆者だけではないはずです。
上記のように二重に取り出す処理を書くことで正しくデータを受け取ることができます。


=== LLM連携

llm.pyでは、LLMにステージ生成を依頼する関数を実装しています。

==== LLMをゲームデザイナーに仕立てるプロンプト

LLMにステージを生成させるためのシステムプロンプトは、約100行にわたります。
LLMへの「仕様書」であり「ゲームデザインガイドライン」です。

//emlist[システムプロンプトの冒頭（抜粋）]{
STAGE_GENERATION_SYSTEM = """
あなたは物理パズルゲームのステージを生成する優秀なゲームデザイナーです。
ユーザーの要件に従い、Matter.jsで動作する有効なステージJSONを生成してください。
ステージは必ず解けるものでなければなりません。
出力はJSON形式のステージデータのみで、余計な説明やテキストを含めないでください。
"""
//}

プロンプトに含めている要素を整理します。

 * @<b>{ゲームメカニクス} ― ボールは自由落下、板はドラッグ可能、ゴールはセンサー
 * @<b>{難易度ガイドライン} ― easy/normal/hardごとの板の枚数・障害物の数・ゴールサイズ
 * @<b>{物理パラメータ} ― friction、restitution、densityの推奨値
 * @<b>{JSONテンプレート} ― 出力フォーマットの具体例
 * @<b>{座標計算式} ― 「ゴールはy = 590 - h/2」のような具体的な数式

特に最後がポイントです。
「床の上に配置して」という曖昧な指示では、LLMはゴールを宙に浮かせたり壁に埋めたりすることがあります。
「y = 590 - h/2」と数式で座標を示せば、ゲームとして成立するステージを安定して生成させることができます。
LLMには曖昧さを与えず、具体的に伝えることが大切です。


==== LLMを信用しないバリデーション

LLMの出力は常に正しいとは限りません。
たまにゴールのないステージを生成したり、ボールを2つ配置したりと、自由奔放な創造性を発揮します。
そこで、生成されたステージがゲームとして成立しているかをバリデーションする関数を実装します。
@<code>{jsonschema}でスキーマ検証し、さらにゲームロジック上の整合性もチェックしています。

//emlist[バリデーション]{
def validate_stage(stage: dict) -> bool:
    try:
        jsonschema.validate(stage, STAGE_SCHEMA)
        labels = [b.get("label")
                  for b in stage.get("bodies", [])]
        if "ball" not in labels \
           or "goal" not in labels:
            return False
        goal_id = stage["rules"]["win"]["goalId"]
        ids = [b.get("id")
               for b in stage.get("bodies", [])]
        return goal_id in ids
    except jsonschema.ValidationError:
        return False
//}

この関数では、3段階のチェックを行っています。

 1. @<b>{JSON Schemaによるスキーマ検証} ― 必須フィールドの存在、型の整合性
 2. @<b>{必須ラベルの存在確認} ― @<code>{ball}と@<code>{goal}がbodies内にあるか
 3. @<b>{ゴールIDの整合性} ― ルールの@<code>{goalId}が実際のボディIDと一致するか

バリデーションに失敗した場合は最大3回リトライします。
「LLMの出力は正しいはず」という前提でコードを書くと、ユーザーに壊れたステージが届いてしまいます。
LLMの出力を全信頼せず、検証することは、LLM連携アプリ全般に通じる鉄則です。


=== 会話でパズルを調整する

@<code>{generate_stage}関数は、既存のステージがある場合はそのJSONをコンテキストとしてLLMに渡します。
これは@<code>{cl.user_session}に保存された現在のステージを引数として受け取る形になっています。

//emlist[ステージ調整モード]{
async def generate_stage(
    requirements: str,
    current_stage: dict = None) -> dict:
    context = ""
    if current_stage:
        context = (
            f"\n\n現在のステージの情報\n"
            f"```json\n"
            f"{json.dumps(current_stage, indent=2)}"
            f"\n```\n\n"
            f"このステージを{requirements}の"
            f"要件に合わせて調整してください。")
//}

これにより「もっと難しくして」「重力を弱くして」「障害物を増やして」といった自然言語での調整が可能になります。
チャットで対話しながらパズルを微調整できるのは、Chainlit + LLMならではの機能です。


== フロントエンド

public/index.htmlでは、ゲームのフロントエンドを実装しています。
Chainlitの標準チャットUIは@<code>{iframe}で読み込んでいるだけで、あとは完全に独立した画面になっています。
フロントエンドの主な役割は、@<code>{send_window_message}で送られてくるステージJSONをMatter.jsで描画し、ユーザーの操作を受け付けることです。

Chainlitはプロジェクトルートに@<code>{public/}ディレクトリがあると、その中のファイルを@<code>{/public/}パスで自動的に配信します。
@<code>{public/index.html}を置くだけで@<code>{http://localhost:8000/public/index.html}でアクセスできるようになります。
本アプリでは、Chainlitの@<code>{custom_js}設定で@<code>{public/redirect.js}というスクリプトを注入し、ルートURL（@<code>{/}）にアクセスしたユーザーを自動的に@<code>{/public/index.html}へリダイレクトしています。
これにより、@<code>{chainlit run app.py}で起動するだけで、ブラウザにチャット＋ゲーム画面が表示されます。


=== レイアウト

//image[hojo-3][2カラムレイアウト：左にチャット、右にゲームキャンバス]{
//}

CSS Gridで画面を左右に分割し、左ペインに標準チャットUIをiframeで表示、右ペインにゲームキャンバスを表示しています。

//emlist[2カラムレイアウト（HTML・CSS抜粋）]{
<style>
  .app {
    min-height: 100vh;
    display: grid;
    grid-template-columns: 400px 1fr;
  }
  .chat iframe {
    width: 100%; height: 100vh; border: 0;
  }
</style>
<main class="app">
  <section class="chat">
    <iframe src="/"></iframe>
  </section>
  <section class="game">
    <!-- ツールバー、キャンバス、ステータスバー -->
  </section>
</main>
//}

iframeの@<code>{src="/"}はChainlitのルートURLです。通常、@<code>{chainlit run app.py}で起動するとルートURLにChainlitの標準チャットUIが配信されます。
これをiframeで読み込むことで、メッセージ送受信・Markdown表示・Step表示などの機能をそのまま活用できるのが大きなメリットです。


=== Matter.jsの概要

本アプリでは、ブラウザ上で動く2D物理エンジンであるMatter.jsを使って、ステージの描画と物理シミュレーションを行っています。
CDNから1行で読み込むだけで、重力・衝突・摩擦といったリアルな物理シミュレーションが使えるようになります。

//emlist[CDNからの読み込み]{
<script src="https://cdn.jsdelivr.net/npm/matter-js@0.20.0/build/matter.min.js">
</script>
//}

Matter.jsはグローバルに@<code>{Matter}オブジェクトを公開しており、そこからモジュールを分割代入で取り出して使います。

//emlist[モジュールの取り出し]{
const { Engine, Render, Runner, Bodies, Body,
        Composite, Query, Events } = Matter;
//}

各モジュールの役割を整理しておきます。

//table[matter-modules][Matter.jsの主要モジュール]{
モジュール	役割
-----------------
Engine	物理演算の中核。重力の設定、シミュレーションの管理
Render	Canvas上への描画を担当
Runner	物理演算ループを一定間隔で実行
Bodies	円や矩形などの物体を生成するファクトリ
Body	既存の物体を操作（位置・速度・静的/動的の切り替え）
Composite	Engineのworldに物体を追加・削除・一覧取得
Query	指定座標にある物体を検索
Events	Engine・Renderにイベントリスナーを登録
//}


=== send_window_messageからゲーム画面へ

@<code>{send_window_message}で受け取ったステージのJSONは、以下の流れでゲームとして動き出します。

 1. 古いステージのリソース（Engine・Render・Runner）を破棄してクリーンアップ
 2. JSONの@<code>{physics.gravityX}/@<code>{gravityY}で物理エンジン（@<code>{Engine}）を作成
 3. JSONの@<code>{bodies}配列をループし、壁・床・板・ボール・ゴールをMatter.jsのボディに変換
 4. @<code>{Render}を起動してCanvasに描画（この時点では物理演算は停止）
 5. ユーザーがドラッグで板を配置し、スタートボタンで@<code>{Runner}を起動

ステップ1のクリーンアップでは、Render → Runner → Engineの順に停止・破棄します。
チャットで「新しいパズルを作って」と言うたびにこのクリーンアップが走り、まっさらな状態からゲーム画面を構築し直します。

ステップ2〜3では、@<code>{Engine}が物理演算を、@<code>{Render}がCanvasへの描画を、@<code>{Runner}がフレームごとの更新ループを担当します。
この3つが分離しているのがMatter.jsの設計で、本アプリではステージ読み込み時に@<code>{Render}だけを起動し、@<code>{Runner}はスタートボタンが押されるまで起動しません。
これにより、ボールは@<code>{isStatic: true}で固定され、描画はされるが物理演算は進まない「スタート待ち」の状態を実現しています。


=== 物体（ボディ）の生成

ステージJSONの@<code>{bodies}配列をループし、Matter.jsのボディに変換します。

//emlist[ステージJSONからMatter.jsボディへの変換]{
for (const b of stage.bodies) {
  const ramp = b.label === "ramp";
  const opts = {
    isStatic: ramp ? true : (b.isStatic !== false),
    isSensor: !!b.isSensor,
    friction: b.friction ?? 0.1,
    restitution: b.restitution ?? 0.2,
    density: b.density ?? 0.001,
    angle: b.angle || 0,
    render: {
      fillStyle: ramp ? "#fb923c" : (b.color||"#888")
    },
  };
  let body;
  if (b.type === "circle" || b.r != null) {
    body = Bodies.circle(b.x, b.y, b.r || 15, opts);
  } else {
    body = Bodies.rectangle(
      b.x, b.y, b.w || 100, b.h || 20, opts);
  }
  Composite.add(engine.world, body);
}
//}

ポイントは、LLMが生成するJSONの構造がMatter.jsのパラメータにそのまま対応している点です。
@<code>{friction}（摩擦）、@<code>{restitution}（反発）、@<code>{density}（密度）といった物理パラメータをJSONに含めているため、プロンプトを通じてLLMがゲームの物理的な振る舞いまでコントロールできます。

ゴールは@<code>{isSensor: true}にしているため、ボールが物理的にぶつかることはなく、すり抜けます。
衝突判定だけが有効で、ゴール領域にボールが入ったかどうかを検知するために使っています。


=== ゲームのライフサイクル

ゲームには@<code>{idle}（配置中）→ @<code>{playing}（プレイ中）→ @<code>{clear}（クリア）の3つの状態があります。

//emlist[スタート：ボールを物理演算に委ねる]{
function startGame() {
  gameState = "playing";
  Body.setStatic(currentBallBody, false);
  runner = Runner.create();
  Runner.run(runner, engine);
}
//}

スタートボタンが押されると@<code>{Body.setStatic(false)}でボールを動的に切り替え、@<code>{Runner}を起動します。
これだけで、ボールは重力に従って落下し始めます。

リセット時は@<code>{Runner}を止めてから、すべてのボディの位置・角度・速度を初期値に戻します。
ステージ読み込み時に各ボディの初期状態を配列に保存しておき、リセット時にそこから復元しています。

ゴール領域にボールが一定時間（@<code>{dwellMs}ミリ秒）止まるとクリアです。
@<code>{dwellMs}もステージのJSONの一部なので、LLMが難易度に応じて値を変えることもできます。
チャットで「新しいパズルを作って」と言うたびに、このサイクルの起点となる@<code>{send_window_message}からすべてが始まります。


== まとめ

本章では、Chainlitの以下の機能を組み合わせて物理パズルゲームを実装しました。

 * @<code>{@cl.on_chat_start}/@<code>{@cl.on_message} ― イベントハンドリング
 * @<code>{cl.user_session} ― セッション単位のデータ保持
 * @<code>{cl.Step} ― 処理中の可視化
 * @<code>{cl.send_window_message} ― フロントエンドへのリアルタイム通信
 * @<code>{public/index.html} ― カスタムフロントエンドの配信

中でも@<code>{cl.send_window_message}は、Chainlitをチャットの枠を超えたインタラクティブアプリのプラットフォームに変える強力な機能です。
チャットUIから独自のフロントエンドにデータを送れるこの仕組みは、物理パズルに限らず、様々な応用が考えられます。

 * @<b>{可視化ダッシュボード} ― LLMが生成したグラフやチャートをリアルタイム描画
 * @<b>{コード実行環境} ― チャットで書いたコードをサンドボックスで実行
 * @<b>{インタラクティブ教材} ― チャットの指示で図やアニメーションを操作

ぜひ、@<code>{cl.send_window_message}を活用して、チャットと連動する新しい体験を作ってみてください。
