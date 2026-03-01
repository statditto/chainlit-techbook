= CustomElementを使って絵しりとりを作る！

//lead{
Chainlitのチャット画面の中にお絵かきキャンバスを埋め込み、AIと絵しりとりで遊べるWebアプリを作ります。
標準のMessage/Actionだけではできない、CustomElementという仕組みを主役にした実装を紹介します。
//}

//pagebreak

本章のソースコード全体はサポートページ@<fn>{support}で公開しています。本文中のコードは要点を抜粋したものです。
//footnote[support][https://github.com/xxx/e-shiritori （※実際のURLに差し替えてください）]

== この章で作るもの

この章では、ユーザーがキャンバスに絵を描き、AIが「しりとり」のルールで次の絵を返す――そんな「絵しりとり」Webアプリを、Chainlit + Gemini APIで作ります。

//image[sample-diagram][完成イメージ：チャット内にキャンバスが表示され、描いた絵をAIが判定して次の絵を返す]{
//}

ゲームの流れは@<img>{sample-diagram}のとおりです。ユーザーが絵を描き、AIが判定して次の絵を返す――これをNラウンド繰り返します。

//image[sample-diagram][ゲームフロー]{
//}

=== たった3ファイルで動く

ファイル構成を見てみましょう。驚くほどシンプルです。

//table[files][ファイル構成]{
ファイル    役割
-----------------
app.py  Chainlit UIレイヤー（約150行）
gemini.py   Gemini APIレイヤー（約120行）
public/elements/DrawCanvas.jsx  お絵かきキャンバス（約155行）
//}

あとは設定ファイル（@<code>{.chainlit/config.toml}、@<code>{.env}）とCSS/JSが少々。
3ファイル合計400行台で、AIと絵しりとりができるアプリが完成します（@<img>{sample-diagram}）。

//image[sample-diagram][アーキテクチャ：3ファイルの役割とデータの流れ]{
//}

Chainlitの基本（@<code>{on_chat_start}、@<code>{cl.Message}など）は本書の他章に譲ります。本章では、Chainlitの「裏技」的な機能であるCustomElementに焦点を当てます。

== CustomElementの基本

=== なぜCustomElementが必要か

Chainlitには@<code>{cl.Message}でテキストや画像を送る機能、@<code>{cl.Action}でボタンを配置する機能があります。
しかし「キャンバスに自由にお絵かき」のようなリッチなインタラクションは、標準の仕組みだけでは実現できません。
そこで登場するのがCustomElementです。
JSXコンポーネントを書いて@<code>{public/elements/}に置くだけで、チャットのメッセージ内に独自のUIを埋め込めます。

=== 仕組み：3つのポイント

CustomElementで押さえるべきポイントは3つだけです。

==== ① JSXファイルを置くだけで認識される

@<code>{public/elements/DrawCanvas.jsx}というファイルを作ると、バックエンドから@<code>{cl.CustomElement(name="DrawCanvas")}で呼び出せるようになります。
ファイル名とname引数が対応する、というシンプルな規約です。

//emlist[バックエンド側（app.py）]{
canvas = cl.CustomElement(
    name="DrawCanvas",  # public/elements/DrawCanvas.jsx に対応
    props={"hint": "好きな絵を描いてね！", "round": 1, "maxRounds": 5},
)
await cl.Message(content="描いてね！", elements=[canvas]).send()
//}

==== ② propsでバックエンドからデータを受け取る

バックエンドで指定した@<code>{props}は、JSX内でグローバル変数@<code>{props}として参照できます。
関数の引数ではなくグローバルに注入される点が通常のReactコンポーネントとの違いです。

//emlist[フロントエンド側（DrawCanvas.jsx）]{
// propsはグローバルに注入される（引数ではない！）
const hint = props.hint || "絵を描いてね！";
const round = props.round || 1;
const maxRounds = props.maxRounds || 5;
//}

==== ③ callActionでフロントエンドからバックエンドに送信する

