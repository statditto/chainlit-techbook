= LLMでインタラクティブな物理パズルゲームを作る

//lead{
Chainlitの標準のチャットUIは、基本的にユーザーがメッセージを送信し、システムがそのメッセージに応答する形で完結しています。
これは、システムとの対話には適していますが、LLMの創造性を十分に活かしたインタラクティブな体験をユーザーに提供することはできません。

しかし、Chainlitには@<code>{send_window_message}という機能があります。
@<code>{send_window_message}は、チャットUIの外側にある独自のフロントエンドにリアルタイムでデータを送ることができるのです。
この機能を使い、ChainlitでLLMに生成させたゲームで遊べれば面白くないでしょうか？

ただ、LLMは毎回固定されたフォーマットで正しいデータを返してくれるとは限りません。
せっかくLLMの創造性に任せてステージを生成させても、ゲームとして成立しないデータが返ってくることもあります。

そこで本章では、LLMを用いて生成したゲームのステージをバリデーションし、@<code>{send_window_message}で独自のフロントエンドにデータを送ることで遊べる物理ゲームの作り方を紹介します。
//}

//pagebreak


== Chainlitで実現する物理パズルゲーム

「簡単なパズルを作って」―チャットにそう入力するだけで、LLMが物理パズルのステージを生成し、ブラウザ上でそのまま遊べるアプリを作ります。

//image[hojo-1][完成イメージ：チャットでパズルを生成し、右側のゲーム画面で遊ぶ]{
//}

ゲームの流れは以下の通りです。

 1. ユーザーがチャットでパズルの要件を入力する（例：「簡単なパズルを作って」など）
 2. LLMがステージを作成し、カスタムフロントエンドにゲームが表示される
 3. ユーザーはボールがゴールに到達できるように板を配置する
 4. スタートボタンを押して、ボールがゴールに到達すればクリア！

「もっと難しくして」や「月面モードのパズルを作って」と追加で指示すれば、LLMが既存のステージを調整して再生成することもできます。
ステージデザイナーを雇わなくても、チャットで無限に新しいパズルを遊ぶことができるのです。


== 実装方法

今回作成するアプリは、たった3つの主要なコンポーネントで構成されており、これらを上手く繋ぎ合わせることでLLMで遊べる物理ゲームが完成します。

//image[hojo-2][データの流れ：チャット → LLM → send_window_message → フロントエンド]{
//}

基本的なデータの流れは以下の通りです。

 1. ユーザーからChainlitのチャットUIを通して、ステージの要望を受け取る
 2. ユーザーの要望をLLMに渡す
 3. LLMが生成したステージJSONをバリデーションして、Chainlitに返す
 4. @<code>{send_window_message}を用いてフロントエンドにステージJSONを送信し、フロントエンドでゲームステージが描画される

各コンポーネントを実現するファイルは、以下の3つです。

//table[files][ファイル構成]{
ファイル	役割	行数
-----------------
app.py	Chainlit UIレイヤー	約30行
llm.py	LLM連携・ステージ生成・バリデーション	約180行
public/index.html	ゲームフロントエンド	約300行
//}

app.pyでは、ユーザーのチャット入力を受け取り、LLMにステージ生成を依頼し、生成されたステージを@<code>{send_window_message}でフロントエンドに送る役割を担います。
30行ほどのコードで、ChainlitのイベントハンドラーやAPIのみで完結しています。
llm.pyでは、LLMにステージ生成を依頼する関数と、LLMの出力をバリデーションする関数を実装します。
今回は、OpenAIのAPIを用いてLLMとやり取りするコードを実装しています。
public/index.htmlでは、ゲームフロントエンドのHTML・CSS・JavaScriptを実装します。
Chainlitの標準チャットUIは@<code>{iframe}で読み込んでいるだけで、あとはゲーム画面のレイアウトや描画ロジックを実装しています。

このアプリの核心は、Chainlitを@<b>{チャットUIとゲーム画面を繋ぐハブ}として使っている点です。
通常のChainlitアプリは「ユーザーがメッセージを送信し、システムがそのメッセージに応答する」だけです。
ところが@<code>{send_window_message}を1行足すだけで、チャットの外側にある独自フロントエンドにリアルタイムでデータを送れるようになります。
つまり、Chainlitが「LLMの出力をゲーム画面に中継するハブ」になるのです。


== 実装してみよう

===  Chainlit UIレイヤー

Chainlit UIレイヤーのコードを紹介します。

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

ここで重要なのは、@<code>{cl.send_window_message}でLLMが生成したステージJSONをフロントエンドに送っている点です。
@<code>{cl.send_window_message}は、Chainlitバックエンドからブラウザの@<code>{window}オブジェクトに任意のデータを送信するAPIです。

//emlist[バックエンドからフロントエンドへの送信]{
await cl.send_window_message({"type": "stage_updated", "stage": stage})
//}