ユーザーの操作結果をバックエンドに送るには、グローバル関数@<code>{callAction}を使います。

//emlist[フロントエンド側（DrawCanvas.jsx）]{
callAction({
  name: "submit_drawing",
  payload: { image: canvas.toDataURL("image/png") }
});
//}

//emlist[バックエンド側（app.py）]{
@cl.action_callback("submit_drawing")
async def on_submit(action: cl.Action):
    img_data = action.payload.get("image", "")
    # Base64デコードして処理...
//}

@<code>{callAction}の@<code>{name}と@<code>{@cl.action_callback}のname引数が対応します。
payloadには任意のJSONシリアライズ可能なデータを載せられるため、今回はBase64エンコードした画像データを送っています。

=== 使えるライブラリ

CustomElementのJSX内では、以下のライブラリがimport可能です。
 * React（@<code>{useState}、@<code>{useEffect}、@<code>{useRef}など）
 * shadcn/ui（@<code>{@/components/ui/button}など）
 * lucide-react（アイコン）

逆にいえば、任意のnpmパッケージは使えません。
しかしCanvas APIはブラウザ標準なので、お絵かき機能はこの制約内で十分実装可能です。

== お絵かきキャンバスを実装する

いよいよ本章の主役、@<code>{DrawCanvas.jsx}の実装に入ります。
コード全体（約155行）はサポートページを参照してください。ここでは実装のポイントを5つに絞って解説します。
@<code>{DrawCanvas.jsx}の構造を大まかにまとめると次のとおりです。

 * state管理 ― @<code>{isDrawing}（描画中か）、@<code>{penColor}、@<code>{penSize}、@<code>{submitted}（送信済みか）
 * props受信 ― バックエンドから@<code>{hint}、@<code>{round}、@<code>{maxRounds}を受け取る
 * 描画ロジック ― Canvas APIで@<code>{beginPath}→@<code>{lineTo}→@<code>{stroke}
 * 送信 ― @<code>{callAction}でBase64画像をバックエンドに送る
 * JSX ― canvas要素＋色/太さ入力＋クリア/送信ボタン

=== ポイント① Canvas APIでの描画

お絵かきの核となるのは、HTML5 Canvas APIの@<code>{beginPath}→@<code>{moveTo}→@<code>{lineTo}→@<code>{stroke}のサイクルです。

//emlist[描画の基本サイクル]{
const startDraw = (e) => {
  const ctx = canvas.getContext("2d");
  const pos = getPos(e, canvas);
  ctx.beginPath();        // 新しいパスを開始
  ctx.moveTo(pos.x, pos.y);  // 始点を設定
  setIsDrawing(true);
};
const draw = (e) => {
  if (!isDrawing) return;
  const pos = getPos(e, canvas);
  ctx.lineTo(pos.x, pos.y);  // 始点から現在位置まで線を引く
  ctx.stroke();               // 実際に描画
};
//}

@<code>{lineCap = "round"}を設定することで、線の端が丸くなり、手書き風の滑らかな線になります。

=== ポイント② 座標補正とタッチ対応

CSSで@<code>{width: 100%}とレスポンシブにしているため、canvasの表示サイズと内部解像度（400×400）にズレが生じます。

@<code>{getBoundingClientRect()}で表示サイズを取得し、比率で補正します。
//emlist[座標補正（マウス・タッチ兼用）]{
const getPos = (e, canvas) => {
  const rect = canvas.getBoundingClientRect();
  const scaleX = canvas.width / rect.width;
  const scaleY = canvas.height / rect.height;
  const src = e.touches ? e.touches[0] : e;
  return {
    x: (src.clientX - rect.left) * scaleX,
    y: (src.clientY - rect.top) * scaleY,
  };
};
//}

タッチイベント（@<code>{onTouchStart}等）もマウスイベントと同じハンドラで処理しています。
canvasタグにTailwindの@<code>{touch-none}クラスを付けることで、描画中のページスクロールを防止しています。

=== ポイント③ callActionで画像を送信する

「送る」ボタンが押されたら、canvasの内容をBase64エンコードしてバックエンドに送ります。

//emlist[画像の送信]{
const submit = () => {
  if (submitted) return;
  const canvas = canvasRef.current;
  const imageData = canvas.toDataURL("image/png");
  setSubmitted(true);
  callAction({
    name: "submit_drawing",
    payload: { image: imageData }
  });
};
//}

@<code>{canvas.toDataURL("image/png")}で@<code>{data:image/png;base64,...}形式の文字列が得られます。
これをそのまま@<code>{callAction}のpayloadに含めてバックエンドに送信します。
送信後は@<code>{setSubmitted(true)}でボタンとキャンバスを無効化し、二重送信を防止しています。

=== ポイント④ propsで毎ラウンドの情報を受け取る

ラウンドが進むたびに、バックエンドは新しいCustomElementを生成して送ります。
フロントエンドでは@<code>{props}の変化を@<code>{useEffect}で検知して、キャンバスを白紙にリセットしています。

//emlist[propsの変化でキャンバスをリセット]{
useEffect(() => {
  const canvas = canvasRef.current;
  if (!canvas) return;
  const ctx = canvas.getContext("2d");
  ctx.fillStyle = "#ffffff";
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  setSubmitted(false);
}, [props.hint]);  // hintが変わったら（=新ラウンド）リセット
//}

== バックエンドでゲームを動かす

キャンバスから送られてきた画像データを受け取り、Gemini APIで処理し、結果を返すバックエンドの実装を見ていきます。

=== ゲームフローの全体像

@<code>{action_callback}に集約されたゲームフローは、次のステップで構成されます。
//emlist[app.py：action_callbackの骨格（抜粋）]{
@cl.action_callback("submit_drawing")
async def on_submit(action: cl.Action):
    await action.remove()           # 「使用済み」表示を防止
    img_bytes = decode(action.payload)  # Base64デコード
    # AIが絵を判定 → しりとりの次の単語を選ぶ
    async with cl.Step(name="🤔 考え中...") as step:
        thinking, user_word = await identify_drawing(...)
        step.output = thinking
    # AIの絵を生成して表示 → 次のキャンバスを送る
    ai_img_bytes = await generate_image(ai_word)
    await cl.Message(
        content="続きを描いてね！",
        elements=[_canvas(hint, round_num + 1)],
    ).send()
//}

①の@<code>{await action.remove()}は地味ですが重要です。
@<code>{callAction}でアクションを実行すると、Chainlitはデフォルトで「使用済み」というフィードバックをUIに表示します。
コールバック先頭で@<code>{action.remove()}を呼ぶことで、この表示を抑制できます。
なお@<code>{cl.Step}の完了時にも同じ「使用済み」が表示されます。
これは@<code>{.chainlit/translations/ja.json}を編集し、@<code>{chat.messages.status.used}の値を「完了！」などに変更するとよいでしょう。
また、@<code>{_canvas}ヘルパーで毎ラウンド新しいCustomElementを生成しているのもポイントです。

//emlist[app.py：キャンバス生成ヘルパー]{
def _canvas(hint, round_num):
    return cl.CustomElement(
        name="DrawCanvas",
        props={"hint": hint, "round": round_num,
               "maxRounds": MAX_ROUNDS},
    )
//}

=== cl.Stepで「AIの思考」を折りたたみ表示

@<code>{cl.Step}はコンテキストマネージャとして使うことで、チャット内に折りたたみ可能なセクションを作れます。
//emlist[cl.Stepの基本パターン]{
async with cl.Step(name="🤔 AIが絵を見て考え中...") as step:
    thinking, user_word = await identify_drawing(img_bytes, last_char)
    step.output = thinking  # 折りたたみの中に表示される内容
//}

cl.Stepを活用すると、AIの思考過程（「えっとね、まるいかたちで…これはりんごかな！」）を見せながら、チャット画面をすっきり保てます。

=== Gemini APIの構造化出力で応答を安定させる