任意のJSONシリアライズ可能なデータを送れます。
本アプリでは、LLMが生成したステージJSON（物体の位置・サイズ・物理パラメータなど数十項目）をまるごと投げています。

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

app.pyでは、他にも@<code>{cl.user_session}を用いて、「現在のステージ」の情報を保持することで、ゲーム仕様を対話的に調整することを可能にしています。
また、@<code>{cl.Step}を用いて、処理状況を可視化することで、UXにも配慮しています。


=== LLM連携

LLMを用いてステージ情報を生成する仕組みを説明します。
ここで、生成するのは以下のようなゲームステージのJSONです。

//emlist[ステージJSONの例]{
{
    "version": "1.0", 
    "stage_id": "33b91f42-dc7b-4449-abcb-6f8495d8c027", 
    "seed": 640993, 
    "difficulty": "easy", 
    "canvas": {"width": 800, "height": 600}, 
    "physics": {"gravityY": 1.0, "gravityX": 0.0, "timeStepMs": 16.67}, 
    "rules": {"win": {"type": "enter_goal", "targetLabel": "ball", 
                      "goalId": "goal-1", "dwellMs": 500}}, 
    "bodies": [{"id": "ball-1", "label": "ball", "shape": "circle", ...}, 
                { ... }
               ], 
    "constraints": [], 
    "ui": {"allowDrag": true, "dragWhitelistTags": ["ramps"]}
//}

==== LLMをゲームデザイナーに仕立てるプロンプト

LLMにステージを生成させるためのプロンプトは、約100行にわたります。
これは、LLMへの「仕様書」であり「ゲームデザインガイドライン」です。

//emlist[システムプロンプト（抜粋）]{
STAGE_GENERATION_SYSTEM = """
あなたは物理パズルゲームのステージを生成する優秀なゲームデザイナーです。
ユーザーの要件に従い、Matter.jsで動作する有効なステージJSONを生成してください。
ステージは必ず解けるものでなければなりません。
出力はJSON形式のステージデータのみで、余計な説明やテキストを含めないでください。

## ゲームメカニクス（重要）
- 幅800px、高さ600px

## 必須ボディ
1. label="ball": ボール（isStatic=false, isSensor=false, color="#60a5fa"）。
   上部左右どちらかに配置。
（中略）

## 難易度ガイドライン（板の枚数と障害物の数で決定）
- easy: 移動可能な板4~5枚(傾きは全てゴール方向)、障害物0個、ゴール大（w=80, h=60）
（中略）

## 物理パラメータ（通常の場合）
- ボール：friction: 0.01, frictionStatic: 0.02, frictionAir: 0.001, 
         restitution: 0.3, density: 0.005
（中略）

## JSONテンプレート
{
    "version": "1.0",
    "stage_id": "uuid",
    "seed": ランダムな整数,
    "difficulty": "easy" | "normal" | "hard",
    "canvas": {"width": 800, "height": 600},
    "physics": {"gravityY": 1.0, "gravityX": 0.0, "timeStepMs": 16.67},
    "rules": {
        "win": {"type": "enter_goal", "targetLabel": "ball", 
                "goalId": "goal-1", "dwellMs": 500}
    },
    "bodies": [ ... ],
    "constraints": [ ... ],
    "ui": {"allowDrag": true, "dragWhitelistTags": ["ramps"]}
}

##  パズルデザインの注意点
- ボールは上部から落下し、板や障害物を経由してゴールに到達する必要がある。
- 板はバラバラに配置して、プレイヤーが並べ直す余地を残す。
- ゴールは下部の床の上に接地させること（y = 590 - h/2）。上方は解放すること。
- テーマ（宇宙、水中など）はカラーパレットで表現。
- ステージは板を正しく配置すれば必ずクリアできるように設計すること。
  絶対に解けないステージは生成しないこと。
- ゴールには必ず "id": "goal-1" を付与すること。
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
LLMの出力を全信頼せず検証することは、LLM連携アプリ全般に通じる鉄則です。


=== ゲームフロントエンド

ゲームのフロントエンドは、public/index.htmlで実装しています。
Chainlitの標準チャットUIは@<code>{iframe}で読み込んで、あとは完全に独立した画面になっています。
フロントエンドの主な役割は、@<code>{send_window_message}で送られてくるステージJSONを物理エンジンであるMatter.jsで描画し、ユーザーの操作を受け付けることです。

Chainlitはプロジェクトルートに@<code>{public/}ディレクトリがあると、その中のファイルを@<code>{/public/}パスで自動的に配信します。
@<code>{public/index.html}を置くだけで@<code>{http://localhost:8000/public/index.html}でアクセスできるようになります。
本アプリでは、Chainlitの@<code>{custom_js}設定で@<code>{public/redirect.js}というスクリプトを注入し、ルートURL（@<code>{/}）にアクセスしたユーザーを自動的に@<code>{/public/index.html}へリダイレクトするようにしています。

//emlist[ディレクトリ構成]{
chainlit-app/
├── app.py
├── llm.py
├── .chainlit/
│   └── config.toml  # custom_js = "/public/redirect.js"
└── public/
    ├── redirect.js  # /public/index.html へ
    └── index.html  # ゲーム画面本体
//}

//image[hojo-3][リダイレクトの仕組み：ルートURLにアクセスしたらゲーム画面へ飛ばす][scale=0.6]{
//}

これにより、@<code>{chainlit run app.py}で起動するだけで、ブラウザにChainlitのチャット＋ゲーム画面が表示されます。


==== レイアウト

//image[hojo-4][2カラムレイアウト：左にチャット、右にゲームキャンバス]{
//}

CSS Gridで画面を左右に分割し、左ペインにiframeでChainlitの標準チャットUIを表示、右ペインにゲームキャンバスを表示しています。

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
これをiframeで読み込むことで、メッセージ送受信やMarkdown表示などの機能をそのまま活用できるのが大きなメリットです。


==== send_window_messageからデータを受け取る

Chainlitバックエンドから@<code>{send_window_message}で送られてくるデータは、フロントエンドのJavaScriptで@<code>{window.addEventListener("message")}を使って受け取ります。
@<code>{send_window_message}で送るデータは、チャットUIの外側にあるフロントエンドにリアルタイムで送られるため、チャットのやり取りとは独立して、ゲーム画面を更新することができます。

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
Chainlitでは、@<code>{send_window_message}で送ったデータを@<code>{{"type": "window_message", "data": ...@}}でラップします。
そのため、上記のように二重に取り出す処理を書くことで正しくデータを受け取ることができます。


==== Matter.jsを用いたゲーム画面の描画

本アプリでは、ブラウザ上で動く2D物理エンジンであるMatter.js@<fn>{matterjs}を使って、ステージの描画と物理シミュレーションを行っています。

//footnote[matterjs][Matter.js：2D物理エンジンのJavaScriptライブラリ。https://brm.io/matter-js/]

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

詳しくはMatter.jsのドキュメントを参照していただきたいですが、各モジュールの役割を簡単にまとめると以下のようになります。

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


==== ステージのJSONからゲーム画面へ

@<code>{send_window_message}で受け取ったステージのJSONは、以下の流れでゲームとして動き出します。

 1. 古いステージのリソース（Engine・Render・Runner）を破棄してクリーンアップ
 2. JSONの@<code>{physics.gravityX}/@<code>{gravityY}で物理エンジン（@<code>{Engine}）を作成
 3. JSONの@<code>{bodies}配列をループし、壁・床・板・ボール・ゴールをMatter.jsのボディに変換
 4. @<code>{Render}を起動してCanvasに描画（この時点では物理演算は停止）
 5. ユーザーがドラッグで板を配置し、スタートボタンで@<code>{Runner}を起動

ステップ1のクリーンアップでは、Render → Runner → Engineの順に停止・破棄します。
チャットで「新しいパズルを作って」と言うたびに、このサイクルの起点となる@<code>{send_window_message}からデータを受け取り、古いリソースをクリーンアップすることで、まっさらな状態からゲーム画面を構築し直します。


== 遊んでみる

それでは、実際にアプリを起動して遊んでみましょう。
アプリを起動して、チャットで「簡単なパズルを作って」と入力すると、LLMがステージを生成し、ゲーム画面が表示されます。

//image[hojo-5][簡単なパズルのステージ]{
//}

簡単にクリアできました。
「もう少し難しくして」と入力して、もう少し難しいステージで遊んでみましょう。

//image[hojo-6][少し難しいパズルのステージ]{
//}

動かせる板が減り、障害物が出てきました。少し頭を使いますが、なんとかクリアできました。
さらに「もっと難しくして」と入力して、もっと難しいステージで遊んでみましょう。

//image[hojo-7][かなり難しいパズルのステージ]{
//}

動かせる板がさらに減り、障害物は増えました。
頑張ってみましたが、ゴールを通り過ぎてしまい、失敗してしまいました。

このように、チャットでステージの要件を伝えるだけで、LLMが生成したステージで遊ぶことができます。

== おわりに

本章では、LLMによるゲームオブジェクトの生成とChainlitの@<code>{send_window_message}を組み合わせて物理パズルゲームを実装しました。

@<code>{send_window_message}は、Chainlitをチャットの枠を超えたインタラクティブアプリのプラットフォームに変える強力な機能です。
チャットUIから独自のフロントエンドにデータを送れるこの仕組みは、物理パズルに限らず、様々な応用が考えられます。

 * @<b>{可視化ダッシュボード} ― LLMが生成したグラフやチャートをリアルタイム描画
 * @<b>{コード実行環境} ― チャットで書いたコードをサンドボックスで実行
 * @<b>{インタラクティブ教材} ― チャットの指示で図やアニメーションを操作

ぜひ、@<code>{send_window_message}を活用して、チャットと連動する新しい体験を作ってみてください。