LLMの出力は自由形式のテキストです。
「思考過程」と「回答の単語」を分離して取得する必要があります。正規表現でパースする方法もありますが、Gemini APIの@<b>{構造化出力}（Structured Output）を使えば確実です。

//emlist[gemini.py：スキーマ定義と利用（抜粋）]{
_SHIRITORI_SCHEMA = {
    "type": "object",
    "properties": {
        "thinking": {"type": "string",
            "description": "幼稚園児風の短い思考過程"},
        "answer":   {"type": "string",
            "description": "ひらがなの単語"},
    },
    "required": ["thinking", "answer"],
}
resp = client.models.generate_content(
    model=VISION_MODEL, contents=[image_part, prompt],
    config=types.GenerateContentConfig(
        response_mime_type="application/json",
        response_schema=_SHIRITORI_SCHEMA,
    ),
)
data = json.loads(resp.text)  # 必ずスキーマどおりのJSONが返る
//}

@<code>{response_mime_type="application/json"}と@<code>{response_schema}を組み合わせることで、モデルの出力を必ず指定したJSON形式に制約できます。
正規表現によるパースが不要になり、出力が安定します。

=== 画像生成：FlashとImagen

Gemini APIで画像を生成する方法は2つあります。Flashは@<code>{response_modalities}に@<code>{["IMAGE", "TEXT"]}を指定します。レスポンスのpartsから@<code>{inline_data}を取り出して画像データを得ます。Imagenは@<code>{generate_images}メソッドを使う専用APIです。
//emlist[gemini.py：Flashでの画像生成（抜粋）]{
resp = client.models.generate_content(
    model="gemini-2.0-flash-exp-image-generation",
    contents="Draw a cute crayon illustration of: apple.",
    config=types.GenerateContentConfig(
        response_modalities=["IMAGE", "TEXT"]),
)
for part in resp.candidates[0].content.parts:
    if part.inline_data is not None:
        image_bytes = part.inline_data.data
//}

本アプリではFlashをデフォルトにしつつ、環境変数@<code>{IMAGE_MODEL}でImagenに切り替えられるようにしています。
なお、画像生成プロンプトは英語の方が品質がよいため、日本語の単語をいったんGeminiで英訳してから渡しています。

== UIの仕上げ

=== custom_cssで不要な要素を隠す

絵しりとりではテキスト入力を使わないため、Chainlit標準のメッセージ入力欄を非表示にします。
//list[css][public/stylesheet.css][css]{
/* テキスト入力欄を非表示にする */
#message-composer {
  display: none !important;
}
/* ウォーターマーク（免責文）を非表示 */
.watermark {
  display: none !important;
}
//}
@<code>{#message-composer}はChainlitソースコードのMessageComposer/index.tsxに由来するIDです。
@<code>{.chainlit/config.toml}に以下を追加すると適用されます。
//emlist[.chainlit/config.toml]{
[UI]
custom_css = "/public/stylesheet.css"
//}

== まとめ

=== CustomElementで広がる可能性

今回はお絵かきキャンバスを実装しましたが、CustomElementの応用は絵しりとりに限りません。

 * @<b>{フォーム} ― チャット中に構造化データを入力させるカスタムフォーム
 * @<b>{データ可視化} ― グラフやチャートをインラインで表示（Canvas API / SVG）
 * @<b>{ミニゲーム} ― クイズ、パズル、インタラクティブなチュートリアル
 * @<b>{ファイル編集} ― コードエディタやマークダウンエディタの埋め込み

@<code>{props}で受け取り、@<code>{callAction}で返す。
このシンプルな双方向通信パターンさえ押さえれば、チャットUIの中に何でも埋め込むことが可能です。

=== まとめ

zosu!waiwai!

Chainlitは「チャットUI」の印象が強いフレームワークですが、CustomElementを活用すれば、チャットの枠を超えたリッチなインタラクションを実現できます。
ぜひあなたも、自分だけのCustomElementを作ってみてください！
